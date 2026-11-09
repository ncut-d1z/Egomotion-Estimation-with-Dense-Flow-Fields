cmake_minimum_required(VERSION 3.28)

# 当前仅构建已经获准实现的 l=0,1,2 低阶球谐叶子模块。
# 其余 domain/contracts/MAT/CLI/KvD/Adaptive/Runner 等均未获代码实施授权。
project(EgomotionEstimationWithDenseFlowFields VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "Only Debian/Ubuntu GNU/Linux is supported.")
endif()

# GCC 16.2.0 支持 C++23，但本仓库已确定的实现合同仍是 C++17。
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)

add_library(egomotion_harmonics STATIC
    src/harmonics/low_order_basis.cpp
    src/harmonics/low_order_rotation.cpp
    src/harmonics/low_order_choi.cpp
    src/harmonics/low_order_source_projection.cpp
)
add_library(egomotion::harmonics ALIAS egomotion_harmonics)

target_include_directories(egomotion_harmonics
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/harmonics"
)
target_link_libraries(egomotion_harmonics PUBLIC Eigen3::Eigen)
target_compile_features(egomotion_harmonics PUBLIC cxx_std_17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(egomotion_harmonics PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    # 当前只注册已存在的球谐测试源码；无其他业务模块或 MAT 写回读。
    add_executable(test_low_order_harmonics
        tests/unit/test_low_order_harmonics.cpp
    )
    target_link_libraries(test_low_order_harmonics PRIVATE egomotion_harmonics)
    target_compile_features(test_low_order_harmonics PRIVATE cxx_std_17)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(test_low_order_harmonics PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME low_order_harmonics COMMAND test_low_order_harmonics)
    set_tests_properties(low_order_harmonics PROPERTIES
        LABELS "harmonics;low-order"
        TIMEOUT 120
    )

    # 与既有大规模回归相互独立：直接对照归档 AN-G06/07/08 固定数值及源 CS 相位。
    add_executable(test_low_order_archived_golden
        tests/unit/test_low_order_archived_golden.cpp
    )
    target_link_libraries(test_low_order_archived_golden PRIVATE egomotion_harmonics)
    target_compile_features(test_low_order_archived_golden PRIVATE cxx_std_17)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(test_low_order_archived_golden PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME low_order_harmonics_archived_golden COMMAND test_low_order_archived_golden)
    set_tests_properties(low_order_harmonics_archived_golden PROPERTIES
        LABELS "harmonics;low-order;archived-golden"
        TIMEOUT 120
    )

    # 原 MATLAB koeff_aus_near.m 的逐系数拟合，按独立解析七方向参考验收。
    # 仅SH叶子；不引入 Adaptive/Runner/采样器/非球谐的业务测试。
    add_executable(test_low_order_source_projection
        tests/unit/test_low_order_source_projection.cpp
    )
    target_link_libraries(test_low_order_source_projection PRIVATE egomotion_harmonics)
    target_compile_features(test_low_order_source_projection PRIVATE cxx_std_17)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(test_low_order_source_projection PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME low_order_harmonics_source_projection COMMAND test_low_order_source_projection)
    set_tests_properties(low_order_harmonics_source_projection PROPERTIES
        LABELS "harmonics;low-order;source-projection"
        TIMEOUT 120
    )
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Eigen: ${Eigen3_VERSION}")
message(STATUS "Scope: low-order spherical harmonics only (BUILD_TESTING=${BUILD_TESTING})")
