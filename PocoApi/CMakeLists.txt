cmake_minimum_required(VERSION 3.12)
project(PocoRestApi)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include vcpkg toolchain file if not already set
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Users/ebachlitzanakis/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Find POCO package
find_package(Poco CONFIG REQUIRED Foundation Net JSON Util)

# Add executable
add_executable(${PROJECT_NAME} 
    src/main.cpp
    src/handlers/PostHandler.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Poco::Foundation
    Poco::Net
    Poco::JSON
    Poco::Util)
