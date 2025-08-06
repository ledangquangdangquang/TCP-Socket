cmake_minimum_required(VERSION 3.14)

project(huhuClientPi LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Network REQUIRED)  # hoặc Qt5 nếu bạn dùng Qt5

add_executable(huhuClientPi
  main.cpp
  SimpleClient.h SimpleClient.cpp
)
# Link thư viện Qt
target_link_libraries(huhuClientPi
    Qt6::Core
    Qt6::Network
)

install(TARGETS huhuClientPi
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
