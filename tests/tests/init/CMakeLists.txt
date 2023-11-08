cmake_minimum_required(VERSION 3.5)

SET(GPI2_LIB "${CMAKE_BINARY_DIR}/src/libGPI-2.a")

add_executable(proc_init_timeout.bin "proc_init_timeout.c")
target_link_libraries(proc_init_timeout.bin PUBLIC "${GPI2_LIB}")

add_executable(cmd_line_args.bin "cmd_line_args.c")
target_link_libraries(cmd_line_args.bin PUBLIC "${GPI2_LIB}")

add_executable(kill_procs.bin "kill_procs.c")
target_link_libraries(kill_procs.bin PUBLIC "${GPI2_LIB}")

add_executable(print_to.bin "print_to.c")
target_link_libraries(print_to.bin PUBLIC "${GPI2_LIB}")

add_executable(null_ptrs.bin "null_ptrs.c")
target_link_libraries(null_ptrs.bin PUBLIC "${GPI2_LIB}")

add_executable(numa_check.bin "numa_check.c")
target_link_libraries(numa_check.bin PUBLIC "${GPI2_LIB}")

add_executable(strong_sym.bin "strong_sym.c")
target_link_libraries(strong_sym.bin PUBLIC "${GPI2_LIB}")

add_executable(local_rank.bin "local_rank.c")
target_link_libraries(local_rank.bin PUBLIC "${GPI2_LIB}")

add_executable(initialized.bin "initialized.c")
target_link_libraries(initialized.bin PUBLIC "${GPI2_LIB}")

add_executable(hello_world.bin "hello_world.c")
target_link_libraries(hello_world.bin PUBLIC "${GPI2_LIB}")
