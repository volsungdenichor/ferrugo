include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(ferrugo_ut optional.test.cpp iterable.test.cpp)

target_link_libraries(ferrugo_ut
  PRIVATE
  GTest::GTest)

include_directories(
  "${PROJECT_SOURCE_DIR}/include")

add_test(gtests ferrugo_ut)
