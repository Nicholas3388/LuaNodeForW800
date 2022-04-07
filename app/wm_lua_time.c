/**
 * W800 Time library for Lua
 *
 * User can create at most 6 timers
 *
 * WangWei
 * 2022.04.01
 */

#include <stdio.h>
#include <string.h>
#include "lua/lua.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "wm_timer.h"
#include "global.h"

#define CALLBACK_NAME_LENGTH	64

typedef struct WM_LUA_TIMER
{
    int timer_id;
	int lua_callback;
	char callback_name[CALLBACK_NAME_LENGTH];
} WM_LUA_TIMER_ST;

static WM_LUA_TIMER_ST local_timer[TIMER_NUM];
static lua_State *globalL;

static void demo_timer_irq(u8 *arg)
{
	int id = (int)(*arg);
	printf("timer %d irq\n", id);
	if (id >= TIMER_NUM) {
		printf("index out of bound\n");
		return;
	}
	lua_rawgeti(globalL, LUA_REGISTRYINDEX, local_timer[id].lua_callback);
	lua_call(globalL, 0, 0);
}

//Create a new timer (ms). 
//Params: loop(ms), repeat, callback_name
//return timer id
static int wm_create_timer(lua_State *L)
{
	u8 timer_id;
	struct tls_timer_cfg timer_cfg;
	int timer_index = -1;
	for (int i = 0; i < TIMER_NUM; i++) {
		if (local_timer[i].timer_id < 0) {
			timer_index = i;
			break;
		}
	}
	if (timer_index < 0) {
		printf("You can create 6 timer at most\n");
		lua_pushinteger(L, 1);
		return 1;
	}
	
	int ms = luaL_checkinteger(L, 1);
	int repeat = luaL_checkinteger(L, 2);
	const char *callback_name = luaL_optstring(L, 3, NULL);

	timer_cfg.unit = TLS_TIMER_UNIT_MS;
	timer_cfg.timeout = ms;
	timer_cfg.is_repeat = repeat;
	timer_cfg.callback = (tls_timer_irq_callback)demo_timer_irq;
	timer_cfg.arg = &(local_timer[timer_index].timer_id);
	timer_id = tls_timer_create(&timer_cfg);

	local_timer[timer_index].timer_id = timer_id;
	globalL = L;
	if (callback_name != NULL) {
		lua_getglobal(L, callback_name);
		local_timer[timer_index].lua_callback = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	printf("\ntimer id: %d\n", timer_id);
	lua_pushinteger(L, timer_id);
	return 1;
}

//params: none
static void lib_init(void)
{
	for (int i = 0; i < TIMER_NUM; i++) {
		local_timer[i].timer_id = -1;
		local_timer[i].lua_callback = LUA_REFNIL;
	}
}

//params: none
static int wm_start_timer(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	if (id > TIMER_NUM) {
		printf("index out of bound\n");
		return 0;
	}
	tls_timer_start(id);
	return 0;
}

//params: none
static int wm_stop_timer(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	if (id > TIMER_NUM) {
		printf("index out of bound\n");
		return 0;
	}
	tls_timer_stop(id);
	return 0;
}

//Change timer trigger time
static int wm_change_timer(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	if (id > TIMER_NUM) {
		printf("index out of bound\n");
		return 0;
	}
	int new_wait_time = luaL_checkinteger(L, 2);
	tls_timer_change(id, new_wait_time);
	return 0;
}

//Destroy timer
//params: timer id
static int wm_destroy_timer(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	if (id > TIMER_NUM) {
		printf("index out of bound\n");
		return 0;
	}
	tls_timer_destroy(id);
	local_timer[id].timer_id = -1;
	local_timer[id].lua_callback = LUA_REFNIL;
	printf("timer destory\n");
	return 0;
}

//Change timer callback
//params: timer id, callback name
static int wm_set_timer_callback(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	if (id > TIMER_NUM) {
		printf("index out of bound\n");
		return 0;
	}
	const char *callback_name = luaL_optstring(L, 2, NULL);
	if (callback_name != NULL) {
		luaL_unref(L, LUA_REGISTRYINDEX, local_timer[id].lua_callback); //unref
		lua_getglobal(L, callback_name);
		local_timer[id].lua_callback = luaL_ref(L, LUA_REGISTRYINDEX); //new callback ref
	}
	return 0;
}

//read current timer value, 
//params: timer id, 
//return current value
static int wm_read_timer_value(lua_State *L)
{
	int id = luaL_checkinteger(L, 1);
	if (id > TIMER_NUM) {
		printf("index out of bound\n");
		return 0;
	}
	u32 val = tls_timer_read(id);
	lua_pushinteger(L, val);
	return 1;
}

static const luaL_Reg timelib[] = {
	{"create", wm_create_timer}, // restart app
	{"start", wm_start_timer},
	{"stop", wm_stop_timer},
	{"change", wm_change_timer},
	{"destroy", wm_destroy_timer},
	{"callback", wm_set_timer_callback},
	{"value", wm_read_timer_value},
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{
	lua_pushinteger(L, TIMER_NUM);
	lua_setglobal(L, "MAX_TIMER_NUM"); // the max number of timer user can create
	lua_pushinteger(L, 1);
	lua_setglobal(L, "REPEAT");
	lua_pushinteger(L, 0);
	lua_setglobal(L, "ONCE");
}

LUAMOD_API int luaopen_wm_time (lua_State *L) 
{
	luaL_newlib(L, timelib);
	set_lua_global(L);
	lib_init();
	return 1;
}