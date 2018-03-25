cmake_minimum_required(VERSION 3.5)

ADD_EXECUTABLE(sotest spirit.cpp SpiritParsersBridge.cpp KiKadParse.cpp temp.cpp)
#SET(CMAKE_CXX_COMPILER g++-7)
#SET(CMAKE_CXX_COMPILER clang++-5.0)
SET(BOOST_DIR /home/sehe/custom/boost_1_66_0)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -DDEBUG \
    -isystem ${BOOST_DIR}/ \
    -Wl,-rpath,${BOOST_DIR}/stage/lib \
    -L ${BOOST_DIR}/stage/lib \
    -Wall -Wextra -pedantic \
    -std=c++17 \
    -Wno-unused-function \
    -Wno-unused \
    -fsanitize=address,undefined \
    -g -O3 -pthread -march=native"
    )
    #-ggdb -fno-omit-frame-pointer -O0 -pthread -march=native"

TARGET_LINK_LIBRARIES(sotest
    #boost_system
)
