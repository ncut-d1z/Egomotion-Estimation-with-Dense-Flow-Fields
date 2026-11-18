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
option(EGOMOTION_ENABLE_SVG "构建可选 SVG 导出器；关闭后保留全部数值入口" ON)

find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(Sophus CONFIG REQUIRED)
if(NOT TARGET Sophus::Sophus)
    message(FATAL_ERROR "SophusConfig.cmake must define Sophus::Sophus")
endif()


add_library(egomotion_domain STATIC src/domain/motion_estimate.cpp)
add_library(egomotion::domain ALIAS egomotion_domain)
target_include_directories(egomotion_domain PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_link_libraries(egomotion_domain PUBLIC Eigen3::Eigen)
target_compile_features(egomotion_domain PUBLIC cxx_std_17)
target_compile_options(egomotion_domain PRIVATE -Wall -Wextra -Wpedantic)

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
    src/harmonics/real_spherical_harmonics.cpp
    src/harmonics/harmonic_fitter.cpp
    src/harmonics/harmonic_rotator.cpp
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
    src/estimators/coefficient_update.cpp
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
    src/io/mat_read_snapshot.cpp
    src/io/mat_container_validation.cpp
    src/io/mat_payload_validation.cpp
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
add_library(egomotion_dictionary_cli STATIC src/cli/mat_dictionary_command.cpp)
target_link_libraries(egomotion_dictionary_cli PUBLIC egomotion_matio)
target_compile_features(egomotion_dictionary_cli PUBLIC cxx_std_17)
add_executable(mat_data_dict_probe src/mat_data_dict_probe.cpp)
target_link_libraries(mat_data_dict_probe PRIVATE egomotion_dictionary_cli)
target_compile_features(mat_data_dict_probe PRIVATE cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(egomotion_dictionary_cli PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(mat_data_dict_probe PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_library(egomotion_experiments STATIC
    src/experiments/bias_runner.cpp
    src/experiments/adaptive_runner.cpp
    src/experiments/adaptive_entries.cpp
    src/experiments/adaptive_aggregation.cpp
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
if(EGOMOTION_ENABLE_SVG)
    add_library(egomotion_plotting STATIC src/analysis/svg_plot.cpp)
    add_library(egomotion::plotting ALIAS egomotion_plotting)
    target_link_libraries(egomotion_plotting PUBLIC egomotion_analysis egomotion_contracts)
endif()

add_library(egomotion_illustrations STATIC src/illustrations/geometry.cpp src/illustrations/data_json.cpp)
target_link_libraries(egomotion_illustrations PUBLIC egomotion_domain egomotion_contracts
    PRIVATE egomotion_simulation)
if(EGOMOTION_ENABLE_SVG)
    add_library(egomotion_illustration_svg STATIC src/illustrations/svg.cpp)
    target_link_libraries(egomotion_illustration_svg PUBLIC egomotion_illustrations)
endif()

add_library(egomotion_cli STATIC
    src/cli/command.cpp
    src/cli/application.cpp
    src/cli/execution_record.cpp
)
add_library(egomotion::cli ALIAS egomotion_cli)
target_link_libraries(egomotion_cli PUBLIC
    egomotion_experiments egomotion_analysis
    egomotion_matio egomotion_sampling egomotion_contracts egomotion_illustrations)
target_compile_features(egomotion_cli PUBLIC cxx_std_17)
if(EGOMOTION_ENABLE_SVG)
    set(EGOMOTION_SVG_LITERAL true)
else()
    set(EGOMOTION_SVG_LITERAL false)
endif()
configure_file(src/cli/build_identity.hpp.in generated/build_identity.hpp @ONLY)
target_include_directories(egomotion_cli PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_definitions(egomotion_cli PRIVATE
    "EGOMOTION_BUILD_CONFIGURATION=\"$<CONFIG>\"")

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
target_compile_definitions(egomotion PRIVATE "EGOMOTION_ENABLE_SVG=$<BOOL:${EGOMOTION_ENABLE_SVG}>")
if(EGOMOTION_ENABLE_SVG)
    target_link_libraries(egomotion PRIVATE egomotion_plotting egomotion_illustration_svg)
endif()
target_compile_features(egomotion PRIVATE cxx_std_17)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    foreach(target IN ITEMS
            egomotion_sampling egomotion_simulation egomotion_estimators
            egomotion_matio egomotion_experiments egomotion_analysis
            egomotion_plotting egomotion_illustrations egomotion_illustration_svg egomotion_cli egomotion)
        if(TARGET ${target})
            target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
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


    add_executable(test_r06_motion_value tests/contracts/test_r06_motion_value.cpp)
    target_link_libraries(test_r06_motion_value PRIVATE egomotion_contracts)
    target_compile_options(test_r06_motion_value PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_motion_value_invariants COMMAND test_r06_motion_value)
    set_tests_properties(r06_motion_value_invariants PROPERTIES
        LABELS "r06;contracts;domain;motion;value-lifetime;negative" TIMEOUT 30)

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

    add_executable(test_r06_runid_diagnostics tests/integration/test_r06_runid_diagnostics.cpp)
    target_link_libraries(test_r06_runid_diagnostics PRIVATE egomotion_cli)
    add_test(NAME r06_runid_failure_context COMMAND test_r06_runid_diagnostics
        "${CMAKE_CURRENT_BINARY_DIR}/r06_runid_diagnostics")
    set_tests_properties(r06_runid_failure_context PROPERTIES
        LABELS "r06;cli;diagnostics;write-sandbox;synthetic-only" TIMEOUT 30)

    add_executable(test_r06_cli_streams tests/integration/test_r06_cli_streams.cpp)
    target_link_libraries(test_r06_cli_streams PRIVATE egomotion_cli)
    target_compile_options(test_r06_cli_streams PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_cli_output_streams COMMAND test_r06_cli_streams
        "${CMAKE_CURRENT_BINARY_DIR}/r06_cli_streams")
    set_tests_properties(r06_cli_output_streams PROPERTIES
        LABELS "r06;cli;diagnostics;write-sandbox;stream-faults;bounded-bias" TIMEOUT 90)
    add_test(NAME r06_cli_output_real_process COMMAND "${CMAKE_COMMAND}"
        "-DCLI=$<TARGET_FILE:egomotion>" "-DSOURCE_SHA=${EGOMOTION_BUILD_SOURCE_SHA}"
        "-DSANDBOX=${CMAKE_CURRENT_BINARY_DIR}/r06_cli_streams_process"
        "-DSVG_ENABLED=${EGOMOTION_ENABLE_SVG}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_r06_cli_streams.cmake")
    set_tests_properties(r06_cli_output_real_process PROPERTIES
        LABELS "r06;cli;diagnostics;write-sandbox;real-process;dev-full;bounded-bias" TIMEOUT 90)

    add_executable(test_r06_execution_records tests/integration/test_r06_execution_records.cpp)
    target_link_libraries(test_r06_execution_records PRIVATE egomotion_cli)
    target_compile_options(test_r06_execution_records PRIVATE -Wall -Wextra -Wpedantic)
    target_link_options(test_r06_execution_records PRIVATE -Wl,--wrap=linkat -Wl,--wrap=fsync)
    add_test(NAME r06_execution_record_contract COMMAND test_r06_execution_records
        "${CMAKE_CURRENT_BINARY_DIR}/r06_execution_records")
    set_tests_properties(r06_execution_record_contract PROPERTIES
        LABELS "r06;cli;provenance;write-sandbox;publication-faults;bounded-bias" TIMEOUT 90)

    add_executable(test_r06_dictionary_streams tests/integration/test_r06_dictionary_streams.cpp)
    target_link_libraries(test_r06_dictionary_streams PRIVATE egomotion_dictionary_cli)
    target_compile_options(test_r06_dictionary_streams PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_dictionary_output_streams COMMAND test_r06_dictionary_streams
        "${CMAKE_CURRENT_SOURCE_DIR}/origin/egomotion_estimation_dense_flow_fields_matlab_scripts/adaptive_matched_filter_test/produced_data/initialization"
        "${CMAKE_CURRENT_BINARY_DIR}/r06_dictionary_streams")
    set_tests_properties(r06_dictionary_output_streams PROPERTIES
        LABELS "r06;mat;dictionary;write-sandbox;stream-faults" TIMEOUT 90)
    add_library(r06_dictionary_file_faults SHARED tests/integration/r06_dictionary_file_faults.cpp)
    target_link_libraries(r06_dictionary_file_faults PRIVATE ${CMAKE_DL_LIBS})
    target_compile_options(r06_dictionary_file_faults PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_dictionary_real_process COMMAND "${CMAKE_COMMAND}"
        "-DPROBE=$<TARGET_FILE:mat_data_dict_probe>"
        "-DFAULT_LIBRARY=$<TARGET_FILE:r06_dictionary_file_faults>"
        "-DSOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DSOURCE_SHA=${EGOMOTION_BUILD_SOURCE_SHA}"
        "-DSANDBOX=${CMAKE_CURRENT_BINARY_DIR}/r06_dictionary_process"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_r06_dictionary_process.cmake")
    set_tests_properties(r06_dictionary_real_process PROPERTIES
        LABELS "r06;mat;dictionary;real-process;write-sandbox;file-faults" TIMEOUT 180)

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

    add_executable(test_r06_analyzer_services tests/integration/test_r06_analyzer_services.cpp)
    target_link_libraries(test_r06_analyzer_services PRIVATE egomotion_analysis)
    target_compile_options(test_r06_analyzer_services PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_analyzer_service_contract COMMAND test_r06_analyzer_services
        "${CMAKE_CURRENT_BINARY_DIR}/r06_analyzer_services")
    set_tests_properties(r06_analyzer_service_contract PROPERTIES
        LABELS "r06;analysis;services;memory-reader;source-order" TIMEOUT 60)

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

    add_executable(test_r06_kvd_strategy tests/integration/test_r06_kvd_strategy.cpp)
    target_link_libraries(test_r06_kvd_strategy PRIVATE egomotion_estimators)
    target_compile_options(test_r06_kvd_strategy PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_kvd_strategy_composition COMMAND test_r06_kvd_strategy)
    set_tests_properties(r06_kvd_strategy_composition PROPERTIES
        LABELS "r06;kvd;strategy;source-order;failure-injection" TIMEOUT 90)

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

    add_executable(test_r06_harmonic_services tests/integration/test_r06_harmonic_services.cpp)
    target_link_libraries(test_r06_harmonic_services PRIVATE egomotion_harmonics)
    target_compile_options(test_r06_harmonic_services PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_harmonic_service_contract COMMAND test_r06_harmonic_services)
    set_tests_properties(r06_harmonic_service_contract PROPERTIES
        LABELS "r06;sh;services;analytic-reference" TIMEOUT 60)

    add_executable(test_r06_source_angles tests/unit/test_r06_source_angles.cpp)
    target_link_libraries(test_r06_source_angles PRIVATE egomotion_harmonics)
    target_compile_options(test_r06_source_angles PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_source_spherical_angles COMMAND test_r06_source_angles)
    set_tests_properties(r06_source_spherical_angles PROPERTIES
        LABELS "r06;sh;source-angles;analytic-reference" TIMEOUT 60)

    add_executable(test_r06_source_sh tests/integration/test_r06_source_sh.cpp)
    target_link_libraries(test_r06_source_sh PRIVATE egomotion_estimators)
    target_compile_features(test_r06_source_sh PRIVATE cxx_std_17)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(test_r06_source_sh PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME r06_sh_source_phase_reference COMMAND test_r06_source_sh adapter)
    add_test(NAME r06_sh_source_session_action COMMAND test_r06_source_sh session)
    set_tests_properties(r06_sh_source_phase_reference r06_sh_source_session_action PROPERTIES
        LABELS "r06;sh;source-action;independent-l3-candidate;synthetic-step" TIMEOUT 60)

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

    add_executable(test_r06_coefficient_policies tests/contracts/test_r06_coefficient_policies.cpp)
    target_link_libraries(test_r06_coefficient_policies PRIVATE egomotion_estimators)
    target_compile_options(test_r06_coefficient_policies PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_coefficient_policy_composition COMMAND test_r06_coefficient_policies)
    set_tests_properties(r06_coefficient_policy_composition PROPERTIES
        LABELS "r06;adaptive;policy;source-order;failure-injection" TIMEOUT 60)

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

    # R6 ABI/lifetime contract: temporary Results extract owned values;
    # nested Result/Status failures retain original error frame identities.
    add_executable(test_r06_result_lifetimes
        tests/contracts/test_r06_result_lifetimes.cpp)
    target_link_libraries(test_r06_result_lifetimes PRIVATE egomotion_contracts)
    target_compile_features(test_r06_result_lifetimes PRIVATE cxx_std_17)
    add_test(NAME r06_result_lifetimes_error_frames
        COMMAND test_r06_result_lifetimes)
    set_tests_properties(r06_result_lifetimes_error_frames PROPERTIES
        LABELS "r06;contracts;ownership;error-chain" TIMEOUT 30)

    # 故障包装仅链接进测试程序，不改变生产库/CLI 或动态 matio 内部调用。
    add_executable(test_r06_io_faults
        tests/integration/test_r06_io_faults.cpp
        tests/support/io_fault_injection.cpp)
    target_link_libraries(test_r06_io_faults PRIVATE egomotion_experiments egomotion_matio)
    target_compile_features(test_r06_io_faults PRIVATE cxx_std_17)
    target_compile_options(test_r06_io_faults PRIVATE -Wall -Wextra -Wpedantic)
    foreach(symbol IN ITEMS openat write fsync linkat unlinkat fstatat)
        target_link_options(test_r06_io_faults PRIVATE "LINKER:--wrap=${symbol}")
    endforeach()
    add_test(NAME r06_linux_writer_faults COMMAND test_r06_io_faults
        "${CMAKE_CURRENT_BINARY_DIR}/r06_writer_faults" writers)
    add_test(NAME r06_real_manifest_faults COMMAND test_r06_io_faults
        "${CMAKE_CURRENT_BINARY_DIR}/r06_manifest_faults" manifest)
    set_tests_properties(r06_linux_writer_faults r06_real_manifest_faults PROPERTIES
        LABELS "r06;io;fault-injection;synthetic-only;write-sandbox" TIMEOUT 60)

    add_executable(test_r06_mat_independent tests/integration/test_r06_mat_independent.cpp)
    target_link_libraries(test_r06_mat_independent PRIVATE egomotion_matio ZLIB::ZLIB)
    target_compile_features(test_r06_mat_independent PRIVATE cxx_std_17)
    target_compile_options(test_r06_mat_independent PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_mat_independent_storage_values COMMAND test_r06_mat_independent
        "${CMAKE_CURRENT_SOURCE_DIR}/origin/egomotion_estimation_dense_flow_fields_matlab_scripts"
        "${CMAKE_CURRENT_SOURCE_DIR}/verification/mat/archive_storage_identity.v1.tsv")
    set_tests_properties(r06_mat_independent_storage_values PROPERTIES
        LABELS "r06;mat;readonly;independent-l2;storage-tags" TIMEOUT 180)

    add_executable(test_r06_bias_archive tests/integration/test_r06_bias_archive.cpp)
    target_link_libraries(test_r06_bias_archive PRIVATE egomotion_analysis egomotion_matio ZLIB::ZLIB)
    target_compile_features(test_r06_bias_archive PRIVATE cxx_std_17)
    target_compile_options(test_r06_bias_archive PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_bias_archive_independent_curves COMMAND test_r06_bias_archive
        "${CMAKE_CURRENT_SOURCE_DIR}/origin/egomotion_estimation_dense_flow_fields_matlab_scripts"
        "${CMAKE_CURRENT_SOURCE_DIR}/verification/mat/archive_storage_identity.v1.tsv")
    set_tests_properties(r06_bias_archive_independent_curves PROPERTIES
        LABELS "r06;analysis;bias;readonly-archive;independent-l2-l3" TIMEOUT 180)

    add_executable(test_r06_reader_negative tests/integration/test_r06_reader_negative.cpp)
    # 仅重建合成版本负例时显式构建；普通构建/CTest不要求matio具备HDF5写能力。
    add_executable(r06_mat73_fixture_generator EXCLUDE_FROM_ALL tests/support/generate_mat73_fixture.cpp)
    target_link_libraries(r06_mat73_fixture_generator PRIVATE PkgConfig::MATIO)
    target_compile_features(r06_mat73_fixture_generator PRIVATE cxx_std_17)
    target_compile_options(r06_mat73_fixture_generator PRIVATE -Wall -Wextra -Wpedantic)
    add_executable(test_r06_mat73 tests/integration/test_r06_mat73.cpp)
    target_link_libraries(test_r06_mat73 PRIVATE egomotion_matio PkgConfig::MATIO)
    foreach(symbol Mat_Open Mat_VarRead pread)
        target_link_options(test_r06_mat73 PRIVATE "LINKER:--wrap=${symbol}")
    endforeach()
    target_compile_options(test_r06_mat73 PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_mat73_version_rejection COMMAND test_r06_mat73
        "${CMAKE_CURRENT_SOURCE_DIR}/verification/mat/synthetic_v73_double.mat"
        "${CMAKE_CURRENT_BINARY_DIR}/r06_mat73")
    set_tests_properties(r06_mat73_version_rejection PROPERTIES
        LABELS "r06;mat;version;negative;synthetic-only;write-sandbox" TIMEOUT 60)
    add_executable(test_r06_mat_snapshot tests/integration/test_r06_mat_snapshot.cpp)
    target_link_libraries(test_r06_mat_snapshot PRIVATE egomotion_matio PkgConfig::MATIO)
    foreach(symbol memfd_create pread write fcntl Mat_VarRead)
        target_link_options(test_r06_mat_snapshot PRIVATE "LINKER:--wrap=${symbol}")
    endforeach()
    target_compile_options(test_r06_mat_snapshot PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_mat_snapshot_identity COMMAND test_r06_mat_snapshot
        "${CMAKE_CURRENT_BINARY_DIR}/r06_mat_snapshot")
    set_tests_properties(r06_mat_snapshot_identity PROPERTIES
        LABELS "r06;mat;snapshot;write-sandbox;synthetic-only" TIMEOUT 60)
    target_link_libraries(test_r06_reader_negative PRIVATE egomotion_matio PkgConfig::MATIO)
    target_compile_features(test_r06_reader_negative PRIVATE cxx_std_17)
    target_compile_options(test_r06_reader_negative PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_mat_reader_boundaries COMMAND test_r06_reader_negative
        "${CMAKE_CURRENT_BINARY_DIR}/r06_reader_boundaries")
    set_tests_properties(r06_mat_reader_boundaries PROPERTIES
        LABELS "r06;mat;negative;synthetic-only;write-sandbox" TIMEOUT 60)

    add_executable(test_r06_mat_payload tests/integration/test_r06_mat_payload.cpp)
    target_link_libraries(test_r06_mat_payload PRIVATE egomotion_matio ZLIB::ZLIB PkgConfig::MATIO)
    target_link_options(test_r06_mat_payload PRIVATE "-Wl,--wrap=Mat_VarRead")
    target_compile_features(test_r06_mat_payload PRIVATE cxx_std_17)
    target_compile_options(test_r06_mat_payload PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_mat_raw_payload_boundaries COMMAND test_r06_mat_payload
        "${CMAKE_CURRENT_BINARY_DIR}/r06_mat_raw_payload")
    set_tests_properties(r06_mat_raw_payload_boundaries PROPERTIES
        LABELS "r06;mat;raw-payload;negative;synthetic-only;write-sandbox" TIMEOUT 180)

    add_executable(test_r06_adaptive_runner tests/integration/test_r06_adaptive_runner.cpp)
    target_link_libraries(test_r06_adaptive_runner PRIVATE egomotion_experiments egomotion_matio)
    target_include_directories(test_r06_adaptive_runner PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/experiments")
    target_compile_features(test_r06_adaptive_runner PRIVATE cxx_std_17)
    target_compile_options(test_r06_adaptive_runner PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_adaptive_runner_complete_contract COMMAND test_r06_adaptive_runner
        "${CMAKE_CURRENT_BINARY_DIR}/r06_runner_complete" "${EGOMOTION_BUILD_SOURCE_SHA}" complete)
    add_test(NAME r06_adaptive_runner_incomplete_contract COMMAND test_r06_adaptive_runner
        "${CMAKE_CURRENT_BINARY_DIR}/r06_runner_incomplete" "${EGOMOTION_BUILD_SOURCE_SHA}" incomplete)
    add_test(NAME r06_adaptive_default_undefined_contract COMMAND test_r06_adaptive_runner
        "${CMAKE_CURRENT_BINARY_DIR}/r06_runner_undefined" "${EGOMOTION_BUILD_SOURCE_SHA}" undefined)
    set_tests_properties(r06_adaptive_runner_complete_contract r06_adaptive_runner_incomplete_contract PROPERTIES
        LABELS "r06;runner;fixed-600-row-aggregation;synthetic-step;write-sandbox" TIMEOUT 180)
    set_tests_properties(r06_adaptive_default_undefined_contract PROPERTIES
        LABELS "r06;runner;source-trajectory;undefined-angle;synthetic-step;write-sandbox" TIMEOUT 180)

    add_executable(test_r06_bias_runner tests/integration/test_r06_bias_runner.cpp)
    target_link_libraries(test_r06_bias_runner PRIVATE egomotion_experiments egomotion_matio)
    target_compile_features(test_r06_bias_runner PRIVATE cxx_std_17)
    target_compile_options(test_r06_bias_runner PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_bias_runner_contract COMMAND test_r06_bias_runner contract)
    add_test(NAME r06_bias_runner_publication COMMAND test_r06_bias_runner publication
        "${CMAKE_CURRENT_BINARY_DIR}/r06_bias_runner" "${EGOMOTION_BUILD_SOURCE_SHA}")
    set_tests_properties(r06_bias_runner_contract r06_bias_runner_publication PROPERTIES
        LABELS "r06;bias;runner;bounded-level-0-2;cpp-rng" TIMEOUT 180)
    set_property(TEST r06_bias_runner_publication APPEND PROPERTY LABELS "write-sandbox")

    add_executable(test_r06_bias_injection tests/integration/test_r06_bias_injection.cpp)
    target_link_libraries(test_r06_bias_injection PRIVATE egomotion_experiments)
    target_compile_features(test_r06_bias_injection PRIVATE cxx_std_17)
    target_compile_options(test_r06_bias_injection PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_bias_dependency_injection COMMAND test_r06_bias_injection)
    set_tests_properties(r06_bias_dependency_injection PROPERTIES
        LABELS "r06;bias;runner;dependency-injection;fixed-solvers;memory-only" TIMEOUT 180)

    add_executable(test_r06_adaptive_injection tests/integration/test_r06_adaptive_injection.cpp)
    target_link_libraries(test_r06_adaptive_injection PRIVATE egomotion_experiments)
    target_compile_options(test_r06_adaptive_injection PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_adaptive_dependency_injection COMMAND test_r06_adaptive_injection)
    set_tests_properties(r06_adaptive_dependency_injection PROPERTIES
        LABELS "r06;adaptive;runner;dependency-injection;fixed-solvers;memory-only" TIMEOUT 180)

    add_executable(test_r06_adaptive_scene tests/integration/test_r06_adaptive_scene.cpp)
    target_link_libraries(test_r06_adaptive_scene PRIVATE egomotion_experiments)
    target_compile_options(test_r06_adaptive_scene PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_adaptive_scene_composition COMMAND test_r06_adaptive_scene)
    add_test(NAME r06_adaptive_entry_isolation COMMAND test_r06_adaptive_scene entry-isolation)
    set_tests_properties(r06_adaptive_scene_composition r06_adaptive_entry_isolation PROPERTIES
        LABELS "r06;adaptive;runner;scene-injection;fixed-solvers;memory-only;eight-runs" TIMEOUT 180)

    foreach(case IN ITEMS contract archive)
        add_test(NAME r06_cli_process_${case} COMMAND "${CMAKE_COMMAND}"
            "-DCLI=$<TARGET_FILE:egomotion>"
            "-DSOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}"
            "-DSANDBOX=${CMAKE_CURRENT_BINARY_DIR}/r06_cli_process"
            "-DSOURCE_SHA=${EGOMOTION_BUILD_SOURCE_SHA}" "-DCASE=${case}"
            "-DSVG_ENABLED=${EGOMOTION_ENABLE_SVG}"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_r06_cli_process.cmake")
        set_tests_properties(r06_cli_process_${case} PROPERTIES
            LABELS "r06;cli;real-process;write-sandbox;bounded-bias;readonly-archive" TIMEOUT 180)
    endforeach()

    add_executable(r06_analysis_fixture tests/integration/r06_analysis_fixture.cpp)
    target_link_libraries(r06_analysis_fixture PRIVATE egomotion_matio PkgConfig::MATIO)
    target_compile_features(r06_analysis_fixture PRIVATE cxx_std_17)
    target_compile_options(r06_analysis_fixture PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_analysis_finite_cli COMMAND "${CMAKE_COMMAND}"
        "-DCLI=$<TARGET_FILE:egomotion>" "-DFIXTURE=$<TARGET_FILE:r06_analysis_fixture>"
        "-DSANDBOX=${CMAKE_CURRENT_BINARY_DIR}/r06_analysis_finite"
        "-DSOURCE_SHA=${EGOMOTION_BUILD_SOURCE_SHA}"
        "-DSVG_ENABLED=${EGOMOTION_ENABLE_SVG}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_r06_analysis_finite.cmake")
    set_tests_properties(r06_analysis_finite_cli PROPERTIES
        LABELS "r06;analysis;real-process;independent-polynomial;synthetic-only;write-sandbox" TIMEOUT 180)

    if(EGOMOTION_ENABLE_SVG)
        add_executable(test_r06_svg_windows tests/integration/test_r06_svg_windows.cpp)
        target_link_libraries(test_r06_svg_windows PRIVATE egomotion_plotting)
        target_compile_features(test_r06_svg_windows PRIVATE cxx_std_17)
        target_compile_options(test_r06_svg_windows PRIVATE -Wall -Wextra -Wpedantic)
        add_test(NAME r06_svg_source_windows COMMAND test_r06_svg_windows
            "${CMAKE_CURRENT_BINARY_DIR}/r06_svg_windows" "${EGOMOTION_BUILD_SOURCE_SHA}")
        set_tests_properties(r06_svg_source_windows PROPERTIES
            LABELS "r06;plot;source-limits;svg-geometry;synthetic-only;write-sandbox" TIMEOUT 60)
    endif()

    add_executable(test_r06_motion_errors tests/integration/test_r06_motion_errors.cpp)
    target_include_directories(test_r06_motion_errors PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(test_r06_motion_errors PRIVATE egomotion_contracts)
    target_compile_options(test_r06_motion_errors PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_source_motion_errors COMMAND test_r06_motion_errors)
    set_tests_properties(r06_source_motion_errors PROPERTIES
        LABELS "r06;metrics;analytic;boundaries" TIMEOUT 30)

    add_executable(test_r06_kvd_stopping tests/integration/test_r06_kvd_stopping.cpp)
    target_link_libraries(test_r06_kvd_stopping PRIVATE egomotion_estimators)
    target_compile_options(test_r06_kvd_stopping PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_kvd_stopping_reference COMMAND test_r06_kvd_stopping)
    set_tests_properties(r06_kvd_stopping_reference PROPERTIES
        LABELS "r06;kvd;independent-reduced-reference;bounded-fixtures" TIMEOUT 60)

    add_executable(test_r06_adaptive_conditioning tests/integration/test_r06_adaptive_conditioning.cpp)
    target_include_directories(test_r06_adaptive_conditioning PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_link_libraries(test_r06_adaptive_conditioning PRIVATE egomotion_estimators)
    target_compile_options(test_r06_adaptive_conditioning PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_adaptive_conditioning_reference COMMAND test_r06_adaptive_conditioning)
    set_tests_properties(r06_adaptive_conditioning_reference PROPERTIES
        LABELS "r06;adaptive;independent-analytic;conditioning;bounded-fixtures" TIMEOUT 60)

    add_executable(test_r06_session_interleaving tests/integration/test_r06_session_interleaving.cpp)
    target_link_libraries(test_r06_session_interleaving PRIVATE egomotion_estimators)
    target_compile_options(test_r06_session_interleaving PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_real_session_interleaving COMMAND test_r06_session_interleaving)
    set_tests_properties(r06_real_session_interleaving PROPERTIES
        LABELS "r06;session;real-step;interleaving;bounded-fixtures" TIMEOUT 60)

    add_executable(test_r06_run_context_interleaving tests/integration/test_r06_run_context_interleaving.cpp)
    target_link_libraries(test_r06_run_context_interleaving PRIVATE egomotion_experiments egomotion_matio)
    target_compile_options(test_r06_run_context_interleaving PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_run_context_interleaving COMMAND test_r06_run_context_interleaving
        "${CMAKE_CURRENT_BINARY_DIR}/r06_run_context_interleaving" "${EGOMOTION_BUILD_SOURCE_SHA}")
    set_tests_properties(r06_run_context_interleaving PROPERTIES
        LABELS "r06;run-context;interleaving;real-io;write-sandbox;synthetic-only" TIMEOUT 60)
    add_executable(test_r06_illustrations tests/integration/test_r06_illustrations.cpp)
    target_link_libraries(test_r06_illustrations PRIVATE egomotion_cli)
    target_compile_definitions(test_r06_illustrations PRIVATE "EGOMOTION_ENABLE_SVG=$<BOOL:${EGOMOTION_ENABLE_SVG}>")
    if(EGOMOTION_ENABLE_SVG)
        target_link_libraries(test_r06_illustrations PRIVATE egomotion_illustration_svg)
    endif()
    target_compile_options(test_r06_illustrations PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_illustrations_independent_geometry COMMAND test_r06_illustrations)
    set_tests_properties(r06_illustrations_independent_geometry PROPERTIES
        LABELS "r06;illustrations;independent-rodrigues;optional-svg" TIMEOUT 60)
    add_test(NAME r06_illustrations_real_cli COMMAND "${CMAKE_COMMAND}"
        "-DCLI=$<TARGET_FILE:egomotion>" "-DSOURCE_SHA=${EGOMOTION_BUILD_SOURCE_SHA}"
        "-DSANDBOX=${CMAKE_CURRENT_BINARY_DIR}/r06_illustrations_cli"
        "-DSVG_ENABLED=${EGOMOTION_ENABLE_SVG}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_r06_illustrations_cli.cmake")
    set_tests_properties(r06_illustrations_real_cli PROPERTIES
        LABELS "r06;illustrations;real-process;write-sandbox" TIMEOUT 90)
    add_executable(r06_adaptive_cli_values tests/integration/r06_adaptive_cli_values.cpp)
    target_link_libraries(r06_adaptive_cli_values PRIVATE egomotion_matio ZLIB::ZLIB)
    target_compile_options(r06_adaptive_cli_values PRIVATE -Wall -Wextra -Wpedantic)
    add_executable(r06_adaptive_init_trace tests/integration/r06_adaptive_init_trace.cpp)
    target_link_libraries(r06_adaptive_init_trace PRIVATE egomotion_experiments)
    target_include_directories(r06_adaptive_init_trace PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
    target_compile_options(r06_adaptive_init_trace PRIVATE -Wall -Wextra -Wpedantic)
    add_test(NAME r06_adaptive_real_cli COMMAND "${CMAKE_COMMAND}"
        "-DCLI=$<TARGET_FILE:egomotion>" "-DVALUE_PROBE=$<TARGET_FILE:r06_adaptive_cli_values>"
        "-DINIT_TRACE=$<TARGET_FILE:r06_adaptive_init_trace>"
        "-DSOURCE_ROOT=${CMAKE_CURRENT_SOURCE_DIR}" "-DSOURCE_SHA=${EGOMOTION_BUILD_SOURCE_SHA}"
        "-DSANDBOX=${CMAKE_CURRENT_BINARY_DIR}/r06_adaptive_cli"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/integration/test_r06_adaptive_cli.cmake")
    set_tests_properties(r06_adaptive_real_cli PROPERTIES
        LABELS "r06;adaptive;real-process;real-step;write-sandbox;readonly-l2-diagnostic" TIMEOUT 420)

    # 所有测试在无显示服务的子进程环境运行；不修改调用者的环境。
    get_property(egomotion_tests DIRECTORY PROPERTY TESTS)
    set_tests_properties(${egomotion_tests} PROPERTIES
        ENVIRONMENT_MODIFICATION "DISPLAY=unset:;WAYLAND_DISPLAY=unset:")
endif()

message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Eigen: ${Eigen3_VERSION}")
message(STATUS "Scope: R0-R5 compiled source targets; R6 config/build/tests require separate H-11 authorization")
message(STATUS "BUILD_TESTING=${BUILD_TESTING} (not a test result)")
message(STATUS "EGOMOTION_ENABLE_SVG=${EGOMOTION_ENABLE_SVG}")
