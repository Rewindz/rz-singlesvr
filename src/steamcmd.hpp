#pragma once
#include <filesystem>
#include <optional>
#include <functional>

#include <string_view>

class SteamCmdManager {
public:

    explicit SteamCmdManager(std::filesystem::path installDir);

    using OptionalConstPathRef = std::optional<std::reference_wrapper<const std::filesystem::path>>;

    bool setup();
    bool isInstalled() const;
    bool updateApp(std::string_view appId, OptionalConstPathRef appInstallDir = std::nullopt);

private:
    std::filesystem::path m_installDir;

    bool downloadArchive(const std::filesystem::path& archivePath);
    bool extractArchive(const std::filesystem::path& archivePath);
    bool initializeSteamCmd();
};
