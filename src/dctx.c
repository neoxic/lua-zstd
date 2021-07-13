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

/* ARG: ddict */
static int m_refDDict(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	ZSTD_DDict *ddict = checkddict(L, 2);
	lua_settop(L, 2);
	lua_getuservalue(L, 1);
	lua_insert(L, 2);
	lua_rawseti(L, 2, 1);
	zstd__check(L, ZSTD_DCtx_refDDict(dctx, ddict));
	return 0;
}

/* ARG: data
** RES: data, ['end'] | nil, error */
static int m_decompressStream(lua_State *L) {
	size_t res, slen, spos = 0, dlen = 0, dpos = 0, blen = 100;
	void *ud, *buf, *dst = 0;
	int err = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	const void *src = luaL_checklstring(L, 2, &slen);
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

/* ARG: data, ddict
** RES: data | nil, error */
static int m_decompressBlock(lua_State *L) {
	size_t res, slen, dlen;
	void *ud, *dst;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	const void *src = luaL_checklstring(L, 2, &slen);
	ZSTD_DDict *ddict = checkddict(L, 3);
	int wlog;
	zstd__check(L, ZSTD_decompressBegin_usingDDict(dctx, ddict));
	zstd__check(L, ZSTD_DCtx_getParameter(dctx, ZSTD_d_windowLogMax, &wlog));
	if (wlog > ZSTD_BLOCKSIZELOG_MAX) wlog = ZSTD_BLOCKSIZELOG_MAX;
	checkmem(L, dst = allocf(ud, 0, 0, dlen = (size_t)1 << wlog));
	if (zstd__error(L, res = ZSTD_decompressBlock(dctx, dst, dlen, src, slen))) {
		allocf(ud, dst, dlen, 0);
		return 2;
	}
	lua_pushlstring(L, dst, res);
	allocf(ud, dst, dlen, 0);
	return 1;
}

/* ARG: [mode] */
static int m_reset(lua_State *L) {
	ZSTD_DCtx *dctx = checkdctx(L, 1);
	int mode = zstd__checkresetmode(L, 2);
	zstd__check(L, ZSTD_DCtx_reset(dctx, mode));
	if (mode == ZSTD_reset_session_only) return 0; // Dictionary stays referenced
	lua_getuservalue(L, 1);
	lua_pushnil(L);
	lua_rawseti(L, -2, 1);
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
	{"refDDict", m_refDDict},
	{"decompressStream", m_decompressStream},
	{"decompressBlock", m_decompressBlock},
	{"reset", m_reset},
	{"__gc", m__gc},
	{0, 0}
};

/* RES: dctx */
int zstd__newDCtx(lua_State *L) {
	ZSTD_DCtx **dctx = lua_newuserdata(L, sizeof(*dctx));
	checkmem(L, *dctx = ZSTD_createDCtx());
	lua_createtable(L, 1, 0);
	lua_setuservalue(L, 1);
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
