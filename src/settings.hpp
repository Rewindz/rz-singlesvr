#pragma once


#include <nlohmann/json.hpp>


struct AppSettings
{
    std::string app_id{};
    std::string install_dir{};
    std::string steam_dir{};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(AppSettings, app_id, install_dir, steam_dir);
