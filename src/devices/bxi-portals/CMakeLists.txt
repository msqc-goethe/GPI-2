cmake_minimum_required(VERSION 3.5)

find_package(Portals REQUIRED)

add_library(
	bxi-device SHARED "GPI2_BXI.c" "GPI2_BXI_ATOMIC.c" "GPI2_BXI_GRP.c" "GPI2_BXI_IO.c"
             "GPI2_BXI_PASSIVE.c" "GPI2_BXI_SEG.c"
)

target_include_directories(bxi-device PUBLIC "../include" "${CMAKE_SOURCE_DIR}/src" "${CMAKE_SOURCE_DIR}/src/include")
target_link_libraries(bxi-device PRIVATE "Portals::Portals")
target_compile_features(bxi-device PRIVATE c_std_11)
target_compile_definitions(bxi-device PRIVATE "GPI2_DEVICE_BXI" "$<$<CONFIG:DEBUG>:DEBUG>")
