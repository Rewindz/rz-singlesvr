#include "steamcmd.hpp"

#include <cstdlib>
#include <fstream>

#include <spdlog/spdlog.h>
#include <cpr/cpr.h>

#include <string_view>
#include <string>
#include <format>

constexpr std::string_view STEAMCMD_URL{"https://client-update.steamstatic.com/installer/steamcmd_linux.tar.gz"};

SteamCmdManager::SteamCmdManager(std::filesystem::path installDir)
    : m_installDir(installDir) {}

bool SteamCmdManager::isInstalled() const {
    return std::filesystem::exists(m_installDir / "steamcmd.sh");
}

bool SteamCmdManager::setup() {
    if (isInstalled()) {
        spdlog::info("SteamCMD is already installed at {}", m_installDir.string());
        return true;
    }

    //spdlog::info("SteamCMD not found. Starting installation process...");

    std::error_code ec;
    std::filesystem::create_directories(m_installDir, ec);

    std::filesystem::path archivePath = m_installDir / "steamcmd_linux.tar.gz";

    if (!downloadArchive(archivePath)) return false;
    if (!extractArchive(archivePath)) return false;

    std::filesystem::remove(archivePath, ec);

    if(!initializeSteamCmd()) return false;

    // --- FIX VALVE'S STEAMCLIENT.SO BUG ---
    spdlog::info("Applying steamclient.so symlink fix...");

    const char* home = std::getenv("HOME");
    if (home) {
        std::filesystem::path steamSdkDir = std::filesystem::path(home) / ".steam" / "sdk64";
        std::error_code ec;

        std::filesystem::create_directories(steamSdkDir, ec);

        std::filesystem::path source = m_installDir / "linux64" / "steamclient.so";
        std::filesystem::path target = steamSdkDir / "steamclient.so";

        if (!std::filesystem::exists(target, ec)) {
            std::filesystem::create_symlink(source, target, ec);
            if (ec) {
                spdlog::warn("Failed to create steamclient.so symlink: {}", ec.message());
            } else {
                spdlog::info("Successfully linked steamclient.so to ~/.steam/sdk64/");
            }
        }
    } else {
        spdlog::error("Failed to find home directory! Did not apply fix!");
    }

    return true;
}

bool SteamCmdManager::downloadArchive(const std::filesystem::path& archivePath) {
    spdlog::info("Downloading SteamCMD from {}...", STEAMCMD_URL);

    std::ofstream of(archivePath, std::ios::binary);
    if (!of.is_open()) {
        spdlog::error("Failed to open file for writing: {}", archivePath.string());
        return false;
    }

    cpr::Response r = cpr::Download(of, cpr::Url{STEAMCMD_URL});
    if (r.status_code == 200) {
        spdlog::info("Download complete.");
        return true;
    } else {
        spdlog::error("Failed to download SteamCMD. HTTP Status: {}", r.status_code);
        return false;
    }
}

bool SteamCmdManager::extractArchive(const std::filesystem::path& archivePath) {
    spdlog::info("Extracting SteamCMD archive...");

    std::string cmd = "tar -xzf " + archivePath.string() + " -C " + m_installDir.string();

    int result = std::system(cmd.c_str());
    if (result == 0) {
        spdlog::info("Extraction successful.");
        return true;
    } else {
        spdlog::error("Failed to extract archive. System command returned: {}", result);
        return false;
    }
}

bool SteamCmdManager::initializeSteamCmd() {
    spdlog::info("Initializing SteamCMD (downloading core updates)...");

    std::string scriptPath = (m_installDir / "steamcmd.sh").string();
    std::string cmd = scriptPath + " +quit";

    int result = std::system(cmd.c_str());
    if (result == 0) {
        spdlog::info("SteamCMD initialized successfully.");
        return true;
    } else {
        spdlog::error("SteamCMD initialization failed with code {}. Do you have 32-bit libraries (lib32gcc-s1) installed?", result);
        return false;
    }
}

bool SteamCmdManager::updateApp(std::string_view appId, OptionalConstPathRef appInstallDir) {
    spdlog::info("Updating AppID {} into {}", appId,
        (!appInstallDir.has_value() ? "default Steam app directory." : appInstallDir->get().string())
    );

    std::string forceInstall = appInstallDir.has_value() ?
            (" +force_install_dir " + appInstallDir->get().string())
            :
            "";

    std::string cmd = std::format(
       "{}{} +login anonymous +app_update {} validate +quit",
       (m_installDir / "steamcmd.sh").string(),
       forceInstall,
       appId
    );

    int result = std::system(cmd.c_str());
    return result == 0;
}
