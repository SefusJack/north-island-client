# Lua 5.4.7

From https://www.lua.org/ftp/lua-5.4.7.tar.gz (SHA-256 9fbf5e28ef86c69858f6d3d34eccc32e911c1a28b4120ff3e84aaa70cfbf1e30), unchanged.

Only the files the client builds are kept: the core, the auxiliary library and the base, math, string and table libraries, with
all of the headers. The interpreter, the compiler and the io, os, package, debug, utf8 and coroutine libraries are left out
(see the Lua part of the top CMakeLists.txt and the Electron Control's script host, client/models/remote/RemoteScript).

MIT licence: see LICENSE.
