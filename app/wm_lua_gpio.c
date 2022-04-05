/**
 * WinnerMicro W800 GPIO library for Lua
 *
 * WangWei
 * 2022.04.01
 */

#include <stdio.h>
#include "lua/lua.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "wm_include.h"

/*
 * Lua Example:
 * 
 * require "wmgpio"
 * ret=wmgpio.init(PORTB, 8, GPIO_OUT, FLOATING)
 * if ret==0 then 
 *   wmgpio.out(PORTB, 8, HIGH)
 * 	 wmgpio.out(PORTB, 8, LOW)
 * else 
	 print("GPIO init failed")
 * end
 */

static int get_pin_index(char *port, int offset)
{
	int pin_index = 0;
	if (strstr(port, "PA") != NULL) {
		pin_index = 0;
	} else if (strstr(port, "PB") != NULL) {
		pin_index = 16;
	} else {
		return -1;
	}
	return pin_index+offset;
}

//params: port, pin, direction, attribute
static int wm_gpio_init(lua_State *L) 
{
	int pin_index = 0;
	const char *port_name = luaL_optstring(L, 1, NULL);
	printf("lua set port: %s\n", port_name);
	if (port_name == NULL) {
		printf("Invalid port\n");
		lua_pushinteger(L, 1);
		return 1;
	}
	int offset = luaL_checkinteger(L, 2);
	pin_index = get_pin_index(port_name, offset);
	if (pin_index < 0) {
		printf("Invalid pin index\n");
		lua_pushinteger(L, 2);
		return 1;
	}
	int dir = luaL_checkinteger(L, 3);
	int attribute = luaL_checkinteger(L, 4);
	tls_gpio_cfg(pin_index, dir, attribute);
	lua_pushinteger(L, 0);
	return 1;
}

//params: port, pin, value
static int wm_gpio_out(lua_State *L) 
{
	int pin_index = 0;
	const char *port_name = luaL_optstring(L, 1, NULL);
	//printf("lua set port: %s\n", port_name);
	if (port_name == NULL) {
		printf("Invalid port\n");
		lua_pushinteger(L, 1);
		return 1;
	}
	int offset = luaL_checkinteger(L, 2);
	pin_index = get_pin_index(port_name, offset);
	if (pin_index < 0) {
		printf("Invalid pin index\n");
		lua_pushinteger(L, 2);
		return 1;
	}
	int value = luaL_checkinteger(L, 3);
	tls_gpio_write(pin_index, value);
	lua_pushinteger(L, 0);
	return 0;
}

//read pin value
static int wm_gpio_in(lua_State *L) 
{
	return 1;
}

static const luaL_Reg gpiolib[] = {
	{"out", wm_gpio_out},
	{"in", wm_gpio_in},
	{"init", wm_gpio_init},
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{
	lua_pushinteger(L, WM_GPIO_DIR_OUTPUT);
	lua_setglobal(L, "GPIO_OUT");
	lua_pushinteger(L, WM_GPIO_DIR_INPUT);
	lua_setglobal(L, "GPIO_IN");
	lua_pushinteger(L, 1);
	lua_setglobal(L, "HIGH");
	lua_pushinteger(L, 0);
	lua_setglobal(L, "LOW");
	lua_pushinteger(L, WM_GPIO_ATTR_FLOATING);
	lua_setglobal(L, "FLOATING");
	lua_pushinteger(L, WM_GPIO_ATTR_PULLHIGH);
	lua_setglobal(L, "PULLHIGH");
	lua_pushinteger(L, WM_GPIO_ATTR_PULLLOW);
	lua_setglobal(L, "PULLLOW");
	lua_pushstring(L, "PB");
	lua_setglobal(L, "PORTB");
	lua_pushstring(L, "PA");
	lua_setglobal(L, "PORTA");
}

LUAMOD_API int luaopen_wm_gpid (lua_State *L) 
{
	luaL_newlib(L, gpiolib);
	set_lua_global(L);
	return 1;
}