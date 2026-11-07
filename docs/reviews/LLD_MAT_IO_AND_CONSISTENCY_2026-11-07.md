# SRS v0.4 / LLD 与 MAT I/O 一致性审查记录（2026-11-07）

> **检查性质**：本轮已在 GitHub 分支修改设计文档，并从 GitHub 连接器**只读**读取文档及源树，用 JavaScript 完成限定范围的 ID/依赖关系检查。**未在 Debian/Ubuntu 执行 Bash 追踪脚本、CMake、CTest、MATLAB/Octave、生产 Reader/Writer、MAT 写入/回读或数值测试**；因此本记录不是软件验收、SRS/HLD/LLD 冻结或 DEC-005/008 批准。

- 仓库：`ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`
- 目标分支：`dev-plan-P0-P1-mat-dictionary-probe`
- 文档修订前检查点：`2c879a62696b17af58f537232c1a75da59c88cd0`
- 本次独立 GitHub 内容静态复核检查点：`d18c630008e5da0e4899906080e7dd560fdde8ef`；后续只新增本记录/进度登记时，不能把新 HEAD 冒称为该复核已覆盖的精确 SHA。
- 项目长期政策：[无需 MATLAB 执行](../golden/NO_MATLAB_EXECUTION_POLICY.md)。L1 原 MATLAB 运行结果永久退出强制门禁；验收参考以 L2/L3/L4 为准。

## 1. 评审项与落实位置

| 优先级 | 审查问题 | 修订内容 | 仍需完成 |
|---|---|---|---|
| 高 | PLAN、TEST_PLAN 中 SRS 版本为 v0.3 | 两文件改为引用 **SRS v0.4 草案**；未将 HLD 独立文档版本 v0.3 误改为 v0.4 | 基线仍未冻结 |
| 高 | 历史相对容差 1e-10 与当前宽松候选冲突 | SRS §7、PLAN §7、TEST_PLAN §2.5 统一：**历史 `rtol=1e-10` 非现行候选**，现行未批准候选 `atol=0.01`、`rtol=0.1`，结构/几何/状态门槛独立 | DEC-005 批准、真实数值结果 |
| 高 | TEST_PLAN 顶部原 MATLAB Golden 采集表 | 明确现行按 L2/L3/L4 独立参考，永久不要求运行 MATLAB/Octave 或 L1；§2.6 改为 L2/L3 逐元素参考缺口 | L2 SHA-256/逐值提取、L3 独立数据 |
| 高/中 | 采样/实验/contracts/对象模型语义残留 | `sampling_noise.md §4` 改独立 L3 有序方向；`experiments.md §4` 改 L2/L3/L4；`contracts.md §5` 的 expected_intermediates 改 L3；`OBJECT_MODEL.md §8` 改 L2/L3 输入 | 独立固定输入、代码和 CTest |
| 低 | HLD 逐步算法 Golden 泛称 | `ARCHITECTURE.md §2.2` 明确“独立 L3 逐节点参考缺失” | HLD 性能/接口/构建证据 |
| 中 | 球谐旋转参考表标题 | `harmonics.md §7.3` 将“源 Golden”改为“独立 L3 参考” | R3/R5 多样本/许可/实现验证 |
| 高 | mat_io MatDenseArray 仅声明 | `mat_io.md v0.5` 增补**非复数 dense double 的自拥有候选值对象**：MatVariableInfo 元数据、二维 dims、只读数据访问、std::vector<double> 列主序、工厂校验、copy/move、零长度拒绝、乘法溢出 | P0.8/DEC-008 决定正式接口并实现/测试 |
| 高 | MAT 错误、写入参数边界不具体 | 以 contracts.md §3 的 Result/Status/MatErrorCode+ErrorDetail 为唯一候选来源；root 由 Composition Root 验证，RunContext owns RunId，Writer 在受控 output/RunId 中校验与发布且必须抗路径逃逸 | 资源、TOCTOU、路径安全真实负例及签名冻结 |
| 中 | readFirstCompatible 的多变量与后缀不明确 | 增加 §7.4：单变量读取；多变量无独立已批准顺序时 `AmbiguousVariable`、禁止按字母序猜测；无后缀候选优先级单独列示 | 独立 L3 多变量/后缀用例，DEC-008 |
| 中 | MAT I/O 测试实体与 CMake target 不具体 | 在 `mat_io.md §5/§8–9` 增补负例→TC→未来测试文件/CTest 与 `egomotion_matio` 的 domain/contracts/matio/zlib 依赖、公/私有头文件边界；TEST_PLAN §3 的 `TC-DC-IO-001` 补 `tests/contracts/test_mat_raii.cpp` **候选** | **测试文件及 CMake targets 尚未创建**，CTest 无运行记录 |

## 2. 可用证据和当前 MAT 限制

- 既有探针报告 **582/582 MAT v5+zlib、double/double、五种真实形状 READ_OK**；这是已有 MAT 元数据/读取事实，不等于 `MatDenseArray`、Reader/Writer、错误负例与数据按值验收。
- MAT 原始数据 L2 的**独立 SHA-256、原双精度逐元素参考/列主序**尚未生成。损坏副本、非法 class/rank/维度、空/多变量、RunId、TOCTOU 等负例属于 L3 可独立构造样例，但没有正式执行。
- `MatDenseArray` 是候选**二维实数 double-only**模型，不承诺支持未在归档出现的其他 MAT class；存量单变量兼容性不等于完整 MATLAB `fieldnames` 多变量行为。
- 仍保留原始 `origin/**` 只读、输出隔离 `output/<RunId>/`、不覆盖既有结果与人类批准的发布/运行权限。

## 3. 当前轮只读静态检查的实际范围

- SRS 规范性 REQ：**66**；TEST_PLAN TC：**66**；PLAN P：**98**；检查 REQ↔AC↔TC、P→REQ/TC/AC、未知前置 P 和 P 依赖环，**本次检查范围错误 0**。
- HLD §4.1 **26 条 target 边**的文档依赖关系无环（**不是**实际 CMake target 编译/链接通过）。
- 比较 `origin/` Git 子树哈希：更正前后均为 `70ee155131c24a50169346d0ffbaff0c01b1c547`，原始 **45 个 .m、582 个 .mat** 没有改动。
- 检查 `mat_io.md v0.5` 中 MatDenseArray/读写候选、§7.4 多变量、§8 负例、§9 target、TEST_PLAN 中 TC-DC-IO-001 的测试候选引用；**并未**执行 Shell 静态审查脚本或源代码数值测试。

## 4. 退出边界

**本轮仅解决设计文档的一致性、具体可评审性和候选测试对照**。依然**未具备**：冻结的公共接口及错误合同、生产 Reader/Writer、MAT L2 按值与 SHA-256、L3 负例、正式 CMake target/CTest、容差批准、第三方许可和软件验收证据。评审结论只能是“草案内容进一步明确，未冻结/未验收”，不能从候选签名推导程序可编译或通过。
