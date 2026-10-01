
set(DAEMON_SRC
  "main.cpp"
  "luapi.cpp"
)

list(TRANSFORM DAEMON_SRC PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/src/")


add_executable(rzdaemon)

target_compile_features(rzdaemon PRIVATE cxx_std_23)

target_sources(rzdaemon PRIVATE ${DAEMON_SRC})

target_include_directories(rzdaemon PRIVATE "${CMAKE_SOURCE_DIR}/include/")

target_link_libraries(rzdaemon PRIVATE rzutils spdlog::spdlog lua)
