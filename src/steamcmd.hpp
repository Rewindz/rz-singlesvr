#pragma once
#include <string>
#include <filesystem>

class SteamCmdManager {
public:

    explicit SteamCmdManager(std::filesystem::path installDir);

    bool setup();
    bool isInstalled() const;
    bool updateApp(int appId, const std::filesystem::path& installDir);

private:
    std::filesystem::path m_installDir;
    const std::string STEAMCMD_URL = "https://steamcdn-a.akamaihd.net/client/installer/steamcmd_linux.tar.gz";

    bool downloadArchive(const std::filesystem::path& archivePath);
    bool extractArchive(const std::filesystem::path& archivePath);
    bool initializeSteamCmd();
};
