cmake_minimum_required(VERSION 3.25)

project(PaymentTransaction LANGUAGES CXX)

set(CMAKE_CXX_FLAGS_INIT "-Wall -std=c++17")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")