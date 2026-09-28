
#include <exception>
#include <print>
#include <iostream>

#include <argparse/argparse.hpp>

#include <rz/rzutils.hpp>

#include "settings.hpp"

int main(int argc, char** argv)
{
    argparse::ArgumentParser program("rzsvr", "1.0.0");


    program.add_argument("-i", "--init")
        .flag()
        .help("Initialize the configuration.");

    try {

        program.parse_args(argc, argv);

    } catch (const std::exception& e) {
        std::println(stderr, "{}\n{}", e.what(), program.help().str());
        return 1;
    }


    auto appPath = rz::fs::GetAppConfigPath("rzsvr");
    std::filesystem::path configPath{};
    if(appPath) {
        configPath = *appPath / "config.json";
    } else {
        std::println(stderr, "Failed to get app config path!");
        return 1;
    }
    rz::json::Saveable<AppSettings> appSettings{configPath, 4};

    /* ------------------------------------- BEFORE LOADING CONFIG ------------------------------------- */

    if(program["--init"] == true) {
        std::printf("Steam Server AppID: ");
        std::getline(std::cin, appSettings->app_id);
        std::printf("Directory to force install (leave empty for default install directory): ");
        std::getline(std::cin, appSettings->install_dir);
        std::println("AppID: {}\tInstall Dir: {}", appSettings->app_id,
            (appSettings->install_dir.empty() ? "default" : appSettings->install_dir));
        auto res = appSettings.Save();
        if(res == rz::STATUS::RZ_ERROR) {
            std::println(stderr, "Failed to save config! {}", configPath.string());
            return 1;
        }
        return 0;
    }

    if(!rz::fs::ExistsAndIsRegularFile(configPath)) {
        std::println("rzsvr config does not exist! Create with {} --init\n{}", argv[0], program.help().str());
        return 1;
    }

    {
        auto res = appSettings.Load();
        if(res == rz::STATUS::RZ_ERROR) {
            std::println(stderr, "Failed to load config! {}", configPath.string());
            return 1;
        }
    }

    /* ------------------------------------- AFTER LOADING CONFIG ------------------------------------- */

    return 0;
}
