cmake_minimum_required(VERSION 3.10)
project(DeribitOrderManagement)

# Set C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# If using vcpkg, include the toolchain file (uncomment if applicable)
set(CMAKE_TOOLCHAIN_FILE "C:/Users/aakas/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Find required packages
find_package(CURL REQUIRED)
find_package(websocketpp REQUIRED CONFIG)  # Ensure CONFIG mode is used
find_package(Boost REQUIRED COMPONENTS system thread)  # Ensure Boost::system and Boost::thread are available
find_package(RapidJSON REQUIRED) 

# Include directories (limit scope to target)
add_executable(deribit_order_management)

target_include_directories(deribit_order_management PRIVATE 
    ${PROJECT_SOURCE_DIR}/include  # Your project's include directory
    ${Boost_INCLUDE_DIRS}          # Boost headers
    ${CURL_INCLUDE_DIRS}           # cURL include directory
)

# Source files
file(GLOB SOURCES "src/*.cpp")  # Include all .cpp files in src/

# Define executable with sources
target_sources(deribit_order_management PRIVATE ${SOURCES})

# Link libraries
target_link_libraries(deribit_order_management PRIVATE 
    CURL::libcurl              # Link cURL library
    websocketpp::websocketpp   # Link websocketpp
    Boost::system              # Link Boost::system
    Boost::thread              # Link Boost::thread
)

# Compiler options for better debugging and warnings
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(deribit_order_management PRIVATE -Wall -Wextra -pedantic)
endif()

# Print message to confirm successful configuration
message(STATUS "DeribitOrderManagement project configured successfully!")
