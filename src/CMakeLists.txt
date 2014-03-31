rock_library(EthernetDrivers
    SOURCES Dummy.cpp
    HEADERS Dummy.hpp)

rock_executable(EthernetDrivers_bin Main.cpp
    DEPS EthernetDrivers)

