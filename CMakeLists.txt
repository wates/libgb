project(libgb)

if(MSVC)
set(CompilerFlags
CMAKE_CXX_FLAGS
CMAKE_CXX_FLAGS_DEBUG
CMAKE_CXX_FLAGS_RELEASE
CMAKE_C_FLAGS
CMAKE_C_FLAGS_DEBUG
CMAKE_C_FLAGS_RELEASE
)
if (MSVC_VERSION GREATER_EQUAL "1900")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
  if (_cpp_latest_flag_supported)
    add_compile_options("/std:c++latest")
  endif()
endif()
  add_definitions(-D_UNICODE -DUNICODE)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
foreach(CompilerFlag ${CompilerFlags})
string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()
endif(MSVC)
if(ANDROID)
endif(ANDROID)

include_directories("src")

add_library(gb
  src/libgb.cc
  src/libgb.h
)

file(GLOB bincc_data "app/bincc/*.cc")
add_executable(app
  app/main.cc
  app/viewport.h
  app/viewport_windows.cc
  app/msgq.cc
  app/msgq.h
  ${bincc_data}
)
target_link_libraries(app gb)

add_executable(test
  test/test.cc
  ${bincc_data}
)
target_link_libraries(test gb)
