#include <lua.hpp>

static int l_GetVersion(lua_State* L)
{
    lua_pushnumber(L, lua_version(L));
    return 1;
}

void RegisterLuaFunctions(lua_State* L)
{
    lua_pushcfunction(L, l_GetVersion);
    lua_setglobal(L, "GetVersion");
}
