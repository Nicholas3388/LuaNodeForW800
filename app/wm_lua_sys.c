/**
 * W800 System library for Lua
 *
 * WangWei
 * 2022.04.01
 */

#include <stdio.h>
#include "lua/lua.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "global.h"
#include "wm_include.h"

/*
 * Lua Example:
 * 
 * require "wmsys"
 * print(wmsys.ticks())
 * wmsys.delay(1000)
 * print(wmsys.version())
 * wmsys.restart()
 */

// restart app
static int wm_app_restart (lua_State *L) 
{
	printf("restart app\n");
	tls_sys_reset();
	return 0;
}

// get current ticks
static int wm_get_time(lua_State *L) 
{
	int ticks = tls_os_get_time();
	lua_pushinteger(L, ticks);
	return 1;
}

// delay current task for n ticks
static int wm_delay(lua_State *L)
{
	if (lua_isinteger(L, 1)) {
		lua_Integer n = lua_tointeger(L, 1);
		tls_os_time_delay(n);
		printf("delay over\n");
	} else {
		printf("parameter is not integer\n");
	}
	return 0;
}

// return lib version
static int wm_lib_version(lua_State *L)
{
	lua_pushstring(L, LIB_VERSION);
	return 1;
}

//init watchdog
//params: time (us)
//return none
static int wm_watchdog_init(lua_State *L)
{
	if (lua_isinteger(L, 1)) {
		u32 t = lua_tointeger(L, 1);
		tls_watchdog_init(t);
	}
	return 0;
}

static int wm_watchdog_deinit(lua_State *L)
{
	tls_watchdog_deinit();
	return 0;
}

static int wm_watchdog_feed(lua_State *L)
{
	tls_watchdog_clr();
	return 0;
}

static const luaL_Reg syslib[] = {
	{"restart", wm_app_restart}, // restart app
	{"ticks", wm_get_time},
	{"delay", wm_delay},
	{"version", wm_lib_version},
	{"watchdog_init", wm_watchdog_init},
	{"watchdog_deinit", wm_watchdog_deinit},
	{"watchdog_feed", wm_watchdog_feed},
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{
}

LUAMOD_API int luaopen_wm_sys (lua_State *L) 
{
	luaL_newlib(L, syslib);
	set_lua_global(L);
	return 1;
}