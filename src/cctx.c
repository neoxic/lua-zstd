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
	"compressionLevel",
	"windowLog",
	"hashLog",
	"chainLog",
	"searchLog",
	"minMatch",
	"targetLength",
	"strategy",
	"enableLongDistanceMatching",
	"ldmHashLog",
	"ldmMinMatch",
	"ldmBucketSizeLog",
	"ldmHashRateLog",
	"contentSizeFlag",
	"checksumFlag",
	"dictIDFlag",
	"nbWorkers",
	"jobSize",
	"overlapLog",
	"rsyncable",
	"format",
	"forceMaxWindow",
	"forceAttachDict",
	"literalCompressionMode",
	"targetCBlockSize",
	"srcSizeHint",
	"enableDedicatedDictSearch",
	"stableInBuffer",
	"stableOutBuffer",
	"blockDelimiters",
	"validateSequences",
	0
};

static const int v_param[] = {
	ZSTD_c_compressionLevel,
	ZSTD_c_windowLog,
	ZSTD_c_hashLog,
	ZSTD_c_chainLog,
	ZSTD_c_searchLog,
	ZSTD_c_minMatch,
	ZSTD_c_targetLength,
	ZSTD_c_strategy,
	ZSTD_c_enableLongDistanceMatching,
	ZSTD_c_ldmHashLog,
	ZSTD_c_ldmMinMatch,
	ZSTD_c_ldmBucketSizeLog,
	ZSTD_c_ldmHashRateLog,
	ZSTD_c_contentSizeFlag,
	ZSTD_c_checksumFlag,
	ZSTD_c_dictIDFlag,
	ZSTD_c_nbWorkers,
	ZSTD_c_jobSize,
	ZSTD_c_overlapLog,
	ZSTD_c_rsyncable,
	ZSTD_c_format,
	ZSTD_c_forceMaxWindow,
	ZSTD_c_forceAttachDict,
	ZSTD_c_literalCompressionMode,
	ZSTD_c_targetCBlockSize,
	ZSTD_c_srcSizeHint,
	ZSTD_c_enableDedicatedDictSearch,
	ZSTD_c_stableInBuffer,
	ZSTD_c_stableOutBuffer,
	ZSTD_c_blockDelimiters,
	ZSTD_c_validateSequences,
};

int zstd__checkcctxparam(lua_State *L, int arg) {
	return v_param[luaL_checkoption(L, arg, 0, s_param)];
}

/* ARG: name
** RES: value */
static int m_getParameter(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	int param = zstd__checkcctxparam(L, 2);
	int value;
	zstd__check(L, ZSTD_CCtx_getParameter(cctx, param, &value));
	lua_pushinteger(L, value);
	return 1;
}

/* ARG: name, value */
static int m_setParameter(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	int param = zstd__checkcctxparam(L, 2);
	int value = luaL_checkinteger(L, 3);
	zstd__check(L, ZSTD_CCtx_setParameter(cctx, param, value));
	return 0;
}

/* ARG: cctxparams */
static int m_setParameters(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	ZSTD_CCtx_params *params = checkcctxparams(L, 2);
	zstd__check(L, ZSTD_CCtx_setParametersUsingCCtxParams(cctx, params));
	return 0;
}

/* ARG: size */
static int m_setPledgedSrcSize(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	lua_Integer size = luaL_checkinteger(L, 2);
	checkrange(L, size >= -1, 2);
	zstd__check(L, ZSTD_CCtx_setPledgedSrcSize(cctx, size));
	return 0;
}

static const char *const s_op[] = {
	"continue",
	"flush",
	"end",
	0
};

/* ARG: data, [op]
** RES: data | nil, error */
static int m_compressStream(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	size_t slen;
	const void *src = luaL_checklstring(L, 2, &slen);
	int op = luaL_checkoption(L, 3, s_op[0], s_op);
	void *ud, *buf, *dst = 0;
	lua_Alloc allocf = lua_getallocf(L, &ud);
	size_t res, spos = 0, dpos = 0, dlen = 0, blen = 100;
	int err = 0;
	for (;;) {
		if (!(buf = allocf(ud, dst, dlen, blen))) {
			err = ZSTD_error_memory_allocation;
			break;
		}
		dst = buf;
		dlen = blen;
		if (!(res = ZSTD_compressStream2_simpleArgs(cctx, dst, dlen, &dpos, src, slen, &spos, op))) break; /* No more data to flush */
		if ((err = ZSTD_getErrorCode(res))) break; /* Error occurred */
		blen <<= 1;
		res += dlen; /* Last result provides a hint on how much data is left */
		if (blen < res) blen = res;
	}
	if (zstd__pusherror(L, err)) {
		allocf(ud, dst, dlen, 0);
		return 2;
	}
	lua_pushlstring(L, dst, dpos);
	allocf(ud, dst, dlen, 0);
	return 1;
}

/* ARG: [mode] */
static int m_reset(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	int mode = zstd__checkresetmode(L, 2);
	zstd__check(L, ZSTD_CCtx_reset(cctx, mode));
	return 0;
}

static int m__gc(lua_State *L) {
	ZSTD_CCtx *cctx = checkcctx(L, 1);
	lua_pushnil(L);
	lua_setmetatable(L, 1);
	ZSTD_freeCCtx(cctx);
	return 0;
}

static const luaL_Reg t_cctx[] = {
	{"getParameter", m_getParameter},
	{"setParameter", m_setParameter},
	{"setParameters", m_setParameters},
	{"setPledgedSrcSize", m_setPledgedSrcSize},
	{"compressStream", m_compressStream},
	{"reset", m_reset},
	{"__gc", m__gc},
	{0, 0}
};

/* RES: cctx */
int zstd__newCCtx(lua_State *L) {
	ZSTD_CCtx **cctx = lua_newuserdata(L, sizeof(*cctx));
	checkmem(L, *cctx = ZSTD_createCCtx());
	if (luaL_newmetatable(L, TYPE_CCTX)) {
		lua_pushboolean(L, 0);
		lua_setfield(L, -2, "__metatable");
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
#if LUA_VERSION_NUM < 502
		luaL_register(L, 0, t_cctx);
#else
		luaL_setfuncs(L, t_cctx, 0);
#endif
	}
	lua_setmetatable(L, -2);
	return 1;
}
