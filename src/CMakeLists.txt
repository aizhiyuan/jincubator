#自定义库头文件
include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/include/openssl ${CMAKE_SOURCE_DIR}/include/wiringPi ${CMAKE_SOURCE_DIR}/zlog)

#自定义库文件
link_directories( ${CMAKE_SOURCE_DIR}/lib/wiringPi ${CMAKE_BINARY_DIR}/zlog)
link_libraries(libzlog.a)

#目录下所有源文件
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC_FILES)

#生成执行文件
add_executable(${PROJECT_NAME} ${SRC_FILES})    

#标准库
# target_link_libraries(${PROJECT_NAME} zlog wiringPi crypto pthread m dl ${EXTRA_LIBS})
target_link_libraries(${PROJECT_NAME} zlog pthread crypto m dl ${EXTRA_LIBS})