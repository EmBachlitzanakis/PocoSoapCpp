cmake_minimum_required(VERSION 3.10)
project(soap_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_PREFIX_PATH "C:/Users/ebachlitzanakis/vcpkg/installed/x64-windows")
set(CMAKE_TOOLCHAIN_FILE "C:/Users/ebachlitzanakis/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Find POCO package
find_package(Poco REQUIRED Foundation XML Net)

# Add executable
add_executable(soap_service 
    main.cpp
    NameService.hpp
    NameService.cpp
)
#second approach to add executable
# set(HEADERS
#     NameService.hpp
# )

# set(SOURCES
#     main.cpp
#     NameService.cpp
# )

# add_executable(soap_service 
#     ${HEADERS}
#     ${SOURCES}
# )

# Link POCO libraries
target_link_libraries(soap_service
    PRIVATE
    Poco::Foundation
    Poco::Net
    Poco::XML
)
