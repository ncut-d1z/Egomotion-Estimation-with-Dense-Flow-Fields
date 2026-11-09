#!/usr/bin/env bash
# Ubuntu 24.04 / GCC 16.2.0 / CMake 4.3.4
# 一键配置、构建、CTest：仅限已存在的低阶球谐模块，不构建尚未实现的其他模块。
set -Eeuo pipefail

ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build/low-order-harmonics}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
JOBS="${JOBS:-4}"
CMAKE_BIN="${CMAKE_BIN:-cmake}"
CTEST_BIN="${CTEST_BIN:-ctest}"

if [[ "${BUILD_DIR}" != /* ]]; then
    BUILD_DIR="${ROOT_DIR}/${BUILD_DIR}"
fi
BUILD_DIR="$(realpath -m -- "${BUILD_DIR}")"

# 只允许在仓库 build/ 下或仓库外构建，防止把构建文件写入 origin 或源码目录。
case "${BUILD_DIR}" in
    "${ROOT_DIR}/build"|"${ROOT_DIR}/build/"*) ;;
    "${ROOT_DIR}"|"${ROOT_DIR}/"*)
        printf 'ERROR: BUILD_DIR must be inside build/ or outside the repository: %s\n' "${BUILD_DIR}" >&2
        exit 2
        ;;
esac

case "${BUILD_TYPE}" in
    Release|Debug|RelWithDebInfo|MinSizeRel) ;;
    *)
        printf 'ERROR: invalid BUILD_TYPE: %s\n' "${BUILD_TYPE}" >&2
        exit 2
        ;;
esac

if [[ ! "${JOBS}" =~ ^[1-9][0-9]*$ ]]; then
    printf 'ERROR: JOBS must be a positive integer: %s\n' "${JOBS}" >&2
    exit 2
fi

# 尊重调用方 CXX；否则优先使用已知 Ubuntu GCC 16 安装，再回退到 PATH 中的 g++。
if [[ -n "${CXX:-}" ]]; then
    CXX_COMPILER="${CXX}"
elif command -v g++-16 >/dev/null 2>&1; then
    CXX_COMPILER="$(command -v g++-16)"
elif command -v g++ >/dev/null 2>&1; then
    CXX_COMPILER="$(command -v g++)"
else
    printf 'ERROR: no GCC C++ compiler found; set CXX=/path/to/g++-16\n' >&2
    exit 2
fi

for executable in "${CMAKE_BIN}" "${CTEST_BIN}" "${CXX_COMPILER}"; do
    if ! command -v "${executable}" >/dev/null 2>&1; then
        printf 'ERROR: command not found: %s\n' "${executable}" >&2
        exit 2
    fi
done

mkdir -p -- "${BUILD_DIR}/logs"
LOG_FILE="${BUILD_DIR}/logs/build-$(date -u +%Y%m%dT%H%M%SZ)-$$.log"
on_error() {
    local status=$?
    printf 'FAILED (exit %s). Log: %s\n' "${status}" "${LOG_FILE}" >&2
}
trap on_error ERR

configure_args=(
    -S "${ROOT_DIR}"
    -B "${BUILD_DIR}"
    "-DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}"
    "-DCMAKE_CXX_COMPILER:FILEPATH=${CXX_COMPILER}"
    "-DBUILD_TESTING:BOOL=ON"
    "-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON"
)
if [[ -n "${CMAKE_GENERATOR:-}" ]]; then
    configure_args+=(-G "${CMAKE_GENERATOR}")
fi

{
    printf 'Repository: %s\n' "${ROOT_DIR}"
    printf 'Git HEAD: %s\n' "$(git -C "${ROOT_DIR}" rev-parse HEAD 2>/dev/null || printf unknown)"
    printf 'Build dir: %s\nBuild type: %s\nParallel jobs: %s\n' "${BUILD_DIR}" "${BUILD_TYPE}" "${JOBS}"
    printf 'C++ compiler: %s\n' "${CXX_COMPILER}"
    "${CMAKE_BIN}" --version | sed -n '1p'
    "${CXX_COMPILER}" --version | sed -n '1p'
    printf 'C++ standard: C++17 (project contract; compiler may support up to C++23)\n'
    printf 'Scope: egomotion_harmonics + test_low_order_harmonics only\n'
    printf 'Log: %s\n' "${LOG_FILE}"
} 2>&1 | tee -a "${LOG_FILE}"

printf '\n[1/3] CMake configure\n' | tee -a "${LOG_FILE}"
"${CMAKE_BIN}" "${configure_args[@]}" 2>&1 | tee -a "${LOG_FILE}"

printf '\n[2/3] Build\n' | tee -a "${LOG_FILE}"
"${CMAKE_BIN}" --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --parallel "${JOBS}" 2>&1 | tee -a "${LOG_FILE}"

# 仅执行一次 CTest：verbose 在通过时也保留 PASS 检查数、样本数和最大残差。
# 不再要求用户手动重复运行 ctest -V；pipefail 确保测试失败仍使脚本失败。
printf '\n[3/3] CTest (verbose; single run)\n' | tee -a "${LOG_FILE}"
"${CTEST_BIN}" --test-dir "${BUILD_DIR}" -C "${BUILD_TYPE}" \
    --verbose --no-tests=error --parallel "${JOBS}" 2>&1 | tee -a "${LOG_FILE}"

printf '\nPASS: build and CTest finished. Log: %s\n' "${LOG_FILE}" | tee -a "${LOG_FILE}"
