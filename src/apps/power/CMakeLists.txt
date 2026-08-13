##
## \file  CMakeLists.txt
##
## \brief ping-pong example CMakeLists.txt
##
## The Clear BSD License
## Copyright Semtech Corporation 2021. All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted (subject to the limitations in the disclaimer
## below) provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Semtech corporation nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
##
## NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
## THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
## CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
## NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
## PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
cmake_minimum_required(VERSION 3.10)
project(power)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow selection of region
option(REGION_EU868 "Region EU868" ON)
option(REGION_US915 "Region US915" OFF)
option(REGION_CN779 "Region CN779" OFF)
option(REGION_EU433 "Region EU433" OFF)
option(REGION_AU915 "Region AU915" OFF)
option(REGION_AS923 "Region AS923" OFF)
option(REGION_CN470 "Region CN470" OFF)
option(REGION_KR920 "Region KR920" OFF)
option(REGION_IN865 "Region IN865" OFF)
option(REGION_RU864 "Region RU864" OFF)
set(REGION_LIST REGION_EU868 REGION_US915 REGION_CN779 REGION_EU433 REGION_AU915 REGION_AS923 REGION_CN470 REGION_KR920 REGION_IN865 REGION_RU864)

# Allow switching of modulation
set(MODULATION_LIST LORA FSK)
set(MODULATION LORA CACHE STRING "Default modulation is LoRa")
set_property(CACHE MODULATION PROPERTY STRINGS ${MODEM_LIST})

# Select the fixed role of this firmware image.
set(POWER_ROLE send CACHE STRING "Power application role (send or receive)")
set_property(CACHE POWER_ROLE PROPERTY STRINGS send receive)

if(POWER_ROLE STREQUAL send)
    set(POWER_TARGET power_send)
    set(POWER_ROLE_DEFINITION POWER_ROLE_SEND)
    set(POWER_ROLE_SOURCE "${CMAKE_CURRENT_LIST_DIR}/${BOARD}/power_sender.c")
elseif(POWER_ROLE STREQUAL receive)
    set(POWER_TARGET power_receive)
    set(POWER_ROLE_DEFINITION POWER_ROLE_RECEIVE)
    set(POWER_ROLE_SOURCE "${CMAKE_CURRENT_LIST_DIR}/${BOARD}/power_receiver.c")
else()
    message(FATAL_ERROR "Invalid POWER_ROLE '${POWER_ROLE}'. Use 'send' or 'receive'.")
endif()

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

set(POWER_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/${BOARD}/main.c"
    "${CMAKE_CURRENT_LIST_DIR}/${BOARD}/power_protocol.c"
    "${CMAKE_CURRENT_LIST_DIR}/${BOARD}/power_radio.c"
    "${POWER_ROLE_SOURCE}"
)

if(POWER_ROLE STREQUAL send)
    list(APPEND POWER_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${BOARD}/sdm120.c")
endif()

add_executable(${POWER_TARGET}
                            ${POWER_SOURCES}
                            $<TARGET_OBJECTS:system>
                            $<TARGET_OBJECTS:radio>
                            $<TARGET_OBJECTS:peripherals>
                            $<TARGET_OBJECTS:${BOARD}>
)

# Loops through all regions and add compile time definitions for the enabled ones.
foreach( REGION ${REGION_LIST} )
    if(${REGION})
        target_compile_definitions(${POWER_TARGET} PUBLIC -D"${REGION}")
    endif()
endforeach()

if(MODULATION STREQUAL LORA)
    target_compile_definitions(${POWER_TARGET} PRIVATE USE_MODEM_LORA)
elseif(MODULATION STREQUAL FSK)
    target_compile_definitions(${POWER_TARGET} PRIVATE USE_MODEM_FSK)
endif()

# Add compile time definition for the mbed shield if set.
target_compile_definitions(${POWER_TARGET} PUBLIC -D${MBED_RADIO_SHIELD})

target_compile_definitions(${POWER_TARGET} PUBLIC
    ${POWER_ROLE_DEFINITION}
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_COMPILE_DEFINITIONS>>
)

target_include_directories(${POWER_TARGET} PUBLIC
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET ${POWER_TARGET} PROPERTY C_STANDARD 11)

target_link_libraries(${POWER_TARGET} m)

#---------------------------------------------------------------------------------------
# Debugging and Binutils
#---------------------------------------------------------------------------------------

include(gdb-helper)
include(binutils-arm-none-eabi)

# Generate debugger configurations
generate_run_gdb_stlink(${POWER_TARGET})
generate_run_gdb_openocd(${POWER_TARGET})
generate_vscode_launch_openocd(${POWER_TARGET})

# Print section sizes of target
print_section_sizes(${POWER_TARGET})

# Create output in hex and binary format
create_bin_output(${POWER_TARGET})
create_hex_output(${POWER_TARGET})
