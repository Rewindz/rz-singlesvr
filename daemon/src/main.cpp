#include <cstdlib>
#include <filesystem>
#include <string>
#include <rz/rzutils.hpp>

#include <spdlog/spdlog.h>

#include <lua.hpp>

#include <sys/socket.h>
#include <sys/types.h>

#include "dispatcher.hpp"
#include "settings.hpp"
#include "socketfd.hpp"
#include "ipc.hpp"
#include "luapi.hpp"

using AppSettingsFile = rz::json::Saveable<AppSettings>;

inline void LoadConfig(AppSettingsFile& config)
{
    bool success = config.Load() == rz::STATUS::RZ_SUCCESS;
    if(!success) {
        spdlog::error("Failed to load config!");
        std::quick_exit(1);
    }
}

int main(void)
{

    auto appPath = rz::fs::GetAppConfigPath("rzsvr");
    std::filesystem::path configPath{};
    if(appPath) {
        configPath = *appPath / "config.json";
    } else {
        spdlog::error("App path doesn't exist! Have you initialized with the cli?");
        return 1;
    }

    AppSettingsFile appConfig{configPath, APPSETTINGS_INDENT};
    LoadConfig(appConfig);

    LuaWrapper L{};

    RegisterLuaFunctions(L);

    SocketFd serverFd {SocketFd::GetSocket()};

    if(!serverFd.is_valid()) {
        spdlog::error("Failed to create local socket.");
        return 1;
    }

    auto [addr, addr_len] = SocketFd::MakeAddr(SOCKET_NAME);

    if(bind(serverFd.get(), reinterpret_cast<sockaddr*>(&addr), addr_len) == -1) {
        spdlog::error("Socket bind failed.");
        return 1;
    }

    if(listen(serverFd.get(), 5) == -1) {
        spdlog::error("Socket listen failed.");
        return 1;
    }

    spdlog::info("Daemon listening on socket: \\0{}", SOCKET_NAME);

    IPCDispatcher dispatcher
    {{
        { IPCAction::RUN, [](const auto& args){ return std::unexpected{"TODO"}; } },
        { IPCAction::STOP, [](const auto& args){ return std::unexpected{"TODO"}; } },
        { IPCAction::CMD, [](const auto& args){ return std::unexpected{"TODO"}; } },
    }};

    // probably use poll here later
    while(true)
    {
        SocketFd clientFd{accept(serverFd.get(), nullptr, nullptr)};
        if(!clientFd.is_valid()) continue;
        std::array<char, 1024> buffer{};
        ssize_t bytesRead = recv(clientFd.get(), buffer.data(), buffer.size(), 0);
        if(bytesRead > 0) {
            std::string_view msg {buffer.data(), static_cast<size_t>(bytesRead)};
            spdlog::debug("Message received: {}", msg);

            auto reply = nlohmann::json{dispatcher.dispatch(msg)}.dump(APPSETTINGS_INDENT);
            send(clientFd.get(), reply.data(), reply.size(), 0);
        }
    }


    return 0;
}
