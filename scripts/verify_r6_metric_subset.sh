#!/usr/bin/env bash
# R6 有限验证：仅使用现有 C++17 编译器、Eigen 和下列显式源码。
# 执行前由调用者按 AGENTS.md 完成源码提交、远端检查点与 SHA 核实。
# 本脚本不联网、不安装依赖、不修改 CMake，不提供 Sophus/matio 替身。
set -Eeuo pipefail
umask 077

scope='limited subset; not full CMake/CTest or original scientific experiments'

usage() {
    cat <<'USAGE'
Usage: bash scripts/verify_r6_metric_subset.sh EIGEN_INCLUDE_DIR NEW_LOG_DIR

EIGEN_INCLUDE_DIR must contain Eigen/Core and Eigen/src/Core/util/Macros.h.
NEW_LOG_DIR must not exist, and its parent directory must already exist.
Use a directory outside the checkout, or an ignored build/build-* directory.
The checkout must be clean; detached HEAD is supported.
Before invocation, commit the source and verify its remote checkpoint as
required by AGENTS.md. The script does not perform or certify that checkpoint.

Uses /usr/bin/c++, /usr/bin/ar and /usr/bin/timeout; installs nothing.
Compile flags: -std=c++17 -O2 -Wall -Wextra -Wpedantic
This is a limited subset; not full CMake/CTest or original scientific experiments.
USAGE
}

die() {
    printf 'ERROR: %s\n' "$*" >&2
    exit 2
}

[[ $# -eq 2 ]] || { usage >&2; exit 2; }
for argument in "$@"; do
    case "$argument" in
        *$'\n'*|*$'\r'*|*$'\t'*) die 'Arguments cannot contain tabs or newlines.' ;;
    esac
done
for utility in git realpath date uname sha256sum awk mkdir; do
    command -v "$utility" >/dev/null || die "Missing existing utility: $utility"
done

script_path=$(realpath -e -- "${BASH_SOURCE[0]}")
script_dir=${script_path%/*}
repo_dir=$(git -C "$script_dir/.." rev-parse --show-toplevel)
repo_dir=$(realpath -e -- "$repo_dir")
eigen_dir=$(realpath -e -- "$1")
[[ -f "$eigen_dir/Eigen/Core" ]] || die 'Eigen/Core not found.'
eigen_macros="$eigen_dir/Eigen/src/Core/util/Macros.h"
[[ -f "$eigen_macros" ]] || die 'Eigen version header not found.'
[[ ! -e "$2" && ! -L "$2" ]] || die 'Log directory already exists; refusing overwrite.'
output_dir=$(realpath -m -- "$2")
[[ -d "${output_dir%/*}" ]] || die 'The parent of NEW_LOG_DIR must already exist.'
case "$output_dir" in
    "$repo_dir"/build|"$repo_dir"/build/*|"$repo_dir"/build-*) ;;
    "$repo_dir"|"$repo_dir"/*) die 'Logs inside the checkout must use build or build-*.' ;;
esac
case "$output_dir" in
    "$eigen_dir"|"$eigen_dir"/*) die 'Logs cannot be written inside the Eigen dependency.' ;;
esac

compiler=/usr/bin/c++
archiver=/usr/bin/ar
timeout_bin=/usr/bin/timeout
for executable in "$compiler" "$archiver" "$timeout_bin"; do
    [[ -x "$executable" ]] || die "Missing existing executable: $executable"
done
source_sha=$(git -C "$repo_dir" rev-parse HEAD)
source_branch=$(git -C "$repo_dir" rev-parse --abbrev-ref HEAD)
start_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)
run_id="r6-metric-subset_$(date -u +%Y%m%dT%H%M%SZ)_${source_sha:0:12}_$$"

# mkdir 独占领取新目录；不能用 mkdir -p 掩盖目标已存在。
mkdir -- "$output_dir" || die 'Cannot claim a new isolated log directory.'
work_dir="$output_dir/work"
phase=preflight
suite_finished=no
compiled_sources=0
compiled_binaries=0
failed_compilations=0
executed_tests=0
passed_tests=0
failed_tests=0
blocked_tests=0
planned_tests=14

finish() {
    local result=$?
    trap - EXIT
    set +e
    local final_sha
    final_sha=$(git -C "$repo_dir" rev-parse HEAD 2>"$output_dir/git-end.stderr")
    git -C "$repo_dir" status --porcelain=v1 --untracked-files=all \
        >"$output_dir/git-status-end.txt" 2>>"$output_dir/git-end.stderr"
    local git_result=$?
    if [[ $git_result -ne 0 || "$final_sha" != "$source_sha" ||
          -s "$output_dir/git-status-end.txt" ]]; then
        printf 'ERROR: Source checkout changed or its final status could not be verified.\n' >&2
        [[ $result -ne 0 ]] || result=2
    fi
    if [[ $result -eq 0 &&
          ( "$suite_finished" != yes || $passed_tests -ne $planned_tests ) ]]; then
        result=1
    fi
    {
        printf 'end_utc=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)"
        printf 'source_sha_end=%s\nphase_at_exit=%s\n' "$final_sha" "$phase"
        printf 'suite_finished=%s\nexit_code=%s\n' "$suite_finished" "$result"
        printf 'compiled_sources=%s\ncompiled_binaries=%s\nfailed_compilations=%s\n' \
            "$compiled_sources" "$compiled_binaries" "$failed_compilations"
        printf 'planned_tests=%s\nexecuted_tests=%s\npassed_tests=%s\n' \
            "$planned_tests" "$executed_tests" "$passed_tests"
        printf 'failed_tests=%s\nblocked_tests=%s\n' "$failed_tests" "$blocked_tests"
    } >>"$output_dir/run.txt"
    printf '%s\nRunId=%s\nlogs=%s\npassed=%s/%s failed=%s blocked=%s exit=%s\n' \
        "$scope" "$run_id" "$output_dir" "$passed_tests" "$planned_tests" \
        "$failed_tests" "$blocked_tests" "$result"
    exit "$result"
}
trap finish EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

mkdir -- "$output_dir/logs" "$output_dir/objects" "$output_dir/bin" \
    "$output_dir/sandboxes" "$work_dir"
{
    printf 'RunId=%s\nscope=%s\n' "$run_id" "$scope"
    printf 'source_sha=%s\nbranch=%s\nrepo=%s\n' "$source_sha" "$source_branch" "$repo_dir"
    printf 'start_utc=%s\nworking_directory=%s\noutput_directory=%s\n' \
        "$start_utc" "$work_dir" "$output_dir"
    printf 'compiler=%s\ncompiler_resolved=%s\narchiver=%s\n' \
        "$compiler" "$(realpath -e -- "$compiler")" "$archiver"
    printf 'eigen_include=%s\n' "$eigen_dir"
    printf 'LANG=%s\nLC_ALL=%s\n' "${LANG-}" "${LC_ALL-}"
    printf 'CPATH=%s\nCPLUS_INCLUDE_PATH=%s\nLIBRARY_PATH=%s\n' \
        "${CPATH-}" "${CPLUS_INCLUDE_PATH-}" "${LIBRARY_PATH-}"
    printf 'COMPILER_PATH=%s\nGCC_EXEC_PREFIX=%s\n' "${COMPILER_PATH-}" "${GCC_EXEC_PREFIX-}"
    printf 'remote_checkpoint=required_before_invocation; verification_is_external_to_this_script\n'
    printf 'cmake=not_invoked\nctest=not_invoked\nmatio=not_linked\nSophus=not_linked\n'
    printf 'scientific_data=none; synthetic/in-memory fixtures only; no original MAT I/O\n'
    printf 'rng_seed=none; session_l3 --noise consumes a source-defined fixed normal sequence\n'
    printf 'compile_timeout_seconds=300\narchive_timeout_seconds=60\n'
    printf 'test_timeouts=30_or_60_seconds_as_registered_in_CMake\n'
    printf 'core_dump_limit=0\n'
} >"$output_dir/run.txt"
printf '%s\n' "$scope"
printf 'RunId=%s logs=%s\n' "$run_id" "$output_dir"

git -C "$repo_dir" status --porcelain=v1 --untracked-files=all \
    >"$output_dir/git-status-start.txt"
[[ ! -s "$output_dir/git-status-start.txt" ]] || die 'Source checkout must be clean.'
git -C "$repo_dir" show -s --format=fuller "$source_sha" >"$output_dir/commit.txt"
git -C "$repo_dir" ls-tree -r --full-tree "$source_sha" -- \
    include src tests scripts CMakeLists.txt >"$output_dir/source-tree.txt"
uname -a >"$output_dir/os.txt"
if [[ -r /etc/os-release ]]; then cat /etc/os-release >>"$output_dir/os.txt"; fi
awk '/^#define EIGEN_(WORLD|MAJOR|MINOR)_VERSION / {print $2 "=" $3}' \
    "$eigen_macros" >"$output_dir/eigen-version.txt"
sha256sum -- "$eigen_macros" >"$output_dir/eigen-version-header.sha256"
printf 'name\tkind\tstart_utc\tend_utc\texit_code\n' >"$output_dir/commands.tsv"
printf 'test\tbinary\tstatus\texit_code\n' >"$output_dir/tests.tsv"
printf 'test\treason\n' >"$output_dir/excluded.tsv"
printf 'r06_adaptive_session_transaction\tEntire test requires AdaptiveExperimentRunner and TrajectoryKinematics/Sophus; no cases are removed or stubbed.\n' \
    >>"$output_dir/excluded.tsv"
printf 'all_other_CMake_tests\tOutside this explicit subset, including real MAT I/O, CLI/production composition and original scientific experiments.\n' \
    >>"$output_dir/excluded.tsv"

# 每个命令保留可复现 argv、cwd、UTC 起止、stdout、stderr 和原始退出码。
run_logged() {
    local name=$1 kind=$2 limit=$3
    shift 3
    local prefix="$output_dir/logs/$name"
    local started ended result=0
    local command=("$timeout_bin" --signal=TERM --kill-after=5s "$limit" "$@")
    started=$(date -u +%Y-%m-%dT%H:%M:%SZ)
    {
        printf 'cd %q\n' "$work_dir"
        printf '%q ' "${command[@]}"
        printf '\n'
    } >"$prefix.command.sh"
    printf '[%s] %s\n' "$kind" "$name"
    (cd "$work_dir" && "${command[@]}") >"$prefix.stdout" 2>"$prefix.stderr" || result=$?
    ended=$(date -u +%Y-%m-%dT%H:%M:%SZ)
    printf '%s\n' "$result" >"$prefix.exit"
    printf 'start_utc=%s\nend_utc=%s\nexit_code=%s\n' \
        "$started" "$ended" "$result" >"$prefix.meta"
    printf '%s\t%s\t%s\t%s\t%s\n' "$name" "$kind" "$started" "$ended" "$result" \
        >>"$output_dir/commands.tsv"
    return "$result"
}
run_logged environment_compiler environment 30s "$compiler" --version
run_logged environment_archiver environment 30s "$archiver" --version
run_logged environment_timeout environment 30s "$timeout_bin" --version
ulimit -c 0

# 显式白名单：不编译 trajectory/kinematics、MAT 适配器或生产 Runner。
# archive 中未被测试引用的对象不会被强行链接。
production_sources=(
    src/domain/motion_estimate.cpp
    src/domain/harmonic_coefficients.cpp
    src/domain/flow_field.cpp
    src/domain/direction_set.cpp
    src/harmonics/low_order_basis.cpp
    src/harmonics/low_order_rotation.cpp
    src/harmonics/low_order_choi.cpp
    src/harmonics/low_order_source_projection.cpp
    src/harmonics/adapter.cpp
    src/harmonics/real_spherical_harmonics.cpp
    src/harmonics/harmonic_fitter.cpp
    src/harmonics/harmonic_rotator.cpp
    src/sampling/directions.cpp
    src/sampling/random.cpp
    src/simulation/flow.cpp
    src/estimators/adaptive_step.cpp
    src/estimators/adaptive_session.cpp
    src/estimators/coefficient_update.cpp
    src/experiments/output_contract.cpp
    src/experiments/output_values.cpp
    src/experiments/run_context.cpp
    src/experiments/manifest_json.cpp
    src/experiments/adaptive_aggregation.cpp
    src/analysis/reader_helpers.cpp
    src/analysis/result_analyzer.cpp
    src/analysis/bias.cpp
    src/analysis/adaptive.cpp
    src/analysis/dataset_json.cpp
    src/analysis/svg_plot.cpp
)
test_sources=(
    tests/integration/test_r06_motion_errors.cpp
    tests/contracts/test_r06_session_metric_validity.cpp
    tests/contracts/test_r06_adaptive_periods.cpp
    tests/contracts/test_r06_coefficient_policies.cpp
    tests/integration/test_r06_session_interleaving.cpp
    tests/integration/test_r06_session_l3.cpp
    tests/integration/test_r06_partial_adaptive_output.cpp
    tests/contracts/test_r06_adaptive_aggregation.cpp
    tests/contracts/test_r05_manifest.cpp
    tests/integration/test_r06_partial_analysis.cpp
    tests/integration/test_r06_analyzer_services.cpp
    tests/integration/test_r06_analysis_segments.cpp
    tests/integration/test_r06_svg_windows.cpp
)
test_specs=(
    'r06_source_motion_errors:test_r06_motion_errors:30s:none'
    'r06_session_metric_validity:test_r06_session_metric_validity:60s:none'
    'r06_adaptive_periods_atomic_commit:test_r06_adaptive_periods:60s:none'
    'r06_coefficient_policy_composition:test_r06_coefficient_policies:60s:none'
    'r06_real_session_interleaving:test_r06_session_interleaving:60s:none'
    'r06_session_independent_multiframe:test_r06_session_l3:60s:sandbox'
    'r06_noisy_session_independent_multiframe:test_r06_session_l3:60s:noise'
    'r06_partial_adaptive_output:test_r06_partial_adaptive_output:60s:none'
    'r06_adaptive_aggregation_evidence:test_r06_adaptive_aggregation:60s:none'
    'r05_manifest_gate:test_r05_manifest:30s:none'
    'r06_partial_analysis:test_r06_partial_analysis:60s:none'
    'r06_analyzer_service_contract:test_r06_analyzer_services:60s:sandbox'
    'r06_analysis_segments_smoothing:test_r06_analysis_segments:60s:none'
    'r06_svg_source_windows:test_r06_svg_windows:60s:svg'
)
[[ ${#test_specs[@]} -eq $planned_tests ]] || die 'Internal test-plan count mismatch.'
printf 'planned_production_sources=%s\nplanned_binaries=%s\n' \
    "${#production_sources[@]}" "${#test_sources[@]}" >>"$output_dir/run.txt"
printf '%s\n' "${production_sources[@]}" >"$output_dir/production-sources.txt"
printf '%s\n' "${test_sources[@]}" >"$output_dir/test-sources.txt"
printf '%s\n' "${test_specs[@]}" >"$output_dir/test-plan.txt"
for source in "${production_sources[@]}" "${test_sources[@]}"; do
    [[ -f "$repo_dir/$source" ]] || die "Required source is missing: $source"
    git -C "$repo_dir" ls-files --error-unmatch -- "$source" >/dev/null \
        || die "Source is not tracked in the recorded checkpoint: $source"
done

# 记录实际文件字节；全部头与 src 也在内，测试后复核不会只相信 HEAD 名称。
git -C "$repo_dir" ls-files -z -- include src tests/support \
    "${test_sources[@]}" scripts/verify_r6_metric_subset.sh CMakeLists.txt \
    >"$output_dir/source-inputs.list0"
while IFS= read -r -d '' source; do
    sha256sum -- "$repo_dir/$source"
done <"$output_dir/source-inputs.list0" >"$output_dir/source-inputs.sha256"

flags=(-std=c++17 -O2 -Wall -Wextra -Wpedantic
       -I "$repo_dir/include" -I "$repo_dir/src"
       -I "$repo_dir/src/experiments" -I "$eigen_dir")
printf '%q ' "${flags[@]}" >"$output_dir/compile-flags.txt"
printf '\n' >>"$output_dir/compile-flags.txt"
objects=()
phase=compile_production
for source in "${production_sources[@]}"; do
    object_name=${source//\//_}.o
    object="$output_dir/objects/$object_name"
    if run_logged "compile_$object_name" compile 300s \
        "$compiler" "${flags[@]}" -c "$repo_dir/$source" -o "$object"; then
        compiled_sources=$((compiled_sources+1))
    else
        result=$?
        failed_compilations=$((failed_compilations+1))
        exit "$result"
    fi
    objects+=("$object")
done
archive="$output_dir/libegomotion_metric_subset.a"
phase=archive
run_logged archive_production archive 60s "$archiver" rcsD "$archive" "${objects[@]}"

phase=compile_tests
declare -A built=()
for source in "${test_sources[@]}"; do
    binary=${source##*/}
    binary=${binary%.cpp}
    if run_logged "compile_$binary" compile 300s \
        "$compiler" "${flags[@]}" "$repo_dir/$source" "$archive" \
        -o "$output_dir/bin/$binary"; then
        built["$binary"]=1
        compiled_binaries=$((compiled_binaries+1))
    else
        built["$binary"]=0
        failed_compilations=$((failed_compilations+1))
    fi
done

phase=run_tests
for spec in "${test_specs[@]}"; do
    IFS=: read -r test_name binary limit arguments <<<"$spec"
    if [[ ${built["$binary"]:-0} -ne 1 ]]; then
        printf '%s\t%s\tblocked_by_compile_failure\tnot_run\n' "$test_name" "$binary" \
            >>"$output_dir/tests.tsv"
        blocked_tests=$((blocked_tests+1))
        continue
    fi
    command=("$output_dir/bin/$binary")
    case "$arguments" in
        none) ;;
        sandbox) command+=("$output_dir/sandboxes/$test_name") ;;
        noise) command+=("$output_dir/sandboxes/$test_name" --noise) ;;
        svg) command+=("$output_dir/sandboxes/$test_name" "$source_sha") ;;
        *) die "Unknown internal test argument mode: $arguments" ;;
    esac
    executed_tests=$((executed_tests+1))
    if run_logged "test_$test_name" test "$limit" "${command[@]}"; then
        passed_tests=$((passed_tests+1))
        printf '%s\t%s\tpassed\t0\n' "$test_name" "$binary" >>"$output_dir/tests.tsv"
    else
        result=$?
        failed_tests=$((failed_tests+1))
        printf '%s\t%s\tfailed\t%s\n' "$test_name" "$binary" "$result" \
            >>"$output_dir/tests.tsv"
    fi
done

phase=verify_source_identity
run_logged verify_source_bytes audit 30s sha256sum --check --quiet "$output_dir/source-inputs.sha256"
run_logged verify_eigen_version_header audit 30s sha256sum --check --quiet \
    "$output_dir/eigen-version-header.sha256"
suite_finished=yes
phase=finished
[[ $failed_compilations -eq 0 && $failed_tests -eq 0 && $blocked_tests -eq 0 ]]
