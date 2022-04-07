/**
 * W800 Uart library for Lua
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

//uart init
//params: uart num, baudrate, mode
//example: wmuart.init(UART0, 9600, MODE_UART2)
static int wm_uart_init(lua_State *L)
{
	int uart_id = luaL_checkinteger(L, 1);
	int bandrate = luaL_checkinteger(L, 2);
	int mode = luaL_checkinteger(L, 3);

	tls_uart_options_t opt;
	opt.baudrate = bandrate;
    opt.paritytype = TLS_UART_PMODE_DISABLED;
    opt.stopbits = TLS_UART_ONE_STOPBITS;
    opt.charlength = TLS_UART_CHSIZE_8BIT;
    opt.flow_ctrl = TLS_UART_FLOW_CTRL_NONE;

	if (WM_SUCCESS != tls_uart_port_init(uart_id, &opt, mode)) {
		printf("uart init error\n");
		lua_pushinteger(L, 1);
		return 1;
    }

	lua_pushinteger(L, 0);
	return 1;
}

static int wm_register_rx_callback(lua_State *L)
{
	return 0;
}

static const luaL_Reg uartlib[] = {
	{"init", wm_uart_init}, // restart app
	{"rx_callback", wm_register_rx_callback},
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{
	lua_pushinteger(L, 0);
	lua_setglobal(L, "MODE_UART2");
	lua_pushinteger(L, 1);
	lua_setglobal(L, "MODE_7816");
	lua_pushinteger(L, 0);
	lua_setglobal(L, "UART0");
	lua_pushinteger(L, 1);
	lua_setglobal(L, "UART1");
	lua_pushinteger(L, 2);
	lua_setglobal(L, "UART2");
	lua_pushinteger(L, 3);
	lua_setglobal(L, "UART3");
	lua_pushinteger(L, 4);
	lua_setglobal(L, "UART4");
	lua_pushinteger(L, 5);
	lua_setglobal(L, "UART5");
}

LUAMOD_API int luaopen_wm_uart (lua_State *L) 
{
	luaL_newlib(L, uartlib);
	set_lua_global(L);
	return 1;
}