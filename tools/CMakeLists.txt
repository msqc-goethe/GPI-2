cmake_minimum_required(VERSION 3.5)

find_package(Portals REQUIRED)

add_executable(query_bxi_limits "query_bxi_limits.c")
target_compile_features(query_bxi_limits PRIVATE c_std_11)
target_link_libraries(query_bxi_limits PRIVATE  "Portals::Portals")
include(GNUInstallDirs)
install(TARGETS query_bxi_limits DESTINATION ${CMAKE_INSTALL_BINDIR}) 
