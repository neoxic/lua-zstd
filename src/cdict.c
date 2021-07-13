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

/* RES: id */
static int m_getId(lua_State *L) {
	ZSTD_CDict *cdict = checkcdict(L, 1);
	lua_pushnumber(L, ZSTD_getDictID_fromCDict(cdict));
	return 1;
}

static int m__gc(lua_State *L) {
	ZSTD_CDict *cdict = checkcdict(L, 1);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	ZSTD_freeCDict(cdict);
	return 0;
}

static const luaL_Reg t_cdict[] = {
	{"getId", m_getId},
	{"__gc", m__gc},
	{0, 0}
};

/* ARG: data, cctxparams
** RES: cdict */
int zstd__newCDict(lua_State *L) {
	size_t len;
	const void *buf = luaL_checklstring(L, 1, &len);
	ZSTD_CCtx_params *params = checkcctxparams(L, 2);
	ZSTD_CDict **cdict = lua_newuserdata(L, sizeof(*cdict));
	checkmem(L, *cdict = ZSTD_createCDict_advanced2(buf, len, ZSTD_dlm_byCopy, ZSTD_dct_auto, params, ZSTD_defaultCMem));
	if (luaL_newmetatable(L, TYPE_CDICT)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
#if LUA_VERSION_NUM < 502
		luaL_register(L, 0, t_cdict);
#else
		luaL_setfuncs(L, t_cdict, 0);
#endif
	}
	lua_setmetatable(L, -2);
	return 1;
}
