# Egomotion Estimation with Dense Flow Fields

A planned **C++17 reimplementation that directly reads the original MATLAB MAT files** of the scripts accompanying the 2015 research paper *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach*.

> **Permanent evidence policy:** Running MATLAB/Octave or collecting original MATLAB L1 execution traces is **not required** for design, coding, verification, or release. The original scripts are read-only specifications; numerical evidence will use the preserved MAT results (L2), independently checked analytic/reference implementations (L3), and statistical comparisons (L4). See [No MATLAB Execution Policy](docs/golden/NO_MATLAB_EXECUTION_POLICY.md). This change does not authorize C++ builds/tests or imply any numerical acceptance.

> **项目状态（2026-11-17）**：R6仍在验证。最新受测源码 `130a82ba0090e186fcf43bc28bb98c282e68c96c` 增量构建无诊断、**48/48 CTest通过**；MAT封存副本的校验、解码和分析来源SHA-256已绑定，见 [最新验证](docs/reviews/R6_MAT_SNAPSHOT_2026-11-17.md)。真实Adaptive三入口仍因初始化第14帧acos越界或第151帧零旋转返回Incomplete，无MAT；**错误处理通过不等于科学实验成功**。[66项TC证据映射](docs/reviews/R6_TC_EVIDENCE_MAP_2026-11-17.md)列明剩余义务；原始资料哈希不变。

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
    -R '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries)$'
ctest --test-dir "$build_dir" --verbose --no-tests=error \
    -E '^(r02_mat_reader|r06_historical_mat_readonly_schema|r06_mat_independent_storage_values|r06_mat_reader_boundaries|r06_mat_raw_payload_boundaries)$'
~~~

构建产物包含 `egomotion` 和 `mat_data_dict_probe`。探针生成新报告时应指定新的隔离输出文件；下列已提交的数据字典保留其原始来源，不因新 Reader 测试通过而自动重新生成。逐步骤 stdout/stderr、退出码及工具链身份须按仓库规则另行保存，本次实际命令与日志索引见验证记录。

The utility returns exit code 0 only if all **582 expected MAT files** are found and each variable is actually read successfully. It writes diagnostics and returns nonzero otherwise. It refuses to write inside the original source data tree; any existing generated report can only be replaced using the explicit --replace-generated option and a matching generator marker.

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
