# Copyright (c) 2020 Apfel
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software.
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required(VERSION 3.2)
project(Ongaku C)

set(CMAKE_C_STANDARD 11)

if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
endif()

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message("-- > Configuration for third-party packages set to \"${CMAKE_BUILD_TYPE}\".")

if (NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/ThirdParty)
    message(WARNING "The ThiryParty folder is missing, you might encounter errors due to missing packages; please check out the README file.")
endif()

add_definitions(-DWIN32_LEAN_AND_MEAN -DUNICODE -D_CRT_SECURE_NO_DEPRECATE)

add_executable(Ongaku
    ${CMAKE_CURRENT_LIST_DIR}/Sources/Manifest.rc

    ${CMAKE_CURRENT_LIST_DIR}/Sources/Main.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3 /WX /Wv:18")
set_target_properties(Ongaku PROPERTIES LINK_FLAGS "/subsystem:Windows")

target_link_options(Ongaku
    PUBLIC "/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'"
)

# Discord Rich Presence

target_include_directories(Ongaku PUBLIC ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/Discord/${CMAKE_BUILD_TYPE}/include)
target_link_libraries(Ongaku PUBLIC ${CMAKE_CURRENT_LIST_DIR}/ThirdParty/Discord/${CMAKE_BUILD_TYPE}/lib/discord-rpc.lib)

# iTunes COM SDK

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/Modules/iTunes-Wrapper)
add_dependencies(Ongaku iTunes-Wrapper)
target_include_directories(Ongaku PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Modules/iTunes-Wrapper)
target_link_libraries(Ongaku PUBLIC iTunes-Wrapper)
