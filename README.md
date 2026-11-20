# Egomotion Estimation with Dense Flow Fields

> **最新接续：启动异常与 MAT 扫描歧义，仅代码（2026-11-21）**。实际应用装配的标准异常现进入既有 OutputFailed/退出6回退；MAT 扫描在按名读取前拒绝重复后端名称，避免把首项结构冒充后续结构。两个既有目标补充回归源码，记录器目标增加真实入口链接；具体依据与边界见 [本批记录](docs/reviews/R6_STARTUP_MAT_SCAN_CONTINUATION_2026-11-21.md)。**构建/测试继续延期，没有新增运行证据；R6 总体验收未完成，PR #6 保持 Draft。**

> **前批接续：MAT 名称选择与字典指纹，仅代码（2026-11-20）**。原始 MAT 载荷的变量选择与 matio 的首个 NUL 截断语义一致，补齐已有 single 存储拒绝规则；FNV 指纹固定 classic locale。该批核对生产装配与 62 个 CMake 源路径，当前已列明的代码范围具备源码级闭合条件；依据见 [前批记录](docs/reviews/R6_MAT_SELECTION_LOCALE_CONTINUATION_2026-11-20.md)。该结论不等于正确性或验收，构建/测试仍未执行。

> **前批接续：输出目录异常所有权，仅代码（2026-11-20）**。修复 RunDirectory 在路径或 RunId 复制失败时可能提前释放并泄漏目录句柄的问题：构造成功后才移交所有权；补充逐分配点故障与正常析构回归源码。具体依据与限定范围见 [前批记录](docs/reviews/R6_DIRECTORY_OWNERSHIP_CONTINUATION_2026-11-20.md)。该批同样没有构建/测试证据。

> **前批接续：插图数值与记录目录身份，仅代码（2026-11-20）**。插图法向/光照溢出在整数颜色转换前拒绝，保留原零面积面行为；进程记录目录已创建但打开/确认失败时保留实际路径，碰撞不冒认本次记录。实现和待执行回归见 [前批记录](docs/reviews/R6_ILLUSTRATION_RECORD_CONTINUATION_2026-11-20.md)。该批同样没有构建/测试证据。

> **前批接续：停止阈值与输出完整性，仅代码（2026-11-20）**。拒绝 KvD 动态阈值分母溢出；防止格式化失败后的截断 JSON/SVG 或字典指纹被当作成功结果；补齐分析分段及 MAT 哈希失败的来源诊断。实现和待执行回归见 [前批记录](docs/reviews/R6_FORMATTING_FAILURE_CONTINUATION_2026-11-20.md)。该批同样没有构建/测试证据。

> **前批接续：发布封存与错误身份，仅代码（2026-11-20）**。确认发布 Manifest 后封存 RunContext 的计数、错误与输出入口；MAT 具名读取失败保留原请求变量；有限轨迹向量的模长溢出改报计算结果失败。实现及待执行回归见 [前批记录](docs/reviews/R6_LIFECYCLE_DIAGNOSTICS_CONTINUATION_2026-11-20.md)。该批同样没有构建/测试证据。

> **前批接续：失败边界，仅代码（2026-11-20）**。补齐 Session 对成功 Step 载荷的有限性检查、未确认产物的内存/Manifest 状态一致性，以及实验标准异常的 RunId/失败记录保留。生产修改和待执行回归见 [前批记录](docs/reviews/R6_FAILURE_BOUNDARY_CONTINUATION_2026-11-20.md)。该批同样没有构建/测试证据。

> **前批接续：仅代码，构建/测试延期（2026-11-20）**。按所有者最新指令，继续补齐聚合缺测证据、有限输入平滑/SVG 溢出拒绝及分析运行记录的指标身份；本批源码与必要回归源码已修改，**未配置、未编译、未测试**。改动和静态依据见 [前批记录](docs/reviews/R6_CODE_CONTINUATION_2026-11-20.md)。下文 12+1 项及更早通过记录仅对应其固定旧 SHA，不能外推到当前生产源码；PR #6 保持 Draft。

> **2026-11-20 R6 接续**：按所有者已确认的 [DEC-R6-01～04](docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-20.md)，实现稳定平移方向角、SO(3) 旋转测地角、Session 指标有效性与求解状态分离，以及方案 B 的部分有效 MAT/Manifest/分析输出。新 Adaptive 输出带版本标记与有效重复次数；新旋转指标不能与历史“旋转轴方向角”直接混用。当前验证状态与环境限制见 [本轮记录](docs/reviews/R6_METRIC_DECISIONS_IMPLEMENTATION_2026-11-20.md)。下面 2026-11-18 的 73/72 项结果只证明其固定旧 SHA，不能外推到本轮实现；科学复现、最终数值资格与正式验收继续延期。

> **本轮实际验证**：远端受测 `594a1b11f91028cb7549fef0ab1cd512ee4f7edb` 在 Debian 13.7 / GCC 14.2.0 / Eigen 3.4.0 编译 29 个生产文件、11 个测试程序，有限子集 **12/12 通过**；另 13 个文件编译器语法检查通过。后续 `2ed5c2d95474eb34977d6998fa70330226437614` 在生产源码不变、复用已验证静态库的前提下，新增独立 Session 状态测试 **1/1 通过**（1,706 项、48 成功帧、6 求解回滚、3 策略回滚）。这是分两次运行的 12+1 项，不是完整 CMake/CTest。当前环境缺少 CMake、matio、Sophus 开发依赖，未安装依赖、未执行完整 CMake/CTest 或真实新 MAT/CLI，不能将上述子集当作全工程验证。

A planned **C++17 reimplementation that directly reads the original MATLAB MAT files** of the scripts accompanying the 2015 research paper *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach*.

> **Permanent evidence policy:** Running MATLAB/Octave or collecting original MATLAB L1 execution traces is **not required** for design, coding, verification, or release. The original scripts are read-only specifications; numerical evidence will use the preserved MAT results (L2), independently checked analytic/reference implementations (L3), and statistical comparisons (L4). See [No MATLAB Execution Policy](docs/golden/NO_MATLAB_EXECUTION_POLICY.md). This change does not authorize C++ builds/tests or imply any numerical acceptance.

> **项目状态（2026-11-18）**：受测源码`3985b056aa19935c8348f0fefa5f5292f9fbd3b4`两模式配置/编译无诊断，**SVG ON73/73、OFF72/72 CTest**；先MAT7项，再66/65项，零失败/跳过/禁用。新增合成固定输入→真实Adaptive Step/Session→完整35/600帧及8轮聚合→MAT交付独立对照：3676721项、17441次Step、12个MAT/8540值通过，保存值最大差5.406439324458240113e-10，沿用原1e-9工程余量。两配置Step JSONL与MAT数值载荷相同，739项输入SHA及21组既有摘要不变。见 [完整交付链证据](docs/reviews/R6_REAL_STEP_DELIVERY_L3_2026-11-18.md)。合成场景不等于原科学实验成功；原轨迹三入口仍Incomplete、无完整科学MAT，L4、病态资格及正式验收未完成。Issue #1–#5开放，已建立[Draft PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6)，R6未结案。

## Original research and source material

This repository is an **independent, unofficial porting project** based on the following works. It is neither an official release by the original authors nor affiliated with Bielefeld University, DLR, or PLOS.

### Research paper

Simon Strübbe, Wolfgang Stürzl, and Martin Egelhaaf (2015).  
**Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach.**  
*PLOS ONE*, **10**(8), e0128413.  
**DOI:** https://doi.org/10.1371/journal.pone.0128413

The paper investigates self-motion estimation from dense optical flow, relates the Koenderink–van Doorn (KvD) estimator to the matched filter approach (MFA), proposes a modified KvD estimator, and develops an adaptive MFA with a compact depth representation.

### Original MATLAB scripts and data

Simon Strübbe, Wolfgang Stürzl, and Martin Egelhaaf (2015).  
**Egomotion Estimation with Dense Flow Fields: MATLAB Scripts.**  
Bielefeld University, Publications Server (PUB).  
**DOI:** https://doi.org/10.4119/unibi/2736885  
**Repository record:** https://pub.uni-bielefeld.de/record/2736885

This separately archived research artifact provides MATLAB scripts and associated data for the numerical studies described in the paper. In particular, the accompanying material describes experiments comparing:

1. The original KvD algorithm with its modified version.
2. The standard MFA (Franz–Krapp) with the proposed adaptive MFA.

**Relationship to this project:** The archived MATLAB artifact is the intended *reference implementation and input-data source*. The paper is the *scientific and mathematical reference*. This repository aims to build a clearly attributed, inspectable C++ implementation and to read the original MATLAB MAT data directly through matio, without changing the scientific meaning of the experiments.

## Intended scope

- Port the MATLAB computational routines to C++ while documenting algorithmic correspondence to the original source.
- Preserve and read the original MATLAB .mat files directly through matio, without changing variable names, dimensions, numeric types or MATLAB column-major layout.
- Document provenance and transformations for any redistributed data.
- Verify bounded numerical correspondence using existing MAT archives (L2), independently produced deterministic references (L3), and statistics (L4); **do not require running original MATLAB scripts or acquiring L1**. Explicitly disclose any original MATLAB runtime behavior that cannot be verified.

**实现与验证边界：** C++数值求解器及实验管线源码已实现，局部固定输入测试已执行；完整实验复现仍待验证。NumPy NPZ转换不在范围内。

## Project MIT license, references and implementation direction

**Repository-authored C++ software and documentation use the [MIT License](LICENSE)** (Copyright 2026 ncut-d1z and contributors). This project license is explicitly selected by the repository owner. Source attribution is ordinary documentation, **not a repeated copyright-approval gate for development**. If a third-party file is actually included, keep its existing applicable notices; the repository MIT license does not relicense the original separately authored MATLAB archives, MAT data or scientific papers.

Original research references (retained):
- Strübbe, Stürzl, Egelhaaf (2015), *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach*: https://doi.org/10.1371/journal.pone.0128413
- Archived MATLAB scripts and MAT data: https://doi.org/10.4119/unibi/2736885
- Ivanic & Ruedenberg, 1996 original real-SH rotation recursion: https://doi.org/10.1021/jp953350u
- **1998 mandatory corrections** to the original formulas: https://doi.org/10.1021/jp9833350

**SHRotation implementation is Eigen-only:** mathematical priority is the original 1996 Ivanic–Ruedenberg paper **as corrected in 1998**; Don Williamson's [SHRotation](https://github.com/dwilliamson/SHRotation) is only a code reference. **No sdla headers, source dependency, vendor directory or link target are allowed.** When the user uploads the original paper and correction, implement only l=0/1/2 as original C++17/Eigen logic, preserve `(-1,0,1)` / `(-2,-1,0,1,2)` coefficient ordering, and compare full R3/R5 with independent L3 references. Existing original `origin/**` remains read-only; MATLAB/Octave execution and L1 Golden are permanently not required. See [Eigen SH implementation plan](docs/design/SH_ROTATION_IVANIC_RUEDENBERG.md) and [current owner decisions](docs/decisions/PROJECT_OWNER_DECISIONS_2026-11-07.md).

**CLI output convention:** There is **no user-provided output-root flag**. New results always use the repository's fixed `output/<RunId>/`. The optional `--run-id` accepts a safe unique ID; if omitted, the app uses an OS-generated random ID independent of the experiment RNG. Existing output directories/files are never overwritten.

**可选进程记录器（候选接口）**：从仓库根显式执行 `"$build_dir/egomotion-record" inspect-mat --file PATH`，参数与原CLI一致。stdout返回日志位置收据；原命令stdout/stderr保存在 `build-cli-records/record_<ID>/stdout.bin`、`stderr.bin`，开始/结束为 `start.json`、`finish.json`。该目录由既有build-*忽略规则排除，必须是当前用户私有目录；已有共享build目录保持原权限。RecordId不是实验RunId，科学结果仍固定在output/下。详见 [进程记录证据](docs/reviews/R6_PROCESS_RECORDS_2026-11-18.md)；直接执行egomotion仍保持原输出方式。

`egomotion` 检查终端/重定向输出的写入与刷新。普通输出流错误返回6；stderr也失效时无法保证诊断送达。计算可能已经成功保存MAT与Complete Manifest，而最后的stdout通知交付失败；请核对该RunId下的现有产物，不以通知失败覆盖或删除结果。失败审计在stderr通知前保存，默认SIGPIPE行为保持不变。实际范围见 [输出故障验证](docs/reviews/R6_CLI_STREAM_OUTPUT_2026-11-17.md)。

五个 `run-*` 实验在同一RunId目录保存 `invocation.json`（命令、实际参数、seed、源码与环境）和 `execution.json`（输出尝试、完整交付状态与返回码规则）。结束记录确认发布后，程序返回 `exit_code_if_record_published`；记录自身写入/持久化失败返回6并报错，已有计算Manifest保持原状态。信号终止可能只有开始记录，不能据此认定完成。字段与验证范围见 [运行记录说明](docs/reviews/R6_EXECUTION_RECORDS_2026-11-17.md)。

`plot` 和 `illustrate --format svg` 也在已分配的RunId目录保存这两份记录；`record_scope=artifact_export`从导出前开始，保留实际配置和已绑定的输入来源。没有科学RNG或计算Manifest时明确记null。开始记录未确认不导出，结束记录失败返回6并保留已有文件。只读analyze/JSON插图继续不创建输出目录，详见 [导出记录说明](docs/reviews/R6_ARTIFACT_RECORDS_2026-11-18.md)。

## Minimal C++ MAT dictionary probe

The first C++ utility is [src/mat_data_dict_probe.cpp](src/mat_data_dict_probe.cpp). It discovers all original MAT files, opens each file **read-only** using matio, enumerates names with Mat_GetDir, reads each variable using Mat_VarRead, and generates a Markdown data dictionary. Per-file output includes its location, size, FNV-1a 64-bit change-detection fingerprint, MAT version, variable name, class, decoded data type, dimensions, compression, flags and read status. **FNV is not a cryptographic hash.**

当前探针已复用生产 Reader，不能再单独编译一个 `.cpp`。Debian/Ubuntu 依赖为 C++17、CMake ≥3.28、Eigen3、matio、zlib 和提供 `Sophus::Sophus` CMake target 的 Sophus。常规系统依赖安装命令如下（本轮使用既有安装，未执行依赖安装）：

~~~bash
sudo apt-get install g++ cmake pkg-config libeigen3-dev libmatio-dev zlib1g-dev
~~~

Sophus 需预先安装并可由 `find_package(Sophus CONFIG)` 找到；本次实际版本及其余依赖版本见验证记录。遵守 [AGENTS.md](AGENTS.md) 的执行授权和提交/推送检查点后，使用独立目录构建全部目标并运行测试：

~~~bash
build_dir="build/r6-$(date -u +%Y%m%dT%H%M%SZ)-$$"
cmake -S . -B "$build_dir" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "$build_dir" --parallel 4
ctest --test-dir "$build_dir" --verbose --no-tests=error \
    -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries|r06_mat_snapshot_identity|r06_mat73_version_rejection)$'
ctest --test-dir "$build_dir" --verbose --no-tests=error \
    -E '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries|r06_mat_snapshot_identity|r06_mat73_version_rejection)$'
~~~

`EGOMOTION_ENABLE_SVG` 默认 `ON`。需要完全不编译/链接 SVG 导出器时，在新的构建目录配置 `-DEGOMOTION_ENABLE_SVG=OFF`，其余构建和测试命令相同。两种模式都保留 `inspect-mat`、实验 Runner、`analyze` 和 `illustrate` 的默认 JSON；关闭 SVG 后，合法数值输入的 `plot` / `illustrate --format svg` 返回 `BackendUnavailable`（退出码7），不预留输出目录。数值输入自身的失败仍先按原合同报告。CTest 在两种模式都移除子进程的 `DISPLAY` / `WAYLAND_DISPLAY`；仅 SVG 专用画幅测试及混合测试中的 SVG 导出断言随关闭而缺席，全部原数值测试保留。准备/实测状态见 [无绘图构建记录](docs/reviews/R6_HEADLESS_BUILD_2026-11-17.md)。

构建产物包含 `egomotion` 和 `mat_data_dict_probe`。探针生成新报告时应指定新的隔离输出文件；下列已提交的数据字典保留其原始来源，不因新 Reader 测试通过而自动重新生成。逐步骤 stdout/stderr、退出码及工具链身份须按仓库规则另行保存，本次实际命令与日志索引见验证记录。

The default expected count is **582 MAT files**, adjustable with `--expected-count`. Exit code 0 requires the expected count, successful reads, report write/close, and status delivery; 1 indicates a count or read failure; 2 indicates a parameter, execution, report, or status-delivery failure. Diagnostics use stderr. It refuses to write inside the original source data tree; any existing generated report can only be replaced using the explicit --replace-generated option and a matching generator marker.

探针通知失败时保留已有报告；写入失败可能留下部分报告，不能仅凭文件存在认定成功。实际`/dev/full`及报告write/close负例见 [探针验证](docs/reviews/R6_DICTIONARY_OUTPUT_2026-11-17.md)。

The checked-in [docs/MAT_DATA_SCHEMA.md](docs/MAT_DATA_SCHEMA.md) is now the **actual matio-generated report**, identified by its `generated-by` marker, matio version and `READ_OK` records. Its FNV-1a64 per-file fingerprints are **not cryptographic SHA-256 hashes**. Successful source-file decoding is distinct from numerical equivalence, reader/writer contract tests, corrupted-input tests, or a successful solver build.

### Verified source MAT data schema

历史报告由 **Ubuntu 24.04（GCC 13、matio 1.5.26）** 上的探针生成：582个MAT v5/zlib文件均为单变量 MATLAB `double` class，完整解码后为 double，全部 `READ_OK`。**2026-11-17 独立原始标签核查纠正**：物理实部存储标签是502个 `miDOUBLE` 和80个 `miINT32`（两类Bias的全部 `xwerte.mat`），不能把解码后的类型叫作物理存储类型。各文件变量名与文件基名相同，来源/候选SHA清单及独立Reader对照见 [MAT存储验证](docs/reviews/R6_MAT_STORAGE_VALIDATION_2026-11-17.md)。历史生成字典保持原样。

| Variable shape | Number of archived variables |
|---|---:|
| 10×1 | 80 |
| 10×3 | 480 |
| 35×1 | 4 |
| 600×1 | 14 |
| 600×4 | 4 |

This dataset requires **MAT v5 + zlib + dense real double support**, and no source file in this collection requires MAT v7.3/HDF5, cell, struct, sparse or complex data handling. This finding applies **only to the current 582 archived files**. The historical MAT files remain read-only; future inputs with additional types need separate requirements and error handling.

**当前已验证：** 582/582 原始 MAT 只读解码、33340值独立逐位对照、输入前后 SHA-256、合成 MAT 写回读及限定类型/损坏/竞态负例，见 [存储验证](docs/reviews/R6_MAT_STORAGE_VALIDATION_2026-11-17.md) 与 [66 TC证据映射](docs/reviews/R6_TC_EVIDENCE_MAP_2026-11-17.md)。**仍未完成：** 全部L3/L4、论文实验复现和正式验收；不声称支持全部MAT类型或完整输入域。

## 原始插图入口

从仓库根使用本次构建的 `egomotion`。以下候选CLI已在固定SHA测试，合同未冻结；默认JSON只输出数值，SVG结果固定写入新的 `output/<RunId>/`。两轨迹均600点；径向面使用未归一化关联勒让德函数，独立于求解器球谐模块。

~~~bash
"$build_dir/egomotion" illustrate --kind circle
"$build_dir/egomotion" illustrate --kind sinusoidal --format svg
"$build_dir/egomotion" illustrate --kind harmonic --degree 3 --order 2 --format svg
~~~

SVG同时保存同一 `illustration.json`；不允许自定义输出根或覆盖RunId。保持原正弦脚本的圆轨迹标题，用kind区分。球面端点/遮罩及显示灯光的已知边界见 [插图验证](docs/reviews/R6_ILLUSTRATIONS_2026-11-17.md)，不承诺MATLAB逐像素等价。

## Citation

```bibtex
@article{struebbe2015insect,
  author  = {Str{"u}bbe, Simon and St{"u}rzl, Wolfgang and Egelhaaf, Martin},
  title   = {Insect-Inspired Self-Motion Estimation with Dense Flow Fields---An Adaptive Matched Filter Approach},
  journal = {PLOS ONE},
  volume  = {10},
  number  = {8},
  pages   = {e0128413},
  year    = {2015},
  doi     = {10.1371/journal.pone.0128413}
}

@misc{struebbe2015matlab,
  author    = {Str{"u}bbe, Simon and St{"u}rzl, Wolfgang and Egelhaaf, Martin},
  title     = {Egomotion Estimation with Dense Flow Fields: MATLAB Scripts},
  publisher = {Bielefeld University},
  year      = {2015},
  doi       = {10.4119/unibi/2736885}
}
```
