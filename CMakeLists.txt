project(libmeta
  LANGUAGES CXX
)

add_library(libmeta INTERFACE)
target_include_directories(libmeta INTERFACE include)
target_compile_features(libmeta INTERFACE cxx_std_20)

add_executable(main
  "test/main.cpp"
)
target_link_libraries(main PRIVATE libmeta)