# Egomotion Estimation with Dense Flow Fields

A planned **C++ reimplementation and NumPy data conversion** of the MATLAB scripts accompanying the 2015 research paper *Insect-Inspired Self-Motion Estimation with Dense Flow Fields—An Adaptive Matched Filter Approach*.

> **Project status:** Repository preparation only. The MATLAB algorithms have **not** yet been ported to C++, and the original MATLAB data files have **not** yet been converted to NumPy `.npz` files. No equivalence or reproduction results are claimed.

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

**Relationship to this project:** The archived MATLAB artifact is the intended *reference implementation and input-data source*. The paper is the *scientific and mathematical reference*. This repository aims to build a clearly attributed, inspectable C++ implementation and to convert compatible MATLAB data into NumPy `.npz` archives, without changing the scientific meaning of the original experiments.

## Intended scope

- Port the MATLAB computational routines to C++ while documenting algorithmic correspondence to the original source.
- Convert MATLAB data files to NumPy `.npz` format, preserving array shapes, numeric precision, indexing conventions, and relevant metadata as far as practicable.
- Document provenance and transformations for any redistributed data.
- Eventually check numerical agreement and reproducibility against the original MATLAB implementation.

**Not yet implemented:** There is currently no C++ solver, data-conversion utility, published `.npz` dataset, or validated reproduction pipeline.

## Copyright, attribution, and licensing

The **research paper** is published under Creative Commons Attribution (CC BY 4.0). This is a statement about the paper; it must **not** be assumed to license the separately archived MATLAB code or data.

The [original MATLAB artifact](https://doi.org/10.4119/unibi/2736885) is credited to **Simon Strübbe, Wolfgang Stürzl, and Martin Egelhaaf (2015)**. Its Bielefeld University record lists **Open Data Commons Attribution License (ODC-By) v1.0** under **“Copyright und Lizenzen”**. See the [official summary](https://opendatacommons.org/licenses/by/summary/index.html), [full legal text](https://opendatacommons.org/licenses/by/1-0/), and the copy of that text in [LICENSE-ODC-BY-1.0](LICENSE-ODC-BY-1.0). This is **distinct from the paper's CC BY 4.0 license**.

**Important scope distinction:** ODC-By 1.0 licenses database rights and applicable rights in the database, but Section 2.3(a) explicitly excludes computer programs used to make or operate the database, while Section 2.4 cautions that individual database contents may have separate rights. Accordingly, the record's ODC-By label alone is **not proof that every MATLAB `.m` script is licensed as computer code under ODC-By 1.0**. Preserve any individual file notices and verify the original archive's contents before copying, porting, or redistributing the scripts. For a redistributed converted `.npz` dataset, retain the source citation, license URI, applicable notices, and a clear description of the transformation, subject to any separate content rights.

The root [LICENSE](LICENSE) remains **Creative Commons Attribution 4.0 International (CC BY 4.0)** for this repository's **independently authored documentation and other materials** that contributors have the right to license. It does not override the source artifact's ODC-By 1.0 terms, relicense third-party MATLAB files, or determine the license of future C++ code. The license for new C++ implementation files must be explicitly selected before they are added.

When referencing or using this work, please cite the original paper and the MATLAB artifact separately using the two DOIs above. Attribution to this repository is not a substitute for crediting the original authors.

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
