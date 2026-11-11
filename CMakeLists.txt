# north-island-client: a hand-written C++17 / SDL3 client for Wonderland Online (2014 data and protocol).
#
# Layout:
#   src/engine/<library>/, include/engine/<library>/   the engine: reusable libraries that know
#                                                      nothing about the game
#   src/client/, include/client/                       the game client
#   src/app/                                           the executable (main)
#   thirdparty/         vendored SDL3, SDL3_mixer, stb, nlohmann
#   build/              build trees
#   out/                the Release program on Windows
#
# The libraries form levels; a library may only link libraries of a lower level (checked below
# at configure time).
#
# Build: a preset for each platform (CMakePresets.json; Windows x64, ARM64 and XP, Linux, macOS):
#   cmake --preset windows-x64 && cmake --build --preset windows-x64
cmake_minimum_required(VERSION 3.15)

project(north-island-client VERSION 0.4.0 LANGUAGES C CXX)
set(NORTH_ISLAND_VERSION_LABEL "prealpha") # shown after the number; empty for a final release

# C++17: still built by VS2017's v141_xp toolset and MinGW-w64 8.1 (Windows XP targets).
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Used on its own this builds everything; as a subproject (add_subdirectory from another program that only wants the
# engine libraries) NORTH_ISLAND_BUILD_CLIENT=OFF leaves out the game itself.
option(NORTH_ISLAND_BUILD_CLIENT "Build the game client and program" ON)

# On Windows the Release program (north-island-client) goes to out/<build folder>/, such as out/x64-dev/, so each build keeps its own
if(WIN32 AND CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  get_filename_component(NORTH_ISLAND_BUILD_NAME ${CMAKE_BINARY_DIR} NAME)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/out/${NORTH_ISLAND_BUILD_NAME})
endif()

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  add_compile_options(/utf-8 /MP)
endif()
# Windows XP (toolset v141_xp, 32-bit, Windows 7.1A SDK): that SDK's COM headers do not compile with two-phase name lookup
# (/permissive-), and its headers and import libraries are what keeps the exe loadable on XP
string(REGEX MATCH "_xp$" NORTH_ISLAND_XP "${CMAKE_GENERATOR_TOOLSET}")

# ---------------------------------------------------------------- third-party (vendored, static)
# SDL3 for every build, the Windows XP one too: its renderer has the vulkan driver
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
if(MSVC)
  set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "" FORCE)
endif()
add_subdirectory(thirdparty/SDL3 EXCLUDE_FROM_ALL)

# the formats the game uses: WAVE (sound effects, login music) and MP3 (music player); and for the sound packs (mods) Ogg Vorbis
# and FLAC, with the decoders SDL_mixer carries (dr_mp3, stb_vorbis, dr_flac)
set(BUILD_SHARED_LIBS OFF)
foreach(option AIFF VOC AU GME MOD MIDI OPUS WAVPACK VORBIS_VORBISFILE VORBIS_TREMOR FLAC_LIBFLAC MP3_MPG123 TESTS EXAMPLES
               INSTALL DEPS_SHARED)
  set(SDLMIXER_${option} OFF CACHE BOOL "" FORCE)
endforeach()
foreach(option WAVE MP3 MP3_DRMP3 FLAC FLAC_DRFLAC VORBIS_STB VENDORED)
  set(SDLMIXER_${option} ON CACHE BOOL "" FORCE)
endforeach()
add_subdirectory(thirdparty/SDL3_mixer EXCLUDE_FROM_ALL)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb)

# nlohmann/json 3.11.3 (single header): extra-settings.json and the mod packs
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/nlohmann/single_include)

# Lua 5.4.7: the Electron Control's battle scripts. Built as C++ so its errors unwind the client's objects; only the core and the
# libraries the sandbox opens (no io, os, package or debug, no interpreter)
set(NORTH_ISLAND_LUA ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/lua/src)
set(NORTH_ISLAND_LUA_SOURCES)
foreach(file lapi lcode lctype ldebug ldo ldump lfunc lgc llex lmem lobject lopcodes lparser lstate lstring ltable ltm lundump lvm lzio
             lauxlib lbaselib lmathlib lstrlib ltablib)
  list(APPEND NORTH_ISLAND_LUA_SOURCES ${NORTH_ISLAND_LUA}/${file}.c)
endforeach()
set_source_files_properties(${NORTH_ISLAND_LUA_SOURCES} PROPERTIES LANGUAGE CXX)
add_library(lua STATIC ${NORTH_ISLAND_LUA_SOURCES})
target_include_directories(lua SYSTEM PUBLIC ${NORTH_ISLAND_LUA})

# ---------------------------------------------------------------- libraries
# north_island_library(NAME LEVEL BASE) builds src/BASE/NAME/**.cpp with include/BASE/NAME/**.h as a
# static library; headers are included as "NAME/File.h" (subfolders such as client/actions
# belong to their library).  BASE is "engine" or "." (the client).
set(NORTH_ISLAND_LIBRARIES)
function(north_island_library name level base)
  file(GLOB_RECURSE sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/${base}/${name}/*.cpp)
  file(GLOB_RECURSE headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/${base}/${name}/*.h)
  add_library(${name} STATIC ${sources} ${headers})
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/${base}/${name} PREFIX "Source Files" FILES ${sources})
  source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/${base}/${name} PREFIX "Header Files" FILES ${headers})
  target_include_directories(${name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/${base})
  if(MSVC)
    target_compile_options(${name} PRIVATE /W4 /permissive-)
    if(NORTH_ISLAND_XP)
      target_compile_options(${name} PRIVATE /Zc:twoPhase-)
    endif()
  else()
    target_compile_options(${name} PRIVATE -Wall -Wextra)
  endif()
  set_target_properties(${name} PROPERTIES NORTH_ISLAND_LEVEL ${level})
  set(NORTH_ISLAND_LIBRARIES ${NORTH_ISLAND_LIBRARIES} ${name} PARENT_SCOPE)
endfunction()
define_property(TARGET PROPERTY NORTH_ISLAND_LEVEL BRIEF_DOCS "dependency level" FULL_DOCS "dependency level")

# ---- the engine
# level 0: the standard library only
north_island_library(core 0 engine)
north_island_library(xml 0 engine)

# level 1: file formats and the wire protocol
north_island_library(formats 1 engine)
target_link_libraries(formats PUBLIC core PRIVATE stb)
north_island_library(protocol 1 engine)
target_link_libraries(protocol PUBLIC core)

# level 2: platform services (network, drawing, sound)
north_island_library(net 2 engine)
target_link_libraries(net PUBLIC core protocol)
if(WIN32)
  target_link_libraries(net PRIVATE ws2_32)
endif()
north_island_library(gfx 2 engine)
target_link_libraries(gfx PUBLIC core formats SDL3::SDL3-static)
north_island_library(audio 2 engine)
target_link_libraries(audio PUBLIC core SDL3_mixer::SDL3_mixer SDL3::SDL3-static)

# level 3: the user-interface toolkit
north_island_library(ui 3 engine)
target_link_libraries(ui PUBLIC core xml gfx)

if(NORTH_ISLAND_BUILD_CLIENT)
  # ---- the game
  # level 4: the game client (view models, views, server action handlers)
  north_island_library(client 4 .)
  target_link_libraries(client PUBLIC core xml formats protocol net gfx audio ui)
  target_link_libraries(client PRIVATE nlohmann_json lua)
  if(WIN32)
    # the photo's date and address lines (PhotoText) use the system's text drawing and host name
    target_link_libraries(client PRIVATE ws2_32 gdi32)
  endif()

  # The screen layouts (src/client/views/**/*.xml) are compiled into the client, found by bare file
  # name; two files of the same name in different folders would be ambiguous.
  file(GLOB_RECURSE NORTH_ISLAND_LAYOUTS CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/client/views/*.xml)
  set(NORTH_ISLAND_LAYOUT_NAMES)
  foreach(layout ${NORTH_ISLAND_LAYOUTS})
    get_filename_component(layout_name ${layout} NAME)
    string(TOLOWER ${layout_name} layout_key)
    if(layout_key IN_LIST NORTH_ISLAND_LAYOUT_NAMES)
      message(FATAL_ERROR "Two layouts in src/client/views share the file name ${layout_name}: layouts are found by file name, rename one")
    endif()
    list(APPEND NORTH_ISLAND_LAYOUT_NAMES ${layout_key})
  endforeach()
  set(NORTH_ISLAND_LAYOUT_LIST ${CMAKE_CURRENT_BINARY_DIR}/generated/layouts.txt)
  set(NORTH_ISLAND_LAYOUT_SOURCE ${CMAKE_CURRENT_BINARY_DIR}/generated/EmbeddedLayouts.cpp)
  string(REPLACE ";" "\n" NORTH_ISLAND_LAYOUT_LINES "${NORTH_ISLAND_LAYOUTS}")
  file(GENERATE OUTPUT ${NORTH_ISLAND_LAYOUT_LIST} CONTENT "${NORTH_ISLAND_LAYOUT_LINES}\n")
  add_custom_command(
    OUTPUT ${NORTH_ISLAND_LAYOUT_SOURCE}
    COMMAND ${CMAKE_COMMAND} -DOUTPUT=${NORTH_ISLAND_LAYOUT_SOURCE} -DLIST=${NORTH_ISLAND_LAYOUT_LIST}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedLayouts.cmake
    DEPENDS ${NORTH_ISLAND_LAYOUTS} ${NORTH_ISLAND_LAYOUT_LIST} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedLayouts.cmake
    COMMENT "Embedding the screen layouts")
  target_sources(client PRIVATE ${NORTH_ISLAND_LAYOUT_SOURCE})
endif()

# ---------------------------------------------------------------- dependency levels
# Every library may only link libraries of a strictly lower level (third-party targets have no
# level and are allowed everywhere).
foreach(lib ${NORTH_ISLAND_LIBRARIES})
  get_target_property(level ${lib} NORTH_ISLAND_LEVEL)
  get_target_property(deps ${lib} LINK_LIBRARIES)
  if(deps)
    foreach(dep ${deps})
      if(TARGET ${dep} AND dep IN_LIST NORTH_ISLAND_LIBRARIES)
        get_target_property(dep_level ${dep} NORTH_ISLAND_LEVEL)
        if(NOT dep_level LESS level)
          message(FATAL_ERROR "${lib} (level ${level}) must not depend on ${dep} (level ${dep_level})")
        endif()
      endif()
    endforeach()
  endif()
endforeach()

# ---------------------------------------------------------------- the program (level 5)
if(NORTH_ISLAND_BUILD_CLIENT)
  add_executable(north-island-client WIN32 src/app/main.cpp)
  if(WIN32)
    target_sources(north-island-client PRIVATE src/app/north-island-client.rc)
    set(NORTH_ISLAND_VERSION_TEXT ${PROJECT_VERSION})
    if(NORTH_ISLAND_VERSION_LABEL)
      string(APPEND NORTH_ISLAND_VERSION_TEXT "-${NORTH_ISLAND_VERSION_LABEL}")
    endif()
    configure_file(cmake/AppVersion.h.in generated/AppVersion.h @ONLY)
    target_include_directories(north-island-client PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/generated)
  endif()
  target_link_libraries(north-island-client PRIVATE client)
  target_compile_definitions(north-island-client PRIVATE SDL_MAIN_HANDLED)
  if(MSVC)
    set_target_properties(north-island-client PROPERTIES LINK_FLAGS "/ENTRY:mainCRTStartup")
  endif()
endif()
