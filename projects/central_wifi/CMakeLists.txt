cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(central_wifi)

target_sources(app PRIVATE 
src/main.c
src/watchdog_timer/watchdog_timer.c
src/ble_client/ble_fsm.c 
src/ble_client/ble_conn_control.c 
src/ble_client/ble_characteristic_control.c 
src/ble_client/ble_connection_data.c
src/flash_system/flash_system.c
src/measurements/measurements_fsm.c 
src/measurements/measurements_data_storage.c 
src/measurements/measurements_fsm_timer.c
../common/com_protocol/com_protocol.c
src/environment_control/environment_control_config.c
src/environment_control/environment_control_fsm.c
src/wifi_config/wifi_config.c
src/wifi_config/wifi_apis.c
src/cloud_data_inventory/inventory.c
src/timestamp_module/timestamp.c
src/coap_client/coap_client.c
src/coap_client/coap_fsm.c
src/coap_client/coap_message_parsing.c
)

target_include_directories(app PRIVATE 
${CMAKE_SOURCE_DIR}/../common
${CMAKE_SOURCE_DIR}/../common/com_protocol
${CMAKE_SOURCE_DIR}/src)

# Optimise for debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")