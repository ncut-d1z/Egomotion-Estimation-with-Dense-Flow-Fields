# Egomotion Estimation with Dense Flow Fields

A planned **C++17 reimplementation that directly reads the original MATLAB MAT files** of the scripts accompanying the 2015 research paper *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach*.

> **Project status:** The original MATLAB code and data remain under origin/. A first read-only C++ matio data-dictionary probe is now present under src/; the numerical solvers have not been ported. No matio execution or numerical equivalence results are claimed yet.

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
- Eventually check numerical agreement and reproducibility against the original MATLAB implementation.

**Not yet implemented:** No C++ numerical solver or validated experiment reproduction pipeline exists. NumPy NPZ conversion is deliberately out of scope.

## Copyright, attribution, and licensing

The **research paper** is published under Creative Commons Attribution (CC BY 4.0). This is a statement about the paper; it must **not** be assumed to license the separately archived MATLAB code or data.

The [original MATLAB artifact](https://doi.org/10.4119/unibi/2736885) is credited to **Simon Strübbe, Wolfgang Stürzl, and Martin Egelhaaf (2015)**. Its Bielefeld University record lists **Open Data Commons Attribution License (ODC-By) v1.0** under **“Copyright und Lizenzen”**. See the [official summary](https://opendatacommons.org/licenses/by/summary/index.html), [full legal text](https://opendatacommons.org/licenses/by/1-0/), and the original record's license notice. This is **distinct from the paper's CC BY 4.0 license**.

**Important scope distinction:** ODC-By 1.0 licenses database rights and applicable rights in the database, but Section 2.3(a) explicitly excludes computer programs used to make or operate the database, while Section 2.4 cautions that individual database contents may have separate rights. Accordingly, the record's ODC-By label alone is **not proof that every MATLAB `.m` script is licensed as computer code under ODC-By 1.0**. Preserve any individual file notices and verify the original archive's contents before copying, porting, or redistributing the scripts. For any redistributed data, check the applicable database and individual-content rights and retain appropriate citations and notices.

This repository currently does **not** contain a root LICENSE file or declare a repository-wide license. The original paper's CC BY 4.0 and source artifact's ODC-By 1.0 notices remain attributable to their respective original works; neither automatically licenses future C++ code. The new C++ probe currently has no repository-wide license designation; no license is inferred or added by this documentation. Any later decision to license or redistribute the project's new code must be made explicitly.

When referencing or using this work, please cite the original paper and the MATLAB artifact separately using the two DOIs above. Attribution to this repository is not a substitute for crediting the original authors.

## Third-party MATLAB code: authorship and licensing findings

The two MATLAB files discussed below are preserved in
[origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions](origin/egomotion_estimation_dense_flow_fields_matlab_scripts/help_functions).
The PLOS article's CC BY 4.0 terms and the archived dataset's ODC-By 1.0 label do **not** automatically grant a software license to third-party routines embedded in the MATLAB archive.

### SHRotate.m — real spherical harmonic rotation

**What is in this exact source file:** Its historical header identifies the MATLAB author as **bjian (Bing Jian)**, revision 1.4 dated **2007-12-27**. The introduction describes a MATLAB translation based on C++ code previously at mathlib.zfx.info, thought by the author to derive from **Don Williamson's SHRotation**. It cites **Joseph Ivanic and Klaus Ruedenberg**, whose 1996 paper and 1998 corrections describe the recursive mathematical algorithm; those papers are not software licenses.

**What external licenses establish:** MathWorks' [Real-Valued Spherical Harmonics package by Bing Jian](https://www.mathworks.com/matlabcentral/fileexchange/15377-real-valued-spherical-harmonics) is described as **BSD License** for the publicly listed release (2016); it implements Ivanic–Ruedenberg rotation. The [Don Williamson SHRotation repository](https://github.com/dwilliamson/SHRotation) has a separate [MIT LICENSE](https://github.com/dwilliamson/SHRotation/blob/master/LICENSE) naming Don Williamson and 2004–2016.

**What remains unverified:** The 2007 MATLAB revision in this repository has not been byte-for-byte or revision-for-revision matched to Bing Jian's later BSD release. The Williamson MIT license governs the code covered by that repository, not automatically any independently authored modification or differently sourced copy. The exact upstream mathlib.zfx.info revision and the complete applicable notices have not been recovered. Consequently, one cannot label this precise MATLAB file BSD/MIT solely on the basis of these leads.

**Porting implication:** Preserve the original header, verify the exact upstream version and the corresponding BSD/MIT notice before copying or closely translating code, and comply with any attribution/disclaimer conditions. Independently implementing the published mathematical algorithm is a separate approach and still requires faithful numerical comparison.

### rodrigues.m — rotation vector and matrix conversion

**What is in this exact source file:** The file explicitly states **Copyright (c) March 1993 — Pietro Perona, California Institute of Technology**. Notes credit **Jean-Yves Bouguet** with Jacobian review in **1995**, a 180-degree rotation fix in **1997**, and an SVD projection modification in **2003**. These are identifiable source contributors, not merely scientific references.

**Upstream confirmation:** Jean-Yves Bouguet's [Camera Calibration Toolbox for MATLAB documentation](https://robots.stanford.edu/cs223b04/JeanYvesCalib/) and [CaltechDATA software record](https://data.caltech.edu/records/jx9cx-fdh55) support attribution to the Bouguet toolbox. A [public copy of rodrigues.m](https://github.com/fragofer/TOOLBOX_calib/blob/master/rodrigues.m) preserves similar Perona/Bouguet notices but has later changes; its existence is not itself a license grant.

**What remains unverified:** The uploaded rodrigues.m does not contain an explicit MIT, BSD, Apache or GPL license nor an affirmative permission-to-redistribute clause. We did not verify a primary-source software license granting redistribution or close translation of this specific historical file. Calling a toolbox “open source,” hosting it publicly, or citing the original paper is not sufficient to determine the exact license.

**Porting implication:** Keep the historical Perona/Bouguet notice intact. Treat direct reuse or close translation as **pending rights clearance**. For the experimentally required rotation-vector-to-matrix calculation, separately authored code implementing the standard Rodrigues mathematics is a possible alternative, with source attribution and numerical fidelity checks.

**Scope of this review:** These are factual source/license leads, not a definitive legal opinion. Neither identified third-party license is being applied to the entire MATLAB archive or this repository, and no root LICENSE has been added.

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

A data dictionary created through a different, independent metadata-inspection method must be labelled **preliminary** and must not be represented as a successful matio execution. No C++ solver is implied by the presence of this probe.

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
