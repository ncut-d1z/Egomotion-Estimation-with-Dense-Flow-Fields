cmake_minimum_required(VERSION 3.28)

# R6 source graph: all R0-R5 production implementations are now explicitly linked.
# No configure/build/CTest is authorized by writing this CMake file.
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
find_package(Sophus CONFIG REQUIRED)
if(NOT TARGET Sophus::Sophus)
    message(FATAL_ERROR "SophusConfig.cmake must define Sophus::Sophus")
endif()


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
    src/harmonics/adapter.cpp
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


# Source boundaries match the domain-oriented dependency DAG.
add_library(egomotion_sampling STATIC
    src/sampling/directions.cpp
    src/sampling/random.cpp
)
add_library(egomotion::sampling ALIAS egomotion_sampling)
target_link_libraries(egomotion_sampling PUBLIC
    egomotion_domain egomotion_contracts)

add_library(egomotion_simulation STATIC
    src/simulation/flow.cpp
    src/trajectory/trajectory.cpp
    src/trajectory/kinematics.cpp
    src/trajectory/sphere.cpp
)
add_library(egomotion::simulation ALIAS egomotion_simulation)
target_link_libraries(egomotion_simulation PUBLIC
    egomotion_domain egomotion_sampling egomotion_contracts
    PRIVATE Sophus::Sophus)

add_library(egomotion_estimators STATIC
    src/estimators/franz_kvd.cpp
    src/estimators/adaptive_step.cpp
    src/estimators/adaptive_session.cpp
)
add_library(egomotion::estimators ALIAS egomotion_estimators)
target_link_libraries(egomotion_estimators PUBLIC
    egomotion_domain egomotion_harmonics egomotion_simulation)

# R2 uses the existing libmatio via pkg-config (no vendored parser).
# Public interfaces remain MAT-library-free; all matio.h calls are PRIVATE.
find_package(PkgConfig REQUIRED)
pkg_check_modules(MATIO REQUIRED IMPORTED_TARGET matio)
find_package(ZLIB REQUIRED)

add_library(egomotion_matio STATIC
    src/io/mat_result_reader.cpp
    src/io/mat_data_dictionary.cpp
    src/io/sha256.cpp
    src/io/run_directory.cpp
    src/io/run_mat_writer.cpp
    src/io/run_text_publisher.cpp
    src/io/manifest_publisher.cpp
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

add_library(egomotion_experiments STATIC
    src/experiments/bias_runner.cpp
    src/experiments/adaptive_runner.cpp
    src/experiments/output_contract.cpp
    src/experiments/output_values.cpp
    src/experiments/run_context.cpp
    src/experiments/manifest_json.cpp
    src/experiments/production_runner.cpp
)
add_library(egomotion::experiments ALIAS egomotion_experiments)
target_link_libraries(egomotion_experiments PUBLIC
    egomotion_domain egomotion_contracts egomotion_sampling
    egomotion_simulation egomotion_estimators)

add_library(egomotion_analysis STATIC
    src/analysis/reader_helpers.cpp
    src/analysis/result_analyzer.cpp
    src/analysis/bias.cpp
    src/analysis/adaptive.cpp
    src/analysis/dataset_json.cpp
)
add_library(egomotion::analysis ALIAS egomotion_analysis)
target_link_libraries(egomotion_analysis PUBLIC
    egomotion_domain egomotion_contracts)

# Plotting consumes only immutable numerical curves, never a MAT reader.
add_library(egomotion_plotting STATIC src/analysis/svg_plot.cpp)
add_library(egomotion::plotting ALIAS egomotion_plotting)
target_link_libraries(egomotion_plotting PUBLIC egomotion_analysis egomotion_contracts)

add_library(egomotion_cli STATIC
    src/cli/command.cpp
    src/cli/application.cpp
)
add_library(egomotion::cli ALIAS egomotion_cli)
target_link_libraries(egomotion_cli PUBLIC
    egomotion_experiments egomotion_analysis egomotion_plotting
    egomotion_matio egomotion_sampling egomotion_contracts)
target_compile_features(egomotion_cli PUBLIC cxx_std_17)

# Fix executable provenance at CONFIGURE time, not by reading a potentially
# changed runtime HEAD. A dirty/unversioned source tree cannot publish a
# Complete Manifest: it receives UNVERIFIED_BUILD_SOURCE and is rejected
# by makeCompleteManifestJson. Reconfigure after every source commit.
find_package(Git QUIET)
set(EGOMOTION_BUILD_SOURCE_SHA "UNVERIFIED_BUILD_SOURCE")
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" rev-parse --verify HEAD
        OUTPUT_VARIABLE EGOMOTION_GIT_HEAD OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET RESULT_VARIABLE EGOMOTION_GIT_HEAD_RESULT
    )
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" -C "${CMAKE_CURRENT_SOURCE_DIR}" status --porcelain --untracked-files=no
        OUTPUT_VARIABLE EGOMOTION_GIT_DIRTY OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET RESULT_VARIABLE EGOMOTION_GIT_STATUS_RESULT
    )
    string(LENGTH "${EGOMOTION_GIT_HEAD}" EGOMOTION_SHA_LENGTH)
    if(EGOMOTION_GIT_HEAD_RESULT EQUAL 0 AND
       EGOMOTION_GIT_STATUS_RESULT EQUAL 0 AND
       EGOMOTION_SHA_LENGTH EQUAL 40 AND
       EGOMOTION_GIT_HEAD MATCHES "^[0-9a-f]+$" AND
       EGOMOTION_GIT_DIRTY STREQUAL "")
        set(EGOMOTION_BUILD_SOURCE_SHA "${EGOMOTION_GIT_HEAD}")
    endif()
endif()
target_compile_definitions(egomotion_cli PRIVATE
    "EGOMOTION_BUILD_SOURCE_SHA=\"${EGOMOTION_BUILD_SOURCE_SHA}\""
)
message(STATUS "Executable source commit: ${EGOMOTION_BUILD_SOURCE_SHA}")

add_executable(egomotion src/cli/main.cpp)
target_link_libraries(egomotion PRIVATE egomotion_cli)
target_compile_features(egomotion PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target IN ITEMS
            egomotion_sampling egomotion_simulation egomotion_estimators
            egomotion_matio egomotion_experiments egomotion_analysis
            egomotion_plotting egomotion_cli egomotion)
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endforeach()
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

    # R2 exclusively reads two repository fixtures. It never writes origin or reports.
    add_executable(test_r02_reader tests/integration/test_r02_reader.cpp)
    target_link_libraries(test_r02_reader PRIVATE egomotion_cli egomotion_matio)
    target_compile_features(test_r02_reader PRIVATE cxx_std_17)
    add_test(NAME r02_mat_reader COMMAND test_r02_reader
        "${CMAKE_CURRENT_SOURCE_DIR}/origin/egomotion_estimation_dense_flow_fields_matlab_scripts/bias_test/produced_data/occluded_regions/1/xwerte.mat"
        "${CMAKE_CURRENT_SOURCE_DIR}/origin/egomotion_estimation_dense_flow_fields_matlab_scripts/bias_test/produced_data/occluded_regions/1/ywerte3.mat"
    )
    set_tests_properties(r02_mat_reader PROPERTIES LABELS "r02;mat;readonly" TIMEOUT 60)

    # R5 domain-only contract tests must not write to origin or output/.
    add_executable(test_r05_manifest tests/contracts/test_r05_manifest.cpp)
    target_link_libraries(test_r05_manifest PRIVATE egomotion_experiments)
    target_compile_features(test_r05_manifest PRIVATE cxx_std_17)
    add_test(NAME r05_manifest_gate COMMAND test_r05_manifest)
    set_tests_properties(r05_manifest_gate PROPERTIES LABELS "r05;contracts;manifest" TIMEOUT 30)

    # All synthetic writes happen beneath the CMake BINARY directory only.
    # Never clean user files or reuse an old RunId; each run uses OS randomness.
    add_executable(test_r05_io tests/integration/test_r05_io.cpp)
    target_link_libraries(test_r05_io PRIVATE egomotion_matio)
    target_compile_features(test_r05_io PRIVATE cxx_std_17)
    add_test(NAME r05_anchored_io COMMAND test_r05_io
        "${CMAKE_CURRENT_BINARY_DIR}/r05_synthetic_io")
    set_tests_properties(r05_anchored_io PROPERTIES LABELS "r05;io;write-sandbox" TIMEOUT 60)

    add_executable(test_r05_analysis tests/integration/test_r05_analysis.cpp)
    target_link_libraries(test_r05_analysis PRIVATE egomotion_analysis)
    target_compile_features(test_r05_analysis PRIVATE cxx_std_17)
    add_test(NAME r05_analysis_source_contract COMMAND test_r05_analysis
        "${CMAKE_CURRENT_BINARY_DIR}/r05_synthetic_analysis")
    set_tests_properties(r05_analysis_source_contract PROPERTIES
        LABELS "r05;analysis;synthetic-only" TIMEOUT 60)
    # R6 source-only regressions. Register now, execute only after H-11.
    add_executable(test_r06_sampling_flow tests/integration/test_r06_sampling_flow.cpp)
    target_link_libraries(test_r06_sampling_flow PRIVATE
        egomotion_sampling egomotion_simulation)
    target_compile_features(test_r06_sampling_flow PRIVATE cxx_std_17)
    add_test(NAME r06_sampling_flow_contract COMMAND test_r06_sampling_flow)
    set_tests_properties(r06_sampling_flow_contract PROPERTIES
        LABELS "r06;sampling;flow;synthetic-only" TIMEOUT 60)

    add_executable(test_r06_session_contract tests/contracts/test_r06_session_contract.cpp)
    target_link_libraries(test_r06_session_contract PRIVATE
        egomotion_estimators egomotion_experiments)
    target_compile_features(test_r06_session_contract PRIVATE cxx_std_17)
    add_test(NAME r06_adaptive_session_transaction COMMAND test_r06_session_contract)
    set_tests_properties(r06_adaptive_session_transaction PROPERTIES
        LABELS "r06;adaptive;session;failure-atomic" TIMEOUT 60)

    add_executable(test_r06_analysis_segments tests/integration/test_r06_analysis_segments.cpp)
    target_link_libraries(test_r06_analysis_segments PRIVATE egomotion_analysis)
    target_compile_features(test_r06_analysis_segments PRIVATE cxx_std_17)
    add_test(NAME r06_analysis_segments_smoothing COMMAND test_r06_analysis_segments)
    set_tests_properties(r06_analysis_segments_smoothing PROPERTIES
        LABELS "r06;analysis;source-index;synthetic-only" TIMEOUT 60)

    # Independent R6 reference fixtures: Wolfram source math is NOT an
    # approved tolerance, MATLAB result, or previously executed CTest.
    add_executable(test_r06_kvd_l3 tests/integration/test_r06_kvd_l3.cpp)
    target_link_libraries(test_r06_kvd_l3 PRIVATE egomotion_estimators)
    target_compile_features(test_r06_kvd_l3 PRIVATE cxx_std_17)
    add_test(NAME r06_kvd_independent_l3 COMMAND test_r06_kvd_l3)
    set_tests_properties(r06_kvd_independent_l3 PROPERTIES
        LABELS "r06;kvd;franz;independent-l3-candidate" TIMEOUT 90)

    add_executable(test_r06_adaptive_l3 tests/integration/test_r06_adaptive_l3.cpp)
    target_link_libraries(test_r06_adaptive_l3 PRIVATE egomotion_estimators)
    target_compile_features(test_r06_adaptive_l3 PRIVATE cxx_std_17)
    add_test(NAME r06_adaptive_step_independent_l3 COMMAND test_r06_adaptive_l3)
    set_tests_properties(r06_adaptive_step_independent_l3 PROPERTIES
        LABELS "r06;adaptive;6x6;independent-l3-candidate" TIMEOUT 90)

    # H-11 remains a separate gate for even READ-ONLY native execution.
    # This target never modifies origin/** and does NOT compare numeric L2
    # values with an independently approved reference.
    add_executable(test_r06_archive_readonly
        tests/integration/test_r06_archive_readonly.cpp)
    target_link_libraries(test_r06_archive_readonly PRIVATE egomotion_matio)
    target_compile_features(test_r06_archive_readonly PRIVATE cxx_std_17)
    add_test(NAME r06_historical_mat_readonly_schema
        COMMAND test_r06_archive_readonly
        "${CMAKE_CURRENT_SOURCE_DIR}/origin/egomotion_estimation_dense_flow_fields_matlab_scripts")
    set_tests_properties(r06_historical_mat_readonly_schema PROPERTIES
        LABELS "r06;mat;archive;readonly;schema-only" TIMEOUT 180)

    # Physical SH argument action != automatic source-SHRotate equivalence.
    add_executable(test_r06_sh_adapter tests/integration/test_r06_sh_adapter.cpp)
    target_link_libraries(test_r06_sh_adapter PRIVATE
        egomotion_harmonics egomotion_simulation)
    target_compile_features(test_r06_sh_adapter PRIVATE cxx_std_17)
    add_test(NAME r06_sh_argument_action COMMAND test_r06_sh_adapter)
    set_tests_properties(r06_sh_argument_action PROPERTIES
        LABELS "r06;sh;so3;argument-action" TIMEOUT 60)

    # R6 complete ordered source sampling: 714 rows / 2142 coordinates from
    # the pre-existing independent geometry reference (unapproved L3).
    add_executable(test_r06_direction_rows_l3
        tests/integration/test_r06_direction_rows_l3.cpp)
    target_link_libraries(test_r06_direction_rows_l3 PRIVATE egomotion_sampling)
    target_compile_features(test_r06_direction_rows_l3 PRIVATE cxx_std_17)
    add_test(NAME r06_directions_all_ordered_l3
        COMMAND test_r06_direction_rows_l3
        "${CMAKE_CURRENT_SOURCE_DIR}/verification/geometry/source_direction_levels_0_2.v1.json")
    set_tests_properties(r06_directions_all_ordered_l3 PROPERTIES
        LABELS "r06;geometry;sampling;ordered-l3-candidate" TIMEOUT 60)

    # R6 hand-derived source trajectory, SO(3) kinematics and sphere nearness.
    add_executable(test_r06_trajectory_sphere
        tests/integration/test_r06_trajectory_sphere.cpp)
    target_link_libraries(test_r06_trajectory_sphere PRIVATE egomotion_simulation)
    target_compile_features(test_r06_trajectory_sphere PRIVATE cxx_std_17)
    add_test(NAME r06_trajectory_sphere_geometry COMMAND test_r06_trajectory_sphere)
    set_tests_properties(r06_trajectory_sphere_geometry PROPERTIES
        LABELS "r06;trajectory;sphere;analytic" TIMEOUT 60)

    # Session success is observable with a deterministic injected StepSolver.
    # This tests period-1/5/11/21, discarded dumi and two failure injection
    # sites without performing scientific numerical experiments.
    add_executable(test_r06_adaptive_periods
        tests/contracts/test_r06_adaptive_periods.cpp)
    target_link_libraries(test_r06_adaptive_periods PRIVATE egomotion_estimators)
    target_compile_features(test_r06_adaptive_periods PRIVATE cxx_std_17)
    add_test(NAME r06_adaptive_periods_atomic_commit COMMAND test_r06_adaptive_periods)
    set_tests_properties(r06_adaptive_periods_atomic_commit PROPERTIES
        LABELS "r06;adaptive;periodic;success-transaction;synthetic" TIMEOUT 60)

    # R6 Linux anchored I/O safety negatives, sandboxed under build tree.
    # No cleanup, original MAT access, or trusted user output involved.
    add_executable(test_r06_io_safety
        tests/integration/test_r06_io_safety.cpp)
    target_link_libraries(test_r06_io_safety PRIVATE egomotion_matio)
    target_compile_features(test_r06_io_safety PRIVATE cxx_std_17)
    add_test(NAME r06_io_symlink_and_runid_safety
        COMMAND test_r06_io_safety
        "${CMAKE_CURRENT_BINARY_DIR}/r06_synthetic_io_safety")
    set_tests_properties(r06_io_symlink_and_runid_safety PROPERTIES
        LABELS "r06;io;security;synthetic-only" TIMEOUT 60)

endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Eigen: ${Eigen3_VERSION}")
message(STATUS "Scope: R0-R5 compiled source targets; R6 config/build/tests require separate H-11 authorization")
message(STATUS "BUILD_TESTING=${BUILD_TESTING} (not a test result)")
