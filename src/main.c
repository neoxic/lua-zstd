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

static int getlen(lua_State *L, const void *buf, size_t slen, size_t *dlen) {
	unsigned long long size = ZSTD_getFrameContentSize(buf, slen);
	if (size == ZSTD_CONTENTSIZE_ERROR) {
		lua_pushnil(L);
		lua_pushliteral(L, "invalid frame data");
		return 0;
	}
	if (size == ZSTD_CONTENTSIZE_UNKNOWN) {
		lua_pushnil(L);
		lua_pushliteral(L, "unknown content size");
		return 0;
	}
	*dlen = size;
	return 1;
}

/* ARG: data, [level]
** RES: data | nil, error */
static int f_compress(lua_State *L) {
	size_t slen;
	const void *src = luaL_checklstring(L, 1, &slen);
	int level = luaL_optinteger(L, 2, 0);
	size_t dlen = ZSTD_compressBound(slen);
	void *dst;
	checkrange(L, level >= ZSTD_minCLevel() && level <= ZSTD_maxCLevel(), 2);
	if (zstd__error(L, dlen = ZSTD_compress(dst = lua_newuserdata(L, dlen), dlen, src, slen, level))) return 2;
	lua_pushlstring(L, dst, dlen);
	return 1;
}

/* ARG: data
** RES: data | nil, error */
static int f_decompress(lua_State *L) {
	size_t slen, dlen;
	const void *src = luaL_checklstring(L, 1, &slen);
	void *dst;
	if (!getlen(L, src, slen, &dlen)) return 2;
	if (zstd__error(L, dlen = ZSTD_decompress(dst = lua_newuserdata(L, dlen), dlen, src, slen))) return 2;
	lua_pushlstring(L, dst, dlen);
	return 1;
}

/* ARG: data
** RES: true | false */
static int f_isFrame(lua_State *L) {
	size_t len;
	const void *buf = luaL_checklstring(L, 1, &len);
	lua_pushboolean(L, ZSTD_isFrame(buf, len));
	return 1;
}

/* ARG: data
** RES: size | nil, error */
static int f_getFrameContentSize(lua_State *L) {
	size_t slen, dlen;
	const void *buf = luaL_checklstring(L, 1, &slen);
	if (!getlen(L, buf, slen, &dlen)) return 2;
	lua_pushinteger(L, dlen);
	return 1;
}

static const luaL_Reg l_zstd[] = {
	{"compress", f_compress},
	{"decompress", f_decompress},
	{"isFrame", f_isFrame},
	{"getFrameContentSize", f_getFrameContentSize},
	{"CCtx", zstd__newCCtx},
	{"CCtxParams", zstd__newCCtxParams},
	{"CDict", zstd__newCDict},
	{"DCtx", zstd__newDCtx},
	{"DDict", zstd__newDDict},
	{0, 0}
};

int luaopen_zstd(lua_State *L) {
#if LUA_VERSION_NUM < 502
	luaL_register(L, "zstd", l_zstd);
#else
	luaL_newlib(L, l_zstd);
#endif
	lua_pushliteral(L, MODNAME);
	lua_setfield(L, -2, "_NAME");
	lua_pushliteral(L, VERSION);
	lua_setfield(L, -2, "_VERSION");
	return 1;
}
