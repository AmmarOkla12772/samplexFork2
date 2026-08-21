#  Copyright (c) 2026 Eclipse ThreadX contributors
# 
#  This program and the accompanying materials are made available 
#  under the terms of the MIT license which is available at
#  https://opensource.org/license/mit.
# 
#  SPDX-License-Identifier: MIT

add_library(nucleo_bsp STATIC
    src/bsp_board.c
    src/bsp_led.c
    src/bsp_console.c
)

# Set the public and private include paths
target_include_directories(nucleo_bsp PUBLIC
    include
    ${CMAKE_CURRENT_LIST_DIR}/../../../../../bsp/include
)

# Dependencies on ThreadX and STM32CubeF4
target_link_libraries(nucleo_bsp PRIVATE
    azrtos::threadx
    stm32cubef4
)
