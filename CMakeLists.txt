cmake_minimum_required(VERSION 2.8)

set (CMAKE_MODULE_PATH
    "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/../modules/cmake"
)

include(Arduino)

add_executable(yihua898 yihua898D.cpp TM1628.cpp)

target_link_libraries(yihua898
  arduino
)

add_custom_command(TARGET yihua898
  POST_BUILD COMMAND avr-size yihua898)

add_executable(test_led test_led.cpp TM1628.cpp)

target_link_libraries(test_led
  arduino
)

add_custom_command(TARGET test_led
  POST_BUILD COMMAND avr-size test_led)

add_executable(test_tm1628 test_tm1628.cpp TM1628.cpp)

target_link_libraries(test_tm1628
  arduino
)

add_custom_command(TARGET test_tm1628
  POST_BUILD COMMAND avr-size test_tm1628)

add_executable(test_io test_io.cpp TM1628.cpp)

target_link_libraries(test_io
  arduino
)

add_custom_command(TARGET test_io
  POST_BUILD COMMAND avr-size test_io)
