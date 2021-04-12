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

static const char *const s_param[] = {
	"windowLogMax",
	"format",
	"stableOutBuffer",
	"forceIgnoreChecksum",
	"refMultipleDDicts",
	0
};

static const int v_param[] = {
	ZSTD_d_windowLogMax,
	ZSTD_d_format,
	ZSTD_d_stableOutBuffer,
	ZSTD_d_forceIgnoreChecksum,
	ZSTD_d_refMultipleDDicts,
};

int zstd__checkdctxparam(lua_State *L, int arg) {
	return v_param[luaL_checkoption(L, arg, 0, s_param)];
}

/* ARG: name
** RES: value */
static int m_getParameter(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	int param = zstd__checkdctxparam(L, 2);
	int value;
	zstd__check(L, ZSTD_DCtx_getParameter(dctx, param, &value));
	lua_pushinteger(L, value);
	return 1;
}

/* ARG: name, value */
static int m_setParameter(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	int param = zstd__checkdctxparam(L, 2);
	int value = luaL_checkinteger(L, 3);
	zstd__check(L, ZSTD_DCtx_setParameter(dctx, param, value));
	return 0;
}

/* ARG: data
** RES: data, ['end'] | nil, error */
static int m_decompressStream(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	size_t slen;
	const void *src = luaL_checklstring(L, 2, &slen);
	void *ud, *buf, *dst = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	size_t res, spos = 0, dpos = 0, dlen = 0, blen = 100;
	int err = 0;
	luaL_argcheck(L, slen, 2, "empty data"); /* Ensure forward progress */
	for (;;) {
		if (!(buf = allocf(ud, dst, dlen, blen))) {
			err = ZSTD_error_memory_allocation;
			break;
		}
		dst = buf;
		dlen = blen;
		if (!(res = ZSTD_decompressStream_simpleArgs(dctx, dst, dlen, &dpos, src, slen, &spos))) break; /* End of stream */
		if ((err = ZSTD_getErrorCode(res))) break; /* Error occurred */
		if (dpos < dlen) break; /* No more data to flush */
		blen <<= 1;
	}
	if (zstd__pusherror(L, err)) {
		allocf(ud, dst, dlen, 0);
		return 2;
	}
	lua_pushlstring(L, dst, dpos);
	allocf(ud, dst, dlen, 0);
	if (res) return 1;
	lua_pushliteral(L, "end");
	return 2;
}

/* ARG: [mode] */
static int m_reset(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	int mode = zstd__checkresetmode(L, 2);
	zstd__check(L, ZSTD_DCtx_reset(dctx, mode));
	return 0;
}

static int m__gc(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	ZSTD_freeDCtx(dctx);
	return 0;
}

static const luaL_Reg t_dctx[] = {
	{"getParameter", m_getParameter},
	{"setParameter", m_setParameter},
	{"decompressStream", m_decompressStream},
	{"reset", m_reset},
	{"__gc", m__gc},
	{0, 0}
};

/* RES: dctx */
int zstd__newDCtx(lua_State *L) {
	ZSTD_DCtx **dctx = lua_newuserdata(L, sizeof(*dctx));
	checkmem(L, *dctx = ZSTD_createDCtx());
	if (luaL_newmetatable(L, TYPE_DCTX)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
#if LUA_VERSION_NUM < 502
		luaL_register(L, 0, t_dctx);
#else
		luaL_setfuncs(L, t_dctx, 0);
#endif
	}
	lua_setmetatable(L, -2);
	return 1;
}
