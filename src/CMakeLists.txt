cmake_minimum_required(VERSION 3.5)

add_subdirectory(devices)

add_library(
  ${PROJECT_NAME}
  SHARED
  "GPI2.c"
  "GPI2_ATOMIC.c"
  "GPI2_CM.c"
  "GPI2_Coll.c"
  "GPI2_CONFIG.c"
  "GPI2_Env.c"
  "GPI2_GRP.c"
  "GPI2_IO.c"
  "GPI2_Logger.c"
  "GPI2_Mem.c"
  "GPI2_PASSIVE.c"
  "GPI2_SEG.c"
  "GPI2_SN.c"
  "GPI2_Stats.c"
  "GPI2_Threads.c"
  "GPI2_Utility.c"
)

target_include_directories(${PROJECT_NAME} PRIVATE "include")
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE
    "$<$<BOOL:${WITH_IB}>:ib-device>" "$<$<BOOL:${WITH_BXI}>:bxi-device>"
    "$<$<AND:$<NOT:$<BOOL:${WITH_IB}>>,$<NOT:$<BOOL:${WITH_BXI}>>>:tcp-device>"
)
target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE
    "$<$<BOOL:${WITH_IB}>:GPI2_DEVICE_IB>"
    "$<$<BOOL:${WITH_BXI}>:GPI2_DEVICE_BXI>"
    "$<$<AND:$<NOT:$<BOOL:${WITH_IB}>>,$<NOT:$<BOOL:${WITH_BXI}>>>:GPI2_DEVICE_TCP>"
)
target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
target_compile_options(${PROJECT_NAME} PRIVATE "-Wno-deprecated-declarations")
