#CMakeLists.tsti
cmake_minimum_required(VERSION 3.4.1)

project(MQTTClient)
set(SRC_LIST ./src/MQTTClient.cpp
    ./src/SERVER/HttpServer.cpp
    ./src/FACTORY/HttpHandlerFactory.cpp
    ./src/BAL/BALHandler.cpp
    ./src/common/Common.cpp
    ./src/model/Model.cpp
    ${CMAKE_SOURCE_DIR}/src/rknn/detector.cpp
    ${CMAKE_SOURCE_DIR}/src/rknn/postprocess.cc
	)


set(MQTT_INCLUDE_DIRS "/usr/local/include/")
set(POCO_INCLUDE_DIRS "/usr/local/include/Poco/")
set(CJSON_INCLUDE_DIRS "/usr/local/include/cjson")
set(MQTT_LIBRARIES "/usr/lib64") 
set(CJSON_LIBRARIES "/usr/local/lib64/") 

#set(EXECUTABLE_OUTPUT_PATH /app/)

find_package(Poco REQUIRED COMPONENTS Foundation Net XML Util) # add other components here
# check c++11 / c++0x
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11 " COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "-std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "-std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()




# 加入一个配置头文件，用于处理 CMake 对源码的设置
configure_file (
  "${MQTTClient_SOURCE_DIR}/Config.ini.in"
  "${MQTTClient_BINARY_DIR}/Config.ini"
  )

  
configure_file (
    "${MQTTClient_SOURCE_DIR}/Init.dat"
    "${MQTTClient_BINARY_DIR}/Init.dat"
    )



# 加入gdb调试 
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")


include_directories( ${POCO_INCLUDE_DIRS}) 

include_directories( ${CJSON_INCLUDE_DIRS}) 


include_directories( ${MQTT_INCLUDE_DIRS}) 


# target_link_libraries(MQTTClient ${MQTT_LIBRARIES}/libmosquitto.so)
# target_link_libraries(MQTTClient ${MQTT_LIBRARIES}/libPocoFoundation.so)
# target_link_libraries(MQTTClient ${MQTT_LIBRARIES}/libPocoJSON.so)
# target_link_libraries(MQTTClient ${MQTT_LIBRARIES}/libPocoUtil.so)
# target_link_libraries(MQTTClient ${MQTT_LIBRARIES}/libPocoNet.so)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# skip 3rd-party lib dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--allow-shlib-undefined")

# rknn api
set(RKNN_API_PATH ${CMAKE_SOURCE_DIR}/runtime/RK3588/${CMAKE_SYSTEM_NAME}/librknn_api)

set(LIB_ARCH aarch64)
set(RKNN_RT_LIB ${RKNN_API_PATH}/${LIB_ARCH}/librknnrt.so)

include_directories(${RKNN_API_PATH}/include)
include_directories(${CMAKE_SOURCE_DIR}/src/rknn/3rdparty)

# opencv
#set(OpenCV_DIR ${CMAKE_SOURCE_DIR}/src/rknn/3rdparty/opencv/opencv-linux-aarch64/share/OpenCV)
find_package(OpenCV REQUIRED)

#rga
set(RGA_PATH ${CMAKE_SOURCE_DIR}/src/rknn/3rdparty/rga/RK3588)

set(LIB_ARCH aarch64)
set(RGA_LIB ${RGA_PATH}/lib/Linux//${LIB_ARCH}/librga.so)
include_directories( ${RGA_PATH}/include)
include_directories( ${CMAKE_SOURCE_DIR}/src/rknn/include)

add_executable(MQTTClient ${SRC_LIST})

#message(STATUS "${OpenCV_LIBS}")

#target_link_libraries(MQTTClient ${MQTT_LIBRARIES})
target_link_libraries(MQTTClient 
                                ${OpenCV_LIBS}
                                ${MQTT_LIBRARIES}/libmosquitto.so 
                                /usr/local/lib/libPocoFoundation.so 
                                /usr/local/lib/libPocoJSON.so 
                                /usr/local/lib/libPocoUtil.so 
                                /usr/local/lib/libPocoNet.so
                                /usr/lib64/libcjson.so
                                ${RKNN_RT_LIB}
                                ${RGA_LIB}
                                )



