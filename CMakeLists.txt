cmake_minimum_required(VERSION 3.10)
project(DeribitOrderManagement)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
cmake_policy(SET CMP0167 NEW)

# Set output directory for binaries

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# If using vcpkg, include the toolchain file
set(CMAKE_TOOLCHAIN_FILE "C:/Users/aakas/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()
# Add vcpkg prefix path
set(CMAKE_PREFIX_PATH "C:/Users/aakas/vcpkg/installed/x64-windows" ${CMAKE_PREFIX_PATH})

# Find required packages
find_package(websocketpp REQUIRED CONFIG)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(RapidJSON REQUIRED)
find_package(CURL REQUIRED)

# Debugging messages for dependencies
message(STATUS "Boost found: ${Boost_FOUND}")
message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
message(STATUS "CURL found: ${CURL_FOUND}")
message(STATUS "CURL include dirs: ${CURL_INCLUDE_DIRS}")
message(STATUS "websocketpp found: ${websocketpp_FOUND}")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Add the executable and source files
add_executable(deribit_order_management
    src/main.cpp
    src/order_manager.cpp
    src/utils.cpp
    src/websocket_handler.cpp
)

# Link libraries
target_link_libraries(deribit_order_management PRIVATE 
    CURL::libcurl              # cURL library
    websocketpp::websocketpp   # websocketpp
    Boost::system              # Boost::system
    Boost::thread              # Boost::thread
)

# Compiler options for better debugging and warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(deribit_order_management PRIVATE -Wall -Wextra -pedantic)
endif()

# Print message to confirm successful configuration
message(STATUS "DeribitOrderManagement project configured successfully!")