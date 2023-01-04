#!/usr/bin/env bash

cd build

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DTOOLCHAIN_PREFIX="/usr/local/gcc-arm-none-eabi/" \
    -DCMAKE_TOOLCHAIN_FILE="../cmake/toolchain-arm-none-eabi.cmake" \
    -DAPPLICATION="LoRaMac" \
    -DSUB_PROJECT="hoymiles-data" \
    -DCLASSB_ENABLED="ON" \
    -DACTIVE_REGION="LORAMAC_REGION_EU868" \
    -DREGION_EU868="ON" \
    -DREGION_US915="OFF" \
    -DREGION_CN779="OFF" \
    -DREGION_EU433="OFF" \
    -DREGION_AU915="OFF" \
    -DREGION_AS923="OFF" \
    -DREGION_CN470="OFF" \
    -DREGION_KR920="OFF" \
    -DREGION_IN865="OFF" \
    -DREGION_RU864="OFF" \
    -DBOARD="HeltecLoRa151" \
    -DMBED_RADIO_SHIELD="SX1276MB1MAS" \
    -DPERIPHERAL_BH1750="ON" \
    -DPERIPHERAL_BMP280="ON" \
    -DPERIPHERAL_NRF24L01="ON" \
    -DPERIPHERAL_SSD1306="ON" \
    -DPERIPHERAL_VL53L1X="ON" \
    -DUSE_RADIO_DEBUG="ON" ..

make

cd ..
