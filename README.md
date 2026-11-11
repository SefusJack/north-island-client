# North Island Client

A port of IGG's Wonderland Online client (2014 data and protocol), written in C++17 with SDL3. The port is based off a private decomp of `alogin.exe` (version`6.1.9`) that was built back into a runnable Delphi 5 Pascal project by using a combination of a couple of tools:

- Ghidra
- IDR
- Borland Delphi 5 Enterprise

The port removes all of the unused variables, functions, and forms from the original executable. There may be more forms removed if found to be irrelevant at a later date.

It is intended to run with the north-island-server, but if IGG's servers ever came back for some reason, the client should work with those as well. 

## Layout

| path | what |
|---|---|
| `src/engine`, `include/engine` | the game engine (essentially middleware)|
| `src/client`, `include/client` | the game client |
| `src/app` | the program entrypoint (`main`) |
| `thirdparty` | SDL3, SDL3_mixer, stb and nlohmann/json, built static |

The game data is not included. You'll need to get the files of the IGG's Wonderland Online 2014 client yourself (version `6.1.9`, dated `10/27/2014`).

## Build

You may need to run `git clone --recursive` when cloning or `git submodule update --init` if already cloned to ensure the submodules in the thirdparty folder are populated

There is a CMake preset for each platform:

```bash
cmake --preset <preset>
cmake --build --preset <preset>
```

| preset | builds on | toolchain |
|---|---|---|
| `windows-x64` | Windows | Visual Studio 18 (2026) with "Desktop development with C++" |
| `windows-arm64` | Windows | the same, plus "MSVC ARM64/ARM64EC build tools (Latest)" |
| `windows-xp` | Windows | Visual Studio 18 with VS 2017's `v141_xp` toolset and the Windows 7.1A SDK |
| `linux-x64` | Linux | GCC or Clang, Ninja |
| `macos` | macOS | Xcode's command line tools, CMake |

On Windows the program is `out/<preset>/north-island-client.exe`. On Linux and macOS it is
`build/<preset>/north-island-client`. It needs no DLLs.

### Windows XP

Add these in the Visual Studio Installer: **MSVC v141 - VS 2017 C++ x64/x86 build tools** and **C++ Windows XP Support
for VS 2017 (v141) tools**. Without Direct3D 11 and 12 in that SDK, the XP build renders with Direct3D 9, OpenGL, Vulkan
or software.

### Linux

On Ubuntu or Debian:

```bash
sudo apt install build-essential cmake ninja-build pkg-config \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev libxkbcommon-dev \
  libwayland-dev wayland-protocols libdecor-0-dev libegl1-mesa-dev libgl1-mesa-dev libgles2-mesa-dev libdrm-dev libgbm-dev \
  libvulkan-dev libasound2-dev libpulse-dev libpipewire-0.3-dev libudev-dev libdbus-1-dev libibus-1.0-dev
```

File names are looked up without regard to case, so the Windows data folder works as it is.

### macOS

Install Xcode's command line tools (`xcode-select --install`) and CMake. The preset builds one universal program for
macOS 11 and later.

## Run

Copy the program into the Wonderland Online folder (the one with the `data`, `pic` and `font` folders) and start it.

The client looks for the game in this order:

1. the folder given with `--data`
2. the same folder that the executable is in, when it has a `data` folder
3. `C:\Program Files (x86)\Wonderland Online` on Windows

## Point it at a server

Add a server by address in the server list on the login screen, or work on a copy of the game folder and replace its
`SERVER.INI` with:

```
01[Local]1
Local*127.0.0.1
```

## Settings and saved data

Settings, accounts and the log (`north-island-client.log`) are in the game folder's `user` folder. When that folder cannot
be written and `--user` is not given, the client uses `%LOCALAPPDATA%\Wonderland Online\user` instead, filled the first
time with a copy of the game's `user` folder.

The options from the System window are in `user\extra-settings.json`, and the servers you added are in
`user\saved-servers.json`.

## Mods

Mod packs go in a `mods` folder next to the program and replace the game's sounds, pictures and text without changing the
game folder. `mods\load-order.json` lists them, later ones winning.

This is largely an experimental feature. It was only tested with using higher resolution textures than the original game came with

## Command line arguments (optional)

| option | what |
|---|---|
| `--data FOLDER` | the game folder |
| `--user FOLDER` | where settings, accounts and the log go |
| `--views FOLDER` | read the window layouts from a folder |
| `--window WxH` or `last` | the window size, at least 800x600, or the size it had last |
| `--display MODE` | `windowed`, `borderless` or `fullscreen` |
| `--scale N` | pixel scale: 1, 2 or 3 |
| `--pixel-art MODE` | `off`, `scale` or `xbr` |
| `--pixel-art-skip LIST` | comma list of `pictures`, `sprites` and `text`, or `none` |
| `--hud-scale PERCENT` | 100 to 200 |
| `--fps VALUE` | `unlimited`, `vsync` or a number of frames a second |
| `--renderer NAME` | ask for a renderer: `direct3d`, `direct3d11`, `direct3d12`, `opengl`, `vulkan`, `gpu` or `software` |
| `--software` | use software rendering |
| `--background` | for test runs: open the window minimized without taking focus, and play no sound |
| `--perf` | show the frame graph |
| `--perf-log FILE` | write frame times to a CSV file |
| `--accounts FILE` | log more accounts in as the played account's party, one `account password [place]` per line |
| `--mods FOLDER` | load mod packs from another folder |
| `--no-mods` | load no mod packs |
| `--mods-report FILE` | write the list of packs and what each one replaced |

## Disclaimer

North Island Client is an unofficial, non-commercial fan project. It is not affiliated with, endorsed by, or supported by IGG or any other owner of Wonderland Online.

Wonderland Online, its name, game data, artwork, music, and other assets are the property of their respective owners. None of the game's data is included in this repository, and you must supply your own copy of the game files. The license for this project does not apply to those assets or grant any rights to them.

This project was developed in part by reverse engineering the original client. The license applies only to the original work of this project's contributors.

This software is provided "as is", without warranty of any kind. Use it at your own risk.

## Pre-alpha Testing

Though the client should be fully functional as is. The client's code is currently being play-tested by the developers through a normal playthrough of the game to find any issues or mismatches than what is expected. The most common features have already been tested, but group activites and events may have some edge cases not covered yet.  
  

