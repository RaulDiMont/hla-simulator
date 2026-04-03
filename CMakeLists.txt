cmake_minimum_required(VERSION 3.10)
project(HLASimulator)

# Use C++17 so OpenRTI headers use std::unique_ptr instead of std::auto_ptr
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenRTI installed on the system
find_package(PkgConfig REQUIRED)
pkg_check_modules(RTI1516E REQUIRED rti1516e)

# Include OpenRTI headers
include_directories(${RTI1516E_INCLUDE_DIRS})
link_directories(${RTI1516E_LIBRARY_DIRS})

# Define the executable and its source files
add_executable(aircraft_simulator
    src/main.cpp
    src/AircraftFederate.cpp
    src/MonitorFederate.cpp
)

# Link against OpenRTI libraries including logical time support
target_link_libraries(aircraft_simulator
    ${RTI1516E_LIBRARIES}
    rti1516e
    fedtime1516e
)