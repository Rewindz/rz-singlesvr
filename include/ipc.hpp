#pragma once

#include <nlohmann/json.hpp>

enum class IPCAction
{
    STOP,
    RUN,
    CMD,
    INVALID=-1,
};

NLOHMANN_JSON_SERIALIZE_ENUM(IPCAction, {
    {IPCAction::INVALID, nullptr},
    {IPCAction::STOP, "stop"},
    {IPCAction::RUN, "run"},
    {IPCAction::CMD, "cmd"},
});

struct IPCMessage
{
    IPCAction action{IPCAction::INVALID};
    nlohmann::json args{nlohmann::json::object()};
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IPCMessage, action, args);

enum class IPCStatus
{
    ERROR,
    SUCCESS
};

NLOHMANN_JSON_SERIALIZE_ENUM(IPCStatus, {
   {IPCStatus::ERROR, "error"},
   {IPCStatus::SUCCESS, "success"},
});

struct IPCReply
{
    IPCStatus status;
    nlohmann::json data;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(IPCReply, status, data);
