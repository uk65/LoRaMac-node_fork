##
## \file  CMakeLists.txt
##
## \brief peripherals CMakeLists.txt
##
## The Clear BSD License
## Copyright Semtech Corporation 2021. All rights reserved.
## Copyright Stackforce 2021. All rights reserved.
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
project(peripherals)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow selection of region
option(PERIPHERAL_BH1750   "BH1750 luminosity sensor"                      OFF)
option(PERIPHERAL_BMP280   "BMP280 temperature, humidity, pressure sensor" OFF)
option(PERIPHERAL_NRF24L01 "RF24 radio"                                    OFF)
option(PERIPHERAL_SSD1360  "SSD1360 LED driver"                            OFF)
option(PERIPHERAL_VL53L1X  "VL53L1X laser distance flight sensor"          OFF)
set(PERIPHERAL_LIST PERIPHERAL_BH1750 PERIPHERAL_BMP280 PERIPHERAL_NRF24L01 PERIPHERAL_SSD1360 PERIPHERAL_VL53L1X)


#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

if(${SECURE_ELEMENT} MATCHES SOFT_SE)
    file(GLOB ${PROJECT_NAME}_SOURCES
        "*.c"
        "soft-se/*.c"
    )
else()
    if(${SECURE_ELEMENT} MATCHES LR11XX_SE)
        if (${RADIO} MATCHES lr11xx)
            file(GLOB ${PROJECT_NAME}_SOURCES "*.c" "lr11xx-se/*.c")
        else()
            message(FATAL_ERROR "LR11XX_SE secure elemeent can only be used when LR11XX radio is selected.")
        endif()
    elseif((${SECURE_ELEMENT} MATCHES ATECC608A_TNGLORA_SE))
        file(GLOB ${PROJECT_NAME}_SOURCES "*.c"
            "atecc608a-tnglora-se/*.c"
            "atecc608a-tnglora-se/cryptoauthlib/lib/*.c"
            "atecc608a-tnglora-se/cryptoauthlib/lib/basic/*.c"
            "atecc608a-tnglora-se/cryptoauthlib/lib/crypto/*.c"
            "atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes/*.c"
            "atecc608a-tnglora-se/cryptoauthlib/lib/hal/atca_hal.c"
            "atecc608a-tnglora-se/cryptoauthlib/lib/host/*.c"
        )
    else()
        message(FATAL_ERROR "No secure-element selected.")
    endif()
endif()

if(PERIPHERAL_BH1750 STREQUAL ON)
set(${PROJECT_NAME}_SOURCES
    ${${PROJECT_NAME}_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/bh1750/bh1750.c"
)
endif()
if(PERIPHERAL_BMP280 STREQUAL ON)
set(${PROJECT_NAME}_SOURCES
    ${${PROJECT_NAME}_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/bmp280/bmp280.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/bmp280/bmp280_driver.c"
)
endif()
if(PERIPHERAL_NRF24L01 STREQUAL ON)
set(${PROJECT_NAME}_SOURCES
    ${${PROJECT_NAME}_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/nrf24l01/nrf24l01.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/nrf24l01/rf24.cpp"
)
endif()
if(PERIPHERAL_SSD1306 STREQUAL ON)
set(${PROJECT_NAME}_SOURCES
    ${${PROJECT_NAME}_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/ssd1306/ssd1306.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ssd1306/ssd1306_fonts.c"
)
endif()
if(PERIPHERAL_VL53L1X STREQUAL ON)
set(${PROJECT_NAME}_SOURCES
    ${${PROJECT_NAME}_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/vl53l1x/vl53l1x.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/vl53l1x/vl53l1x_api.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/vl53l1x/vl53l1x_calibration.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/vl53l1x/vl53l1_platform.c"
)
endif()

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

if(${SECURE_ELEMENT_PRE_PROVISIONED} MATCHES ON)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSECURE_ELEMENT_PRE_PROVISIONED)
endif()

if(${SECURE_ELEMENT} MATCHES SOFT_SE)
    target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/soft-se)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSOFT_SE)
else()
    if(${SECURE_ELEMENT} MATCHES LR11XX_SE)
        if(${RADIO} MATCHES lr11xx)
            target_include_directories( ${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx-se)
            target_compile_definitions(${PROJECT_NAME} PUBLIC -DLR11XX)
        endif()
    elseif((${SECURE_ELEMENT} MATCHES ATECC608A_TNGLORA_SE))
        target_include_directories( ${PROJECT_NAME} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se/cryptoauthlib/lib
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se/cryptoauthlib/lib/basic
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se/cryptoauthlib/lib/crypto
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se/cryptoauthlib/lib/hal
            ${CMAKE_CURRENT_SOURCE_DIR}/atecc608a-tnglora-se/cryptoauthlib/lib/host
        )
    endif()
endif()

# Loops through all peripherals and add compile time definitions for the enabled ones.
foreach(PERIPHERAL ${PERIPHERAL_LIST})
    if(${PERIPHERAL})
        target_compile_definitions(${PROJECT_NAME} PUBLIC -D"${PERIPHERAL}")
        target_compile_definitions(${PROJECT_NAME} PUBLIC -D"${PERIPHERAL}_LORAMAC_NODE_IMPL")
    endif()
endforeach()


target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:mac,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
