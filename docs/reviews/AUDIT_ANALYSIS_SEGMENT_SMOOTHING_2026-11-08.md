# Analysis rotation/translation smoothing scope audit (2026-11-08)

**Audit input**: `02d59d7a118870073bdad3b043b1fd01df7f9628` (GitHub read-only source inspection). This is static source-to-LLD review, not MATLAB execution, independent L3, Analyzer implementation or CTest.

## Evidence and findings

| Source | Original execution contract | Required document distinction |
|---|---|---|
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` lines 35–48 | **Rotation panel**: for each `z=1:4`, take `allemitrotfehler(begin(z):ende(z), k)`; plot col 1 unfiltered, smooth col 2/3/4 **within that segment** using half-windows 2/5/10, and plot only `begin(z)+f:ende(z)-f` | AF-075. Per-segment smoothing must *not* be replaced with global smoothing followed by the same crop; samples across segment boundaries would enter the result otherwise |
| Same source lines 68–80 | **Translation panel**: col 1 full curve; compute `kurveglaetten(allemittransfehler(:,2/3/4), f)` over all 600 rows, then plot `x=50:550`. No `begin/end` segmentation on translation panel | AF-076. Full-600 smoothing and x slicing have a different support/edge rule from the rotation panel |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` lines 20–60, 79–88 | Comparison rotation panel uses ratio threshold and four begin/end segment loops; **translation panel** plots `alleohnetransfehler(x,1)` and `allemittransfehler(x,1)` for x=50:550 with no segment mask | AF-076. Both comparisons need separate rotation/translation trace behavior |
| `help_functions/kurveglaetten.m` lines 1–12 | `zeros(N,1)`; interior `i=f+1:N-f` mean over `i-f:i+f`; each invocation restarts edge zeros | AF-075/076. Applying the same helper to a segment versus the full 600 rows produces different outputs |

## To verify later without inventing PASS

With a fixed synthetic ratio containing four valid intervals and a nonconstant sequence near a segment boundary, independent L3 should compare (a) per-segment rotation window values and trimmed displayed x; (b) full-length translation window values at 50:550; (c) behavior for too-short segments and missing `rottransratio`. Map to existing `TC-F-AN-001`, `TC-I-ANALYSIS-001` and APL-G04/05; no new TC ID. The boundary-dependent values must differ for a correctly designed discriminating fixture; testing only a constant curve cannot detect this bug.

Existing AF-050 and AF-051 require actual L2/L3 curves, Analyzer/CTest and approval. This report does not satisfy them. No `origin/**` source or MAT files were changed.
