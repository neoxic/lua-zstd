/*
** Copyright (C) 2021 Arseny Vakhrushev <arseny.vakhrushev@me.com>
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
*/

#include "common.h"

/* ARG: name
** RES: value */
static int m_get(lua_State *L) {
	ZSTD_CCtx_params *params = checkcctxparams(L, 1);
	int param = zstd__checkcctxparam(L, 2);
	int value;
	zstd__check(L, ZSTD_CCtxParams_getParameter(params, param, &value));
	lua_pushinteger(L, value);
	return 1;
}

/* ARG: name, value */
static int m_set(lua_State *L) {
	ZSTD_CCtx_params *params = checkcctxparams(L, 1);
	int param = zstd__checkcctxparam(L, 2);
	int value = luaL_checkinteger(L, 3);
	zstd__check(L, ZSTD_CCtxParams_setParameter(params, param, value));
	return 0;
}

static int m_reset(lua_State *L) {
	ZSTD_CCtx_params *params = checkcctxparams(L, 1);
	zstd__check(L, ZSTD_CCtxParams_reset(params));
	return 0;
}

static int m__gc(lua_State *L) {
	ZSTD_CCtx_params *params = checkcctxparams(L, 1);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	ZSTD_freeCCtxParams(params);
	return 0;
}

static const luaL_Reg t_cctxparams[] = {
	{"get", m_get},
	{"set", m_set},
	{"reset", m_reset},
	{"__gc", m__gc},
	{0, 0}
};

/* RES: cctxparams */
int zstd__newCCtxParams(lua_State *L) {
	ZSTD_CCtx_params **params = lua_newuserdata(L, sizeof(*params));
	checkmem(L, *params = ZSTD_createCCtxParams());
	if (luaL_newmetatable(L, TYPE_CCTXPARAMS)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
#if LUA_VERSION_NUM < 502
		luaL_register(L, 0, t_cctxparams);
#else
		luaL_setfuncs(L, t_cctxparams, 0);
#endif
	}
	lua_setmetatable(L, -2);
	return 1;
}
