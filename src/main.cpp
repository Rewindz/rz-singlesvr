
#include <exception>
#include <print>
#include <iostream>

#include <spdlog/spdlog.h>

#include <argparse/argparse.hpp>

#include <rz/rzutils.hpp>

#include "settings.hpp"
#include "steamcmd.hpp"

int main(int argc, char** argv)
{
    argparse::ArgumentParser program("rzsvr", "1.0.0");

    spdlog::set_pattern("[%^%l%$] %v");


    program.add_argument("-i", "--init")
        .flag()
        .help("initialize the configuration");

    program.add_argument("--config")
        .flag()
        .help("print out the path for this programs config file");

    program.add_argument("-u", "--update")
        .flag()
        .help("update/install the app (with appid from the config)");

    try {

        program.parse_args(argc, argv);

    } catch (const std::exception& e) {
        spdlog::error("{}\n{}", e.what(), program.help().str());
        return 1;
    }


    auto appPath = rz::fs::GetAppConfigPath("rzsvr");
    std::filesystem::path configPath{};
    if(appPath) {
        configPath = *appPath / "config.json";
    } else {
        spdlog::error("Failed to get app config path!");
        return 1;
    }
    rz::json::Saveable<AppSettings> appSettings{configPath, 4};

    /* ------------------------------------- BEFORE LOADING CONFIG ------------------------------------- */

    if(program["--init"] == true) {
        std::printf("Steam Server AppID: ");
        std::getline(std::cin, appSettings->app_id);
        std::printf("Directory to force install app (leave empty for default install directory): ");
        std::getline(std::cin, appSettings->install_dir);
        std::printf("Directory to install SteamCMD (leave empty for default): ");
        std::getline(std::cin, appSettings->steam_dir);

        if(appSettings->steam_dir.empty()) {
            auto homePath = rz::fs::GetHomePath();
            if(homePath) {
                std::filesystem::path steamPath = *homePath / "steamcmd";
                appSettings->steam_dir = steamPath.string();
            } else {
                spdlog::error("Failed to get home directory path.");
                return 1;
            }
        }

        spdlog::info("AppID: {}\tInstall Dir: {}\tSteam Dir: {}", appSettings->app_id,
            (appSettings->install_dir.empty() ? "default" : appSettings->install_dir),
            appSettings->steam_dir);

        std::printf("Confirm? [y/n]: ");
        std::string confirm{};
        std::getline(std::cin, confirm);
        if(!confirm.starts_with('y')) {
            spdlog::info("Canceling initialization....");
            return 1;
        }

        auto res = appSettings.Save();
        if(res == rz::STATUS::RZ_ERROR) {
            spdlog::error("Failed to save config! {}", configPath.string());
            return 1;
        }

        spdlog::info("Saved config to: {}", configPath.string());

        SteamCmdManager steam{appSettings->steam_dir};
        if(steam.isInstalled()) {
            spdlog::warn("steamcmd is already installed at {}! skipping steamcmd install process..", appSettings->steam_dir);
        } else {
            spdlog::info("Downloading steamcmd....");
            if(!steam.setup()) {
                spdlog::error("Failed to download/initialize steamcmd...");
                return 1;
            }
            spdlog::info("steamcmd initialization success! Install/update the server app with --update");
        }

        return 0;
    }

    if(!rz::fs::ExistsAndIsRegularFile(configPath)) {
        spdlog::warn("rzsvr config does not exist! Create with {} --init\n{}", argv[0], program.help().str());
        return 1;
    }

    {
        auto res = appSettings.Load();
        if(res == rz::STATUS::RZ_ERROR) {
            spdlog::error("Failed to load config! {}", configPath.string());
            return 1;
        }
    }

    /* ------------------------------------- AFTER LOADING CONFIG ------------------------------------- */

    if(program["--config"] == true) {
        spdlog::info("rzsvr config path:\n{}", configPath.string());
    }

    if(program["--update"] == true) {
        SteamCmdManager steam{appSettings->steam_dir};

        if(!steam.isInstalled()) {
            spdlog::error("steamcmd not install at directory {}!", appSettings->steam_dir);
            return 1;
        }

        bool success{false};
        if(appSettings->install_dir.empty())
            success = steam.updateApp(appSettings->app_id);
        else{
            std::filesystem::path install = appSettings->install_dir;
            success = steam.updateApp(appSettings->app_id, install);
        }

        if(success) {
            spdlog::info("Successfully updated {}!", appSettings->app_id);
            return 0;
        } else {
            spdlog::error("Failed to update {}!", appSettings->app_id);
            return 1;
        }

    }

    return 0;
}
