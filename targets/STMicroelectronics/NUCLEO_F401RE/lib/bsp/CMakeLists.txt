# Copyright (c) 2026 Eclipse ThreadX contributors
#
# SPDX-License-Identifier: MIT

add_library(bsp STATIC
    src/bsp_board.c
    src/bsp_led.c
    src/bsp_console.c
)

target_include_directories(bsp
    PUBLIC
        include
        ${SAMPLEX_ROOT_DIR}/bsp/include
)

target_link_libraries(bsp
    PUBLIC
        stm32cubef4
)
