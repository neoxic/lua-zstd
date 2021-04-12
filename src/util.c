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

int zstd__pusherror(lua_State *L, int err) {
	if (!err) return 0;
	lua_pushnil(L);
	lua_pushfstring(L, "zstd error %d (%s)", err, ZSTD_getErrorString(err));
	return 1;
}

int zstd__error(lua_State *L, size_t res) {
	return zstd__pusherror(L, ZSTD_getErrorCode(res));
}

void zstd__check(lua_State *L, size_t res) {
	if (zstd__error(L, res)) lua_error(L);
}

static const char *const s_reset[] = {
	"session",
	"params",
	"all",
	0
};

int zstd__checkresetmode(lua_State *L, int arg) {
	return luaL_checkoption(L, arg, s_reset[0], s_reset) + 1;
}
