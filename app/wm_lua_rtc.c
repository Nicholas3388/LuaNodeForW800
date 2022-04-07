/**
 * W800 RTC library for Lua
 *
 * Time read/write from pmu RTC
 *
 * WangWei
 * 2022.04.01
 */

#include <stdio.h>
#include <string.h>
#include "lua/lua.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "global.h"
#include "wm_rtc.h"

static struct tm gtblock;
static int lua_callback = LUA_REFNIL;
static lua_State *globalL;

//set time
//params: year, month, day, hour, minute, second
//return none
static int wm_set_rtc(lua_State *L)
{
	int year = luaL_checkinteger(L, 1);
	int month = luaL_checkinteger(L, 2);
	int day = luaL_checkinteger(L, 3);
	int hour = luaL_checkinteger(L, 4);
	int minute = luaL_checkinteger(L, 5);
	int second = luaL_checkinteger(L, 6);
    gtblock.tm_year = year;
    gtblock.tm_mon = month;
    gtblock.tm_mday = day;
    gtblock.tm_hour = hour;
    gtblock.tm_min = minute;
    gtblock.tm_sec = second;
    tls_set_rtc(&gtblock);
	return 0;
}

//get time
//params: none
//return: YY-mm-DD HH:MM:SS
static int wm_get_rtc(lua_State *L)
{
	struct tm tblock;
    tls_get_rtc(&tblock);
	lua_pushinteger(L, tblock.tm_year);
	lua_pushinteger(L, tblock.tm_mon);
	lua_pushinteger(L, tblock.tm_mday);
	lua_pushinteger(L, tblock.tm_hour);
	lua_pushinteger(L, tblock.tm_min);
	lua_pushinteger(L, tblock.tm_sec);
	return 6;
}

static int wm_start_rtc(lua_State *L)
{
	tls_rtc_timer_start(&gtblock);
	return 0;
}

static int wm_stop_rtc(lua_State *L)
{
	tls_rtc_timer_stop();
	return 0;
}

static void rtc_clock_irq(void *arg)
{
    struct tm tblock;
    tls_get_rtc(&tblock);
    printf("rtc clock, sec=%d,min=%d,hour=%d,mon=%d,year=%d\n", tblock.tm_sec, tblock.tm_min, tblock.tm_hour, tblock.tm_mon, tblock.tm_year);
	lua_rawgeti(globalL, LUA_REGISTRYINDEX, lua_callback);
	lua_call(globalL, 0, 0);
}

//register RTC interrupt
//params: callback name
//return none
static int wm_rtc_register(lua_State *L)
{
	const char *callback_name = luaL_optstring(L, 1, NULL);
	tls_rtc_isr_register(rtc_clock_irq, NULL);
	lua_getglobal(L, callback_name);
	lua_callback = luaL_ref(L, LUA_REGISTRYINDEX); //new callback ref
	globalL = L;
	return 0;
}

static const luaL_Reg rtclib[] = {
	{"set", wm_set_rtc},
	{"get", wm_get_rtc},
	{"start", wm_start_rtc},
	{"stop", wm_stop_rtc},
	{"register", wm_rtc_register},
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{

}

LUAMOD_API int luaopen_wm_rtc(lua_State *L) 
{
	luaL_newlib(L, rtclib);
	set_lua_global(L);
	return 1;
}