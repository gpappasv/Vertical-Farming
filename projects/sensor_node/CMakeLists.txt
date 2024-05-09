# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

# mcuboot config file 
set(mcuboot_CONF_FILE
	prj.conf
    ${CMAKE_CURRENT_LIST_DIR}/mcuboot.conf
)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor_node)

target_sources(app PRIVATE 
main/main.c 
temp_hum/temperature_humidity_sensor.c 
ble_server/ble_measurement_service.c 
ble_server/ble_conn_control.c 
ble_server/ble_configure_service.c 
battery_measurement/battery_measurement.c
soil_moisture/soil_moisture.c
adc_calibration/adc_calibration.c
flash_system/flash_system.c 
timer_module/timer_module.c
watchdog_timer/watchdog_timer.c
gpio/gpioif.c
)

# Disable optimisations
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
# Enable the following option for the SW to send emulated sensor values
#
target_compile_definitions(app PRIVATE SW_SENSOR_EMULATION_MODE)
#target_compile_definitions(app PRIVATE BME_280)