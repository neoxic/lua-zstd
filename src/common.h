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

#pragma once

#define ZSTD_STATIC_LINKING_ONLY /* Enable advanced and experimental APIs */

#include <lauxlib.h>
#include <zstd.h>
#include <zstd_errors.h>

#define MODNAME "lua-zstd"
#define VERSION "0.1.0"

#define TYPE_CCTX "zstd.CCtx"
#define TYPE_CCTXPARAMS "zstd.CCtxParams"
#define TYPE_CDICT "zstd.CDict"

#define TYPE_DCTX "zstd.DCtx"
#define TYPE_DDICT "zstd.DDict"

#define checkcctx(L, arg) (*(ZSTD_CCtx **)luaL_checkudata(L, arg, TYPE_CCTX))
#define checkcctxparams(L, arg) (*(ZSTD_CCtx_params **)luaL_checkudata(L, arg, TYPE_CCTXPARAMS))
#define checkcdict(L, arg) (*(ZSTD_CDict **)luaL_checkudata(L, arg, TYPE_CDICT))

#define checkdctx(L, arg) (*(ZSTD_DCtx **)luaL_checkudata(L, arg, TYPE_DCTX))
#define checkddict(L, arg) (*(ZSTD_DDict **)luaL_checkudata(L, arg, TYPE_DDICT))

#define checkmem(L, cond) ((void)((cond) || luaL_error(L, "not enough memory")))
#define checkrange(L, cond, arg) luaL_argcheck(L, cond, arg, "value out of range")

#if LUA_VERSION_NUM < 502
#define lua_getuservalue(L, idx) lua_getfenv(L, idx)
#define lua_setuservalue(L, idx) lua_setfenv(L, idx)
#endif

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

EXPORT int luaopen_zstd(lua_State *L);

#ifndef _WIN32
#pragma GCC visibility push(hidden)
#endif

int zstd__newCCtx(lua_State *L);
int zstd__newCCtxParams(lua_State *L);
int zstd__newCDict(lua_State *L);

int zstd__newDCtx(lua_State *L);
int zstd__newDDict(lua_State *L);

int zstd__pusherror(lua_State *L, int err);
int zstd__error(lua_State *L, size_t res);
void zstd__check(lua_State *L, size_t res);

int zstd__checkresetmode(lua_State *L, int arg);
int zstd__checkcctxparam(lua_State *L, int arg);
int zstd__checkdctxparam(lua_State *L, int arg);

#ifndef _WIN32
#pragma GCC visibility pop
#endif
