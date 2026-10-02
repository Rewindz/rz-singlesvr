#pragma once

#include <lua.hpp>

#include <memory>

void RegisterLuaFunctions(lua_State* L);

class LuaWrapper
{
    public:
        LuaWrapper()
            : L(luaL_newstate())
        {}

        operator lua_State*() const {
            return L.get();
        }

    private:
        struct Deleter {
            void operator()(lua_State* state) const {
                if(state)
                    lua_close(state);
            }
        };
        std::unique_ptr<lua_State, Deleter> L;
};
