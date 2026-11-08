# 原始 MATLAB 来源清单（SOURCE_INVENTORY）

> **文件身份清单，静态快照**；原 `origin/**` 只读。此表来自 GitHub 源树的真实路径、blob SHA 与字节数，再与 [SRS §9.2](REQUIREMENTS.md) 的 REQ/TC 映射逐项比对。**不是代码移植、独立 SHA-256 的 MAT 验证、CTest 或正式来源审批。**

- 来源目录：`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`；固定输入仓库提交：`e6d4e7af6eaba109e189fb647429bc691be22ea9`；来源目录 tree SHA：`459f9172df05a4cbcc966e8f5a3da7e86b992be9`。
- **实际集合**：45 个 `.m`（按函数声明共 50 个：44 个单函数文件＋`SHRotate.m` 的主函数及 M/U/P/V/W 五个内部子函数）、582 个 `.mat`、1 个原始 `readme`（Git blob `e349e4ba44ec9d0e687ed130961ce5c8be87e33c`）。源脚本分组：`adaptive_matched_filter_test` 6；`bias_test` 3；`help_functions` 33；`illustrations` 3。
- **身份差别**：下表 Git blob SHA 是 Git 对象 ID，**不是**文件内容的独立 SHA-256，更不是原 matio 探针记录的 FNV64。582 MAT 的变量/dims/dtype/FNV/读取报告统一见 [MAT_DATA_SCHEMA.md](MAT_DATA_SCHEMA.md)，此处不重复 582 条。
- C++ 候选方法的**唯一职责分配**见 [MIGRATION_MATRIX.md](MIGRATION_MATRIX.md)；正式数学、实现接口/所有权与测试证据分别在 [对象模型](OBJECT_MODEL.md)、[LLD 索引](design/README.md)、[TEST_PLAN](TEST_PLAN.md)、[PROGRESS](PROGRESS.md)。

| 原源相对路径（相对来源目录） | Git blob SHA | 字节 | 主要 REQ | 计划 TC |
|---|---|---:|---|---|
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_many_parameter.m` | `67cb613328dec10105a6c383ce17025c4901e4c9` | 2228 | REQ-F-AN-001, REQ-F-CLI-004 | TC-F-AN-001 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_adaptive_matched_filter_test.m` | `3924038f0ccd70de5ea7b60c589afe166da9c8fb` | 2641 | REQ-F-AN-001, REQ-F-CLI-004 | TC-F-AN-001 |
| `adaptive_matched_filter_test/data_analysis/graph_agent_in_sphere_initialization_test.m` | `8816675ef5a4eb6767919df36ee11f95de36dac6` | 869 | REQ-F-AN-001, REQ-F-CLI-004 | TC-F-AN-001 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m` | `4629901c1233cdee5d6d22c917603720ffd1fab4` | 6156 | REQ-F-EXP-003, REQ-F-CLI-003 | TC-F-EXP-003 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_test.m` | `c9548c83fd0a2ce13d96394e98f6c7573119a7d5` | 5811 | REQ-F-EXP-002, REQ-F-CLI-003 | TC-F-EXP-002 |
| `adaptive_matched_filter_test/data_generation/agent_in_sphere_initialization_test.m` | `a3154ae391be82ccf48c9f3a03c55109dde85f65` | 4551 | REQ-F-EXP-001, REQ-F-CLI-003 | TC-F-EXP-001 |
| `bias_test/data_analysis/graphs_bias_test.m` | `112aa9eca1fa9728f493d31fc8e59620a8445230` | 2137 | REQ-F-BIAS-003, REQ-F-CLI-004 | TC-F-BIAS-003 |
| `bias_test/data_generation/bias_test_nonspherical.m` | `00a3e974fbf068ccd1af4e541a659862beffcad5` | 3345 | REQ-F-BIAS-001, REQ-F-CLI-002 | TC-F-BIAS-001 |
| `bias_test/data_generation/bias_test_spherical_prop_error.m` | `fc3c4f0a7a417171415fb5e3d869a744ba24d04f` | 3374 | REQ-F-BIAS-002, REQ-F-CLI-002 | TC-F-BIAS-002 |
| `help_functions/drehvektor_aus_zwei_vektoren.m` | `e41084d65cfc8605a8d377203e933a834c3098b3` | 529 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 |
| `help_functions/erzeugeflussausnear.m` | `df22f256390395aab08a887f5456546bd9111a11` | 1400 | REQ-F-FLOW-001 | TC-F-FLOW-001 |
| `help_functions/erzeugekreuzproduktmatrix.m` | `7ee35b9192fd54d6b6647da880c7e54d4df5e073` | 235 | REQ-F-GEOM-001 | TC-F-GEOM-001 |
| `help_functions/erzeugerichtungsvektoren_loch.m` | `13992600417e2ebd87193c06c1a7ec5740655735` | 1369 | REQ-F-GEOM-002 | TC-F-GEOM-002 |
| `help_functions/erzeugerichtungsvektoren.m` | `6ad7da8e3865cd2fd63ec641548ea38e0723b700` | 1116 | REQ-F-GEOM-002 | TC-F-GEOM-002 |
| `help_functions/findeDreiecke.m` | `8f6abf2cfb5daeca197314a19e382401d11df75f` | 836 | REQ-F-GEOM-002, REQ-F-GEOM-003 | TC-F-GEOM-002 |
| `help_functions/fixed_kugel_symmetric_perfectpoints_randomnear.m` | `8829ceca395bba3d371db690471ec2c1f086d11b` | 2656 | REQ-F-FLOW-002, REQ-F-FLOW-003 | TC-F-FLOW-002 |
| `help_functions/fixed_propfehler_kugel_symmetric_perfectpoints_randomnear.m` | `e3e01b4fdc0a68b6a14262c417b5dea69eb1e8c1` | 2664 | REQ-F-FLOW-002, REQ-F-FLOW-003 | TC-F-FLOW-002 |
| `help_functions/franzeigen.m` | `6fe556fd038b983f8d80aac5c76ee2ea11c08f38` | 1802 | REQ-F-FILTER-001 | TC-F-FILTER-001 |
| `help_functions/getflowfield.m` | `fafcfeaa357a55572058fa5c6e92d58f934c65a5` | 217 | REQ-F-MAT-003, REQ-I-MAT-001 | TC-F-MAT-003 |
| `help_functions/getnear_in_kugel_Drehung.m` | `ea807d35ae70f4a2cd2533160dc501ad3a154b8b` | 757 | REQ-F-FLOW-004 | TC-F-FLOW-004 |
| `help_functions/getvektor.m` | `c183a2808cf4f6c8bce86f83f93c46d8dec1ed9e` | 339 | REQ-DC-RNG-001 | TC-DC-RNG-001 |
| `help_functions/koeff_aus_near.m` | `5ff91a2210a20e5acf79dec6e59ddd33b55529d3` | 1342 | REQ-F-SH-001 | TC-F-SH-001 |
| `help_functions/kreis_trajektorie.m` | `6d4bd8a7d0b7cb516b1ab6e80061617b386977bd` | 529 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 |
| `help_functions/kreuzprodukt.m` | `559ab7723fa7adb48302931ca18667a2a5664f2e` | 257 | REQ-F-GEOM-001 | TC-F-GEOM-001 |
| `help_functions/kugelflfunc2.m` | `edc0caa1f88bf91d2a189e4d3a283334f809b905` | 611 | REQ-F-SH-001 | TC-F-SH-001 |
| `help_functions/kurveglaetten.m` | `eced66659205a6386ec84723e37a46517d06c017` | 331 | REQ-F-AN-001 | TC-F-AN-001 |
| `help_functions/mue_norm_koenderink_iteration.m` | `9ffdac1baaecac06db20bcc6849bf4cdd165a484` | 4006 | REQ-F-KVD-001 | TC-F-KVD-001 |
| `help_functions/near_aus_koeff.m` | `7373c4376dfd5bbf281136ced39657d411a10ffa` | 1023 | REQ-F-SH-001 | TC-F-SH-001 |
| `help_functions/nonpropfehler.m` | `c25d4743f5dc99b4825b738d822457d541813350` | 1025 | REQ-F-FLOW-003 | TC-F-FLOW-003 |
| `help_functions/norm_koenderink_iteration.m` | `d48048f37b859bfb19402ca85b0f4c20e76afbfd` | 4021 | REQ-F-KVD-001 | TC-F-KVD-001 |
| `help_functions/normrelfehler.m` | `c9fecc37180ab13d854a144d333a2599e601a9a8` | 348 | REQ-F-ERROR-001 | TC-F-ERROR-001 |
| `help_functions/oneit_adaptiv_matched_filter.m` | `48e51bcd52fb8f27cf35b3fce38b28507af36e55` | 2903 | REQ-F-ADAPT-001 | TC-F-ADAPT-001 |
| `help_functions/richtungsvektoren_icosahedron.m` | `c785db126cef2149a6ffc177ecadb21e457699ca` | 1136 | REQ-F-GEOM-003 | TC-F-GEOM-003 |
| `help_functions/rodrigues.m` | `748f2bf171b01ad77fd949cb518d0df4344a39d5` | 4717 | REQ-F-TRAJ-001, REQ-F-SH-002 | TC-F-TRAJ-001 |
| `help_functions/SHRotate.m` | `7b73000411a9294f6cbed2e111bf74ce7fb7d4df` | 3112 | REQ-F-SH-002 | TC-F-SH-002 |
| `help_functions/sinuskurve_trajektorie.m` | `e2857131361619593f13f1bcd2cf2207f1a90493` | 703 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 |
| `help_functions/skalarprodukt.m` | `31602dc0e2c96bfe85639b22efd294fe31ad4033` | 179 | REQ-F-GEOM-001 | TC-F-GEOM-001 |
| `help_functions/trajektorie_aus_punkten.m` | `379d0695e96c443d1c8f33ba4d0e77334b87b135` | 1111 | REQ-F-TRAJ-001 | TC-F-TRAJ-001 |
| `help_functions/transfehler_div.m` | `89708055b98e0b6651990c08dc3850393ca8acad` | 558 | REQ-F-ERROR-001 | TC-F-ERROR-001 |
| `help_functions/transfor_koeff.m` | `7f26019cab53666539e1961dac67a2e07dae614c` | 666 | REQ-F-SH-002 | TC-F-SH-002 |
| `help_functions/winkelausvektor.m` | `5bb7ba5832ae849c791df6c67bd49d53b60f250e` | 237 | REQ-F-SH-001 | TC-F-SH-001 |
| `help_functions/zellendefinieren.m` | `7132613d27135cfa00664429251da6b95553bb7c` | 393 | REQ-F-FILTER-001 | TC-F-FILTER-001 |
| `illustrations/circle_curve_in_the_sphere.m` | `c6bdbff07cdfff796c105f299fb728b15783ac7c` | 847 | REQ-F-AN-002 | TC-F-AN-002 |
| `illustrations/sinosodial_curve_in_the_sphere.m` | `06022612e9249c532f4dabae5658efa9ece4c10d` | 860 | REQ-F-AN-002 | TC-F-AN-002 |
| `illustrations/spherical_harmonic_illustration.m` | `d7f97e0fe15c761f33f6ebd152610ec4144561e9` | 921 | REQ-F-AN-002 | TC-F-AN-002 |

**逐文件集合检查（仅文档/源树静态）**：本表 45 条唯一相对路径与 Git 树、SRS §9.2、OBJECT_MODEL §7 的三份列表各自 **45/45** 一一对应，无漏项/额外项；这不检查原 `.m` 内全部嵌套函数、执行顺序、旧 MAT 数值或 C++ 产物。项目根 MIT 不自动为原作者源文件或 MAT 重新授权，实际复用文件须保留已有必要来源/notice。


**函数声明核对（AF-072）**：在固定源码 Git 树中逐份阅读 45 个 `.m` 的 `function` 行，实际是 **50 个定义**；逐函数职责和 `SHRotate` 的内部五个子函数见 [MIGRATION_MATRIX 的函数声明级补充](MIGRATION_MATRIX.md)。该检查不包含分支覆盖/可编译 C++ 或 L3 测试。
