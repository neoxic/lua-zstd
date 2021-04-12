Zstandard module for Lua
========================

[lua-zstd] is a binding to the [Zstandard] C library for Lua.

Check out the [API Reference] for features.


Dependencies
------------

+ lua >= 5.1 (or luajit)
+ libzstd >= 1.4.9


Building and installing with LuaRocks
-------------------------------------

To build and install, run:

    luarocks make

To install the latest release using [luarocks.org], run:

    luarocks install lua-zstd


Building and installing with CMake
----------------------------------

To build and install, run:

    cmake .
    make
    make install

To build for a specific Lua version, set `USE_LUA_VERSION`. For example:

    cmake -D USE_LUA_VERSION=5.1 .

or for LuaJIT:

    cmake -D USE_LUA_VERSION=jit .

To build in a separate directory, replace `.` with a path to the source.


[lua-zstd]: https://github.com/neoxic/lua-zstd
[Zstandard]: https://github.com/facebook/zstd
[luarocks.org]: https://luarocks.org
[API Reference]: doc/main.md
