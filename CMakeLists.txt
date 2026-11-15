cmake_minimum_required(VERSION 3.28)

# R0/R1: typed CLI bootstrap, header-only public contracts, existing SH leaf.
# Remaining libraries are INTERFACE dependency placeholders until R2-R5.
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


add_library(egomotion_domain INTERFACE)
add_library(egomotion::domain ALIAS egomotion_domain)
target_include_directories(egomotion_domain INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_link_libraries(egomotion_domain INTERFACE Eigen3::Eigen)
target_compile_features(egomotion_domain INTERFACE cxx_std_17)

add_library(egomotion_contracts INTERFACE)
add_library(egomotion::contracts ALIAS egomotion_contracts)
target_link_libraries(egomotion_contracts INTERFACE egomotion_domain)
target_compile_features(egomotion_contracts INTERFACE cxx_std_17)

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
target_link_libraries(egomotion_harmonics PUBLIC egomotion_domain egomotion_contracts)
target_compile_features(egomotion_harmonics PUBLIC cxx_std_17)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(egomotion_harmonics PRIVATE -Wall -Wextra -Wpedantic)
endif()


# Planned interface-only targets express graph edges, not compiled backends.
add_library(egomotion_sampling INTERFACE)
add_library(egomotion::sampling ALIAS egomotion_sampling)
target_link_libraries(egomotion_sampling INTERFACE egomotion_domain egomotion_contracts)

add_library(egomotion_simulation INTERFACE)
add_library(egomotion::simulation ALIAS egomotion_simulation)
target_link_libraries(egomotion_simulation INTERFACE
    egomotion_domain egomotion_sampling egomotion_contracts)

add_library(egomotion_estimators INTERFACE)
add_library(egomotion::estimators ALIAS egomotion_estimators)
target_link_libraries(egomotion_estimators INTERFACE
    egomotion_domain egomotion_harmonics)

# R2 uses the existing libmatio via pkg-config (no vendored parser).
# Public interfaces remain MAT-library-free; all matio.h calls are PRIVATE.
find_package(PkgConfig REQUIRED)
pkg_check_modules(MATIO REQUIRED IMPORTED_TARGET matio)
find_package(ZLIB REQUIRED)

add_library(egomotion_matio STATIC
    src/io/mat_result_reader.cpp
    src/io/mat_data_dictionary.cpp
)
add_library(egomotion::matio ALIAS egomotion_matio)
target_link_libraries(egomotion_matio
    PUBLIC egomotion_domain egomotion_contracts
    PRIVATE PkgConfig::MATIO ZLIB::ZLIB
)
target_compile_features(egomotion_matio PUBLIC cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(egomotion_matio PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Historical command remains a separate thin executable; source location
# and report CLI arguments are unchanged after the refactor.
add_executable(mat_data_dict_probe src/mat_data_dict_probe.cpp)
target_link_libraries(mat_data_dict_probe PRIVATE egomotion_matio)
target_compile_features(mat_data_dict_probe PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mat_data_dict_probe PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(egomotion_experiments INTERFACE)
add_library(egomotion::experiments ALIAS egomotion_experiments)
target_link_libraries(egomotion_experiments INTERFACE
    egomotion_domain egomotion_contracts
    egomotion_sampling egomotion_simulation egomotion_estimators)

add_library(egomotion_analysis INTERFACE)
add_library(egomotion::analysis ALIAS egomotion_analysis)
target_link_libraries(egomotion_analysis INTERFACE
    egomotion_domain egomotion_contracts)

# Plot backend remains optional and is not implemented in R0/R1.
add_library(egomotion_plotting INTERFACE)
add_library(egomotion::plotting ALIAS egomotion_plotting)
target_link_libraries(egomotion_plotting INTERFACE egomotion_analysis)

add_library(egomotion_cli STATIC
    src/cli/command.cpp
    src/cli/application.cpp
)
add_library(egomotion::cli ALIAS egomotion_cli)
target_link_libraries(egomotion_cli PUBLIC
    egomotion_experiments egomotion_analysis
    egomotion_matio egomotion_contracts)
target_compile_features(egomotion_cli PUBLIC cxx_std_17)

add_executable(egomotion src/cli/main.cpp)
target_link_libraries(egomotion PRIVATE egomotion_cli)
target_compile_features(egomotion PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(egomotion_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(egomotion PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    # Preserve all three prior independent low-order SH tests.
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


    add_executable(test_r01_contracts tests/contracts/test_r01_contracts.cpp)
    target_link_libraries(test_r01_contracts PRIVATE egomotion_contracts)
    target_compile_features(test_r01_contracts PRIVATE cxx_std_17)
    add_test(NAME r01_contracts COMMAND test_r01_contracts)
    set_tests_properties(r01_contracts PROPERTIES LABELS "r01;contracts" TIMEOUT 30)

    add_executable(test_r01_cli tests/integration/test_r01_cli.cpp)
    target_link_libraries(test_r01_cli PRIVATE egomotion_cli)
    target_compile_features(test_r01_cli PRIVATE cxx_std_17)
    add_test(NAME r01_cli_contract COMMAND test_r01_cli)
    set_tests_properties(r01_cli_contract PROPERTIES LABELS "r01;cli" TIMEOUT 30)
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Eigen: ${Eigen3_VERSION}")
message(STATUS "Scope: R0/R1 CLI + R2 matio Reader/dictionary + low-order SH")
message(STATUS "R3-R5 simulation, estimators, experiments and analysis: INTERFACE placeholders")
message(STATUS "BUILD_TESTING=${BUILD_TESTING} (not a test result)")
