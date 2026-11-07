# Egomotion Estimation with Dense Flow Fields

A planned **C++17 reimplementation that directly reads the original MATLAB MAT files** of the scripts accompanying the 2015 research paper *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach*.

> **Permanent evidence policy:** Running MATLAB/Octave or collecting original MATLAB L1 execution traces is **not required** for design, coding, verification, or release. The original scripts are read-only specifications; numerical evidence will use the preserved MAT results (L2), independently checked analytic/reference implementations (L3), and statistical comparisons (L4). See [No MATLAB Execution Policy](docs/golden/NO_MATLAB_EXECUTION_POLICY.md). This change does not authorize C++ builds/tests or imply any numerical acceptance.

> **Project status:** The original MATLAB code and data remain unchanged under `origin/`. The independent C++17 `mat_data_dict_probe.cpp` was compiled and run on Ubuntu 24.04 with **matio 1.5.26**: all **582 original MAT files and 582 variables** were read successfully, with no reported failures. The numerical solvers and production Reader/Writer have **not** been implemented or validated, and this does not establish MATLAB/C++ algorithm equivalence.

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

**Not yet implemented:** No C++ numerical solver or validated experiment reproduction pipeline exists. NumPy NPZ conversion is deliberately out of scope.

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

The first C++ utility is [src/mat_data_dict_probe.cpp](src/mat_data_dict_probe.cpp). It discovers all original MAT files, opens each file **read-only** using matio, enumerates names with Mat_GetDir, reads each variable using Mat_VarRead, and generates a Markdown data dictionary. Per-file output includes its location, size, FNV-1a 64-bit change-detection fingerprint, MAT version, variable name, class, stored data type, dimensions, compression, flags and read status. **FNV is not a cryptographic hash.**

On Debian or Ubuntu:

~~~bash
sudo apt-get install g++ pkg-config libmatio-dev zlib1g-dev
mkdir -p build
g++ -std=c++17 -O2 -Wall -Wextra -pedantic \
    src/mat_data_dict_probe.cpp -o build/mat_data_dict_probe \
    $(pkg-config --cflags --libs matio)

./build/mat_data_dict_probe \
    origin/egomotion_estimation_dense_flow_fields_matlab_scripts \
    docs/MAT_DATA_SCHEMA.md \
    --expected-count 582 --source-sha "$(git rev-parse HEAD)"
~~~

The utility returns exit code 0 only if all **582 expected MAT files** are found and each variable is actually read successfully. It writes diagnostics and returns nonzero otherwise. It refuses to write inside the original source data tree; any existing generated report can only be replaced using the explicit --replace-generated option and a matching generator marker.

The checked-in [docs/MAT_DATA_SCHEMA.md](docs/MAT_DATA_SCHEMA.md) is now the **actual matio-generated report**, identified by its `generated-by` marker, matio version and `READ_OK` records. Its FNV-1a64 per-file fingerprints are **not cryptographic SHA-256 hashes**. Successful source-file decoding is distinct from numerical equivalence, reader/writer contract tests, corrupted-input tests, or a successful solver build.

### Verified source MAT data schema

The checked-in report was generated by the independent probe on **Ubuntu 24.04 (GCC 13, matio 1.5.26)**. The archived data comprises **582 MAT v5 files, 582 zlib-compressed variables, all MATLAB `double` class with `double` physical data type**, and all `READ_OK`. Each MAT file contains one variable with the same base name as the file.

| Variable shape | Number of archived variables |
|---|---:|
| 10×1 | 80 |
| 10×3 | 480 |
| 35×1 | 4 |
| 600×1 | 14 |
| 600×4 | 4 |

This dataset requires **MAT v5 + zlib + dense real double support**, and no source file in this collection requires MAT v7.3/HDF5, cell, struct, sparse or complex data handling. This finding applies **only to the current 582 archived files**. The historical MAT files remain read-only; future inputs with additional types need separate requirements and error handling.

**Verified:** 582/582 opened and decoded by matio. **Not yet validated:** element-by-element independent L2/L3 Golden comparison, source-file cryptographic checksums before/after, new MAT writing/readback, strict/compatibility Reader behavior, and full numerical experiment reproduction.

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
