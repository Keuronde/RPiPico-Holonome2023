cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()



add_executable(test
test.c
Asser_Moteurs.c
spi_nb.c
gyro.c
Temps.c
Servomoteur.c
gyro_L3GD20H.c
gyro_ADXRS453.c
QEI.c
APDS_9960.c
Moteurs.c
)

pico_generate_pio_header(test ${CMAKE_CURRENT_LIST_DIR}/quadrature_encoder.pio)

add_definitions(-DGYRO_ADXRS453)
pico_enable_stdio_usb(test 1)
pico_enable_stdio_uart(test 1)
pico_add_extra_outputs(test)
target_link_libraries(test pico_stdlib hardware_spi hardware_pwm hardware_structs hardware_pio pico_multicore)

add_custom_target(Flash
    DEPENDS test
    COMMAND sudo picotool load ${PROJECT_BINARY_DIR}/test.uf2
)