/**
 * WinnerMicro W800 Wifi library for Lua
 *
 * WangWei
 * 2022.04.01
 */

#include <stdio.h>
#include "lua/lua.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"


static const luaL_Reg wifilib[] = {
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{
}

LUAMOD_API int luaopen_wm_wifi (lua_State *L) 
{
	luaL_newlib(L, wifilib);
	set_lua_global(L);
	return 1;
}