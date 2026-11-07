# MAT I/O、路径安全与原始数据合同——详细设计草案

> **LLD v0.5 候选草案，未冻结。独立 matio 只读探针已实现并在 Ubuntu 24.04 上完成 582 文件全量变量读取；正式业务 Reader/Writer、负例、写回读及 L2 逐值/L3 独立参考验证仍未完成。** 依据 getflowfield.m、MATLAB 的 load/save 调用、原始归档样本文件头和 AGENTS.md。需求：REQ-F-MAT-001/002/003/004、REQ-D-MAT-001/002/003、REQ-I-MAT-001、REQ-DC-IO-001、REQ-NF-SAFETY-001；任务 P0.5–P0.6、P1.3–P1.10。

## 1. 原始行为、文件身份和支持版本

原 getflowfield(filename) 使用 MATLAB load(filename)，列出 fieldnames，并读取第一个字段。MATLAB 原分析入口经常省略 .mat 后缀，故必须区分：

- **严格读取**：显式 MAT 路径和变量名，变量缺失即失败。
- **旧脚本兼容读取**：按原脚本的路径/后缀解析规则取得数据，多变量场景须依据静态格式说明和独立 L3 例子核验枚举/冲突规则；若顺序无法确定则明确返回 AmbiguousVariable，**不要求运行 MATLAB 再冻结现有单变量归档合同**。不能随意以名称字典排序代替原行为。
- **inspect**：列出全部变量元数据而不读取成算法领域类型或更改文件。

**现有归档的格式已经实测**：`src/mat_data_dict_probe.cpp` 在 Ubuntu 24.04/matio 1.5.26 以 `Mat_Open(..., MAT_ACC_RDONLY)` 和 `Mat_VarRead` 读取全部 582 MAT/582 变量，失败 0；每个源 MAT 是 v5/zlib 压缩、单变量 class=double/data_type=double，flags 均为 `-`。详见 [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md)。当前数据不需要 HDF5；未来若扩展到 v7.3 应单独审查。

### 1.1 已确认的数据类型支持边界（基于 582/582 实测）

| MATLAB class / MAT dtype | 实测变量数 | 业务读取决定 |
|---|---:|---|
| double / double（实数、压缩 MAT v5） | **582** | **当前必需**：保留变量名、rank/dims、列主序和 double 数值，实测探针已读；生产接口与数值 copy/Map 仍待验收 |
| single/int8…uint64/logical | 0 | **不是本归档必需输入**；遇到新输入须显式类型化读取或报 UnsupportedClassOrType，不允许偷偷转 double |
| cell/struct/object/char/sparse/complex | 0 | **不是本归档必需输入**；未实现的类型必须拒绝，不能伪称兼容 |
| MAT v7.3 / HDF5 | 0 | **当前不需要引入**；只在将来出现真实 v7.3 输入时走变更审批 |

目前已观察到的五种变量形状仅为 **10×1（80）、10×3（480）、35×1（4）、600×1（14）、600×4（4）**。每个文件恰有一个与 `.mat` basename 相同的变量；数据物理字节数均为元素数×8。此结论只覆盖当前归档，不是对未来任意 MAT 类型支持的承诺。

## 2. 候选 C++17 类接口与资源所有权

~~~cpp
// 候选 C++17 接口；未建头文件、未编译。
// namespace 及最终 include 路径均待 P0.8/DEC-008 确认。
struct MatVariableInfo {
    std::string name;
    std::vector<std::size_t> dimensions; // 与 MAT 原 rank/dims 顺序一致
    int original_class_type;             // 诊断用 matio 数值，非对外 matio 指针
    int original_data_type;              // 最终稳定枚举待接口审批
    bool is_complex = false;
    bool is_logical = false;
};

// 仅针对当前归档必要的「二维、实数、密集 double」对象。
// 不对其他 MATLAB class 暗中做类型转换。
class MatDenseArray {
public:
    static Result<MatDenseArray, MatErrorCode> fromColumnMajor(
        MatVariableInfo info,
        std::vector<double> column_major_values);

    const MatVariableInfo& info() const noexcept;
    const std::vector<std::size_t>& dims() const noexcept;
    const double* data() const noexcept;
    std::size_t size() const noexcept; // 元素数
    const std::vector<double>& valuesColumnMajor() const noexcept;

    MatDenseArray(const MatDenseArray&) = default;
    MatDenseArray& operator=(const MatDenseArray&) = default;
    MatDenseArray(MatDenseArray&&) noexcept = default;
    MatDenseArray& operator=(MatDenseArray&&) noexcept = default;

private:
    MatDenseArray() = default;
    MatVariableInfo info_;
    std::vector<double> values_col_major_;
};

class MatFileHandle;     // 内部 move-only；唯一持有 mat_t*
class MatVariableHandle; // 内部 move-only；唯一持有 matvar_t*

class MatResultReader {
public:
    Result<std::vector<MatVariableInfo>, MatErrorCode> inspect(
        const std::filesystem::path& source) const;
    Result<MatDenseArray, MatErrorCode> readNamed(
        const std::filesystem::path& source, std::string_view variable) const;
    Result<MatDenseArray, MatErrorCode> readFirstCompatible(
        const std::filesystem::path& source) const;
};

class RunMatWriter {
public:
    // 内部由 Composition Root 注入受控的仓库根 output/，非 CLI 参数。
    explicit RunMatWriter(std::filesystem::path validated_repository_output);
    Status<MatErrorCode> saveNewVariable(
        std::string_view run_id,
        std::string_view mat_variable_name,
        const MatDenseArray& value) const;
private:
    std::filesystem::path output_root_; // 构造/每次写出时仍应校验目录身份
};
~~~

C++17 的无值操作采用 contracts.md 中的 Status<E> 候选，具体实现须经项目审定；**这些是拟议的签名和枚举，不是已经冻结的 API**。对 matio 的资源操作建议使用 Mat_Open/Mat_Close 与 Mat_VarRead/Mat_VarFree 等 C API，但调用细节须根据项目选定的 matio 版本核准。

### 2.1 内存所有权、shape 与类型合同

- **唯一所有权**：MatFileHandle、MatVariableHandle move-only，析构释放一次；返回 MatDenseArray 后没有指向这些句柄/已释放 matvar_t 的借用视图。
- **自拥有与访问器**：MatDenseArray 内部为 `std::vector<double>`，`data()` 是只读临时指针、`valuesColumnMajor()` 返回只读引用，`dims()` 返回 `info_.dimensions` 的只读引用。对象存活且未移动/修改时才可借用上述引用；不得存成跨对象生命周期的 Eigen::Map。需要 Eigen 时可在调用作用域内创建只读 Map 或复制为自拥有值对象。
- **当前必要类型**：`fromColumnMajor` 仅接受非复杂、非 logical、实数密集 double 的 MAT class/type；`dimensions.size()==2` 且两维均大于 0。零长度数组在 `inspect` 中允许**如实报告元数据**，但正式业务数据读取/输出构造当前按 `InvalidShape` 拒绝；其他 rank/type 属未来变更范围，不暗中转成 double。
- **形状/长度不变量**：在分配前以检查溢出的乘法计算 `elements=dims[0]*dims[1]` 与 `bytes=elements*sizeof(double)`，要求数据向量长度等于 elements；溢出返回 `SizeOverflow`，维度或长度不符返回 `InvalidShape`，class/type/complex/logical 不支持返回 `UnsupportedClassOrType`。列主序线性元素偏移为 `row + rows*column`，0-based 转换要显式核对。
- **复制移动**：数组**可深拷贝**（独立 vector/元数据），也可移动；移动后原对象不得作为具有原数据的有效对象使用。当前不采用以 `std::vector<uint8_t>` 承载任意标量类型的泛型变体，避免将单精度、整数或 complex 假装受支持。
- **元数据边界**：MatVariableInfo 的两个 int 暂是 matio 编码的数值快照，仅做诊断/验证；公共头不引用 matio.h、不提供 mat_t*/matvar_t*。将来稳定 enum/字段及目标 C++ 命名空间是待 P0.8 审核项；本候选不是已冻结、可编译的头文件。

### 2.2 失败结果、错误上下文与 RunContext 边界（候选）

- `Result<T, MatErrorCode>` 成功分支只包含完整、符合不变量的 T；失败分支应同时含 `MatErrorCode` 和 [contracts.md §3](contracts.md) 的 `ErrorDetail`（含 `stage/source_path/variable`，在已分配 RunId 时附加 `run_id`）。采用 `Failure<MatErrorCode>{code, detail}` 还是其他实现方式由公共合同统一冻结，**不得引入第二套 MAT 错误枚举或全局 last_error**。
- `Status<MatErrorCode>` 表示无值的成功/失败；失败同样须传播 `ErrorDetail`，而不是用 `true/false` 丢失失败位置。其实现与 `Result` 共同归 [contracts.md §3](contracts.md) 管理。
- `MatDenseArray::fromColumnMajor` 是建议的唯一对外构造入口。Reader 完整解码后在 MAT I/O 实现边界按验证后的 double/列主序复制成自拥有值对象；**实验 Runner 不直接构造或依赖 MatDenseArray**。Composition Root 注入具体 IResultWriter，MAT I/O 适配层负责将 contracts/domain 值对象按二维 shape 变换为合格 MatDenseArray 后交给 Writer；不得让 experiments/analysis 反向依赖 io 具体类。
- **固定输出根与身份来源**：CLI 没有输出根参数。Composition Root 从已确认的仓库根位置取得 `output/`，并将该**内部可信候选路径**注入 RunMatWriter；RunContext 使用同一根、拥有或生成经验证的 RunId（自动 ID 通过独立 OS 随机源创建，不消耗实验 RNG），在 `output/<RunId>/` 独占新建目录。Writer 每次写出仍需重新检查 root 的设备/目录身份、不覆盖/不逃逸，目标为 `output_root_ / run_id / (mat_variable_name + ".mat")`；不接受用户提供的另一个输出根。
- Writer 在文件系统层必须**再次约束** root、RunId、最终文件名、独占创建和已有目标拒绝，不能仅依赖 CLI 的一次性检查；同一新 RunId 可安全写入多个不同变量，但不能覆盖已有 MAT 文件。符号链接/目录竞态的可靠实现与测试仍是 P1.9 门禁。

## 3. 输入路径与输出路径的强制安全流程

**输入阶段**：
1. 验证输入路径存在、可读、常规文件，记录可审查的规范化路径与 SHA；
2. 以只读方式打开 matio 句柄，读取头/变量元数据并校验;
3. 对压缩块做出明确的解码结果；不能把“仅能读取头”当作“全部变量读取成功”；
4. 返回自拥有的领域数据或具体错误码；输入文件及 origin 目录保持不变。

**输出阶段（拟议保护合同）**：
1. **H-12 已确定仓库根 `output/` 是唯一新数据目录**，用户最新决定删除 CLI 的输出根选项。Composition Root/RunContext/Writer 共享同一受验证的内部输出根，RunId 可由用户提供或 OS 随机源生成，新建 `output/<RunId>/` 时必须独占且拒绝目录/文件复用。Writer 不自行更改隔离根、不使用用户任意路径。任何 `origin/**`、路径逃逸、符号链接覆盖都拒绝；真实 TOCTOU 实现仍待授权后测试。
2. 对路径绝对化并解析已存在父目录及符号链接；拒绝 canonical 祖先关系下覆盖原始输入。需要考虑 ../、符号链接、同一路径多种表示、UTF-8/空格及软链接中途改变问题；
3. 对最终文件使用**不覆盖**的独占创建/安全发布策略；不能仅靠“先 exists 再 open”保证无竞态。实际 matio 写出/临时文件/原子发布的组合在 Debian/Ubuntu 上需验证；
4. 写后独立回读比对变量名、rank/dims、类型、数值，记录结果 SHA；失败时保留日志与不完整输出状态，但不得覆盖任何已有结果；
5. 输出目录和原始目录隔离为设计及测试合同，具体软链接与竞态防护实现需要 P1.9 通过后才可声称安全。

### 3.1 输入/输出错误与回滚

- 输入损坏：报告文件、变量、错误类别；不能通过跳过错误字段伪造成功。
- 输出已存在或路径不合法：明确拒绝，默认不覆盖。
- 写入过程中失败：报告不完整的产物和 RunId；该行为不等同于已完成的原子事务。是否清除临时文件须遵守 AGENTS.md 禁止擅自删除/清理用户数据的要求。
- 内存释放由 RAII 析构负责；不得在异常状态产生悬垂指针。

**H-12 执行边界**：根目录 `.gitignore` 已新增 `/output/` 忽略规则；只计划在该目录生成新的结果和隔离负例副本，**不以全局 `*.mat` 排除原始资料**。即使 H-12 的路径方案已被用户确认，该 JSON 同时包含 H-10=deferred 和 `permissions.writeMat=false`，因此**本轮无权创建生产 Writer、运行写回读或模拟损坏的 MAT 负例**。既有 matio 探针只读记录仍独立有效。

## 4. MAT 数据字典与 L2/L3 参考数据来源

**正式 matio 全量字典已存在**：[MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md)，逐文件列出 FNV64、版本、变量、class/type、dims、flags 和读取结果。以下表格区分已记录证据与仍需开发的业务 Reader/数值验收：

| 字段 | 预期内容 | 状态 |
|---|---|---|
| 相对路径、文件字节数与 FNV64（报告） | 已由探针生成 | **582/582 已记录；FNV 不是 Git SHA/密码学 SHA-256** |
| MAT 版本、压缩、只读加载 | Mat_GetVersion、Mat_VarRead | **582/582 MAT v5+zlib，READ_OK** |
| 变量名、MAT class、物理 data_type、rank/dims | Mat_VarRead 后读取 matvar_t 元数据 | **582/582 double/double，五种 shape** |
| 数值范围、NaN/Inf、逐元素 L2 MAT 原值/独立 SHA-256 | 生产 Reader 按值遍历及独立对照 | **尚未验证**；探针成功读取不能替代数值等价 |
| 严格/兼容读取策略、分析依赖 | 源 load/getflowfield 和数据字典映射 | 待冻结 |
| 解码状态与错误码 | 成功/失败/不支持/只读权限拒绝 | 待验证；一般版权审查不是 MAT 读取/开发阻塞项 |

输出合同：Bias 每目录 xwerte、ywerte1…6；Adaptive 使用初始化四类结果、600 点比较五类结果和更新频率三类当前源码结果。**历史 MAT 的额外文件属于归档证据，不得视为当前脚本已经写出。**

### 4.1 已确认的历史结果变量和真实尺寸

| 归档组 | 实测字段 | 类型与 shape |
|---|---|---|
| occluded_regions（40 次） | 每次 xwerte + ywerte1…6 | xwerte `double[10×1]`（40 个），六个 ywerte 各 `double[10×3]`（240 个） |
| proportional_error（40 次） | 同上 | xwerte `double[10×1]`（40 个），六个 ywerte 各 `double[10×3]`（240 个） |
| initialization | allemit/ohne × trans/rot 四个变量 | 均 `double[35×1]` |
| without_errors | 四误差字段与 rottransratio | 均 `double[600×1]` |
| with_errors | 上述五字段 + alleadaprotfehler / alleadaptransfehler | 均 `double[600×1]` |
| many_parameter | allemittransfehler、allemitrotfehler、alleadaptransfehler、alleadaprotfehler | 均 `double[600×4]` |
| many_parameter | akrot、rottransratio | 均 `double[600×1]` |

历史归档中额外存在的 `alleadap*` / `rottransratio` 的**来源尚未归因**，不得依据它们擅自改变当前 MATLAB 生成入口的 `save` 合同。

### 4.2 L2/L3 参考来源与独立性（非原 MATLAB 运行 Golden）

| 来源等级 | 可用数据与独立参考 | 当前未完成项 |
|---|---|---|
| **L2：原作者历史 MAT** | 对 582 个原始文件的**只读**逐值提取（保留原变量名、class/type、rank/dims、压缩与列主序）及各源文件的独立 SHA-256；业务 Reader 必须与独立数据读取/字节检查逐项对应 | **现有 matio 探针已报 582/582 READ_OK**，但尚未产生逐元素 L2 参考数据与独立 SHA-256；P10.4 仍待复核 |
| **L3：独立构造的合同参考/负例** | 单变量与多变量 MAT、缺变量、类型/shape 错误、元素数溢出、只读/写保护、列主序定值、异常路径；由非被测实现或可手工审核的规则给出预期状态和值 | 负例文件/预期日志及 Reader/Writer 测试尚未建立；不调用或模拟原 MATLAB 运行结果 |
| **L4：分析统计** | 对 L2 结果由独立分析逻辑计算的汇总 | 该层属于分析/实验模块，不是 MAT 解码成功的必要条件 |

**比对口径**：MAT 文件身份/变量名/class/dtype/rank/dims/线性索引与**无转换的原始 double 数值读取**要求精确；如涉及 NaN/Inf，应保留其分类并单独说明比特级规范，不允许靠宽松容差掩盖数据转换。外层分析计算可另按 [容差提案](../golden/GOLDEN_TOLERANCE_PROPOSAL.md) 采用 `atol=0.01`、`rtol=0.1` 的**未批准候选**，但**MAT 解码/复制不使用此容差容忍丢值或转置**。全程不要求 MATLAB/Octave 或 L1 Golden。

## 5. 接口、TC、候选测试实体与 AC（不表示测试已创建）

完整的 TC→拟议文件/CTest→AC 矩阵以 [TEST_PLAN.md §3](../TEST_PLAN.md) 为单一来源，本表只列本模块的对应责任。**文件名、CTest 前缀、测试函数均为候选；当前没有这些业务测试代码、CTest 注册或 PASS 记录。**

| TC / AC | 计划测试实体 | CTest 候选名/审核 | 检查内容与证据来源 |
|---|---|---|---|
| TC-F-MAT-001～004 / 对应 AC | `tests/golden/test_mat_archive.cpp` | `egomotion.mat.*` | L2 全量/代表变量值、strict/compat、写后回读、原文件只读 |
| TC-D-MAT-001～003 / 对应 AC | `tests/golden/test_mat_archive.cpp` | `egomotion.mat.*` | 582 文件/变量合同、class/dtype/rank/dims、SHA-256、历史字段差异 |
| TC-I-MAT-001 / AC-I-MAT-001 | `tests/golden/test_mat_archive.cpp` | `egomotion.mat.*` | 数据读/变量选择/错误传播、列主序数据合同 |
| TC-DC-IO-001 / AC-DC-IO-001 | **新增候选** `tests/contracts/test_mat_raii.cpp` + `docs/OBJECT_MODEL.md` | **候选** `egomotion.mat.raii.*` + 人工评审 | move-only 句柄析构一次、自拥有 MatDenseArray、返回后资源释放、错误上下文不丢失 |
| TC-NF-SAFETY-001 / AC-NF-SAFETY-001 | `tests/contracts/test_output_safety.cpp` | `egomotion.safety.*` | 原始数据保护、已有 RunId、符号链接/路径逃逸、失败输出证据 |
| TC-I-RESULT-001 / AC-I-RESULT-001 | `tests/integration/test_adaptive_experiments.cpp` | `egomotion.experiments.*` | RunContext/Writer 生成结果的变量名/类型/shape/非覆盖、独立回读 |

`test_mat_raii.cpp` 是面向原 `TC-DC-IO-001` 的**候选补充测试实体**，不会另建 TC/AC ID；是否最终采用该文件/CTest 名仍由 P0.8 和 DEC-008 确认。所有异常输入应使用**隔离的测试副本或内存合成数组**，不得破坏 `origin/**`。


## 6. 待决与冻结门槛

实测 matio 版本 **1.5.26**、归档 class/dtype/rank/dims 已确认。**仍待**：完整业务 Reader 的数据复制/映射、CMake target、异常与 CLI 合同、Result<T,E>、文件名/首变量兼容规则、**L2 原 MAT 逐值参考/L3 独立合同参考**的核验、输出写入与路径安全、DEC-005 容差和 P10.4 独立复核。**不把候选接口当作已经建成的 C++ 程序。**


## 7. API 的前置/后置条件与错误映射（编码候选）

公共类型与错误枚举的单一维护位置为 [contracts.md](contracts.md)。第 2 节的示例签名是拟议 API；本节补充**可用来审查方法的完整行为合同**，并不表示实现已创建。

| 方法 | 前置条件 | 后置条件 | 非成功分类 | 关联 TC |
|---|---|---|---|---|
| inspect(path) | 输入为存在的普通文件，且 Reader 不持有写权限 | 按 MAT 文件真实变量记录返回 name/class/dtype/rank/dims/complex，绝不变更输入 | MissingFile、InvalidMatHeader、CorruptCompressedData | TC-F-MAT-002 |
| readNamed(path,name) | name 非空；所需 class 已受支持 | 对**唯一指定变量**读取完整自拥有数据，源文件句柄销毁后结果仍有效 | MissingVariable、UnsupportedClassOrType、InvalidShape | TC-F-MAT-001/003 |
| readFirstCompatible(path) | 路径合法；可明确补齐省略的 `.mat`；不要求 MATLAB 运行 | **单变量文件**读取该变量；**多变量**在没有已获核准的独立顺序依据时拒绝 `AmbiguousVariable`，不擅自按字母序选取；仅承诺现有 582 份单变量归档的兼容范围 | AmbiguousVariable、MissingVariable、UnsupportedClassOrType、InvalidShape | TC-F-MAT-003 |
| saveNewVariable(RunId,name,data) | **构造时根路径已固定为仓库 `output/`，且写入时再次检验根/RunId/文件名**；不存在输出根 CLI 参数。RunId、变量名合法且文件不存在 | 在 `output/<RunId>/` 下独占发布新 MAT，写后回读 dtype/rank/dims/数据并保存 manifest | OutputPathForbidden、OutputExists、WriteFailed、ReadbackMismatch | TC-F-MAT-004 |
| 句柄析构/移动 | 只允许单一所有者持有 mat_t*/matvar_t* | 释放恰好一次；move 后源句柄变为无所有权状态；MatDenseArray 深拷贝/移动后的存活性另测 | 不通过异常伪装成功；记录资源失败 | TC-DC-IO-001 |

**完整错误枚举以 [contracts.md §3](contracts.md) 为唯一候选规范**；上表只是主要示例，不能视为完整 enum 列表。此模块涉及的 `MatErrorCode` 包括：`MissingFile`、`PermissionDenied`、`InvalidMatHeader`、`CorruptCompressedData`、`MissingVariable`、`AmbiguousVariable`、`UnsupportedClassOrType`、`InvalidShape`、`SizeOverflow`、`OutputPathForbidden`、`OutputExists`、`WriteFailed`、`ReadbackMismatch`。从 `Result/Status` 的失败分支传播原 `MatErrorCode` 与 `ErrorDetail`，CLI 退出码继续只引用 `contracts.md §3.1`，不在此处复制映射。无新增错误码得到批准。

### 7.1 具体 matio 调用边界

~~~text
Reader::inspect:
    f = Mat_Open(path, MAT_ACC_RDONLY)
    iterate Mat_VarReadNextInfo(f) until EOF
    copy metadata to stable value objects
    Mat_VarFree(info); Mat_Close(f)
Reader::readNamed:
    f = Mat_Open(path, MAT_ACC_RDONLY)
    v = Mat_VarRead(f, variable_name)
    validate class_type, data_type, isComplex, rank, dims, data size
    copy numeric buffer with correct column-major order
    Mat_VarFree(v); Mat_Close(f)
Reader::readFirstCompatible:
    resolve filename under candidate extension rule (see §7.4)
    inspect variable count and order from metadata
    0 variables -> MissingVariable
    1 variable -> readNamed using the one and only true variable name
    >1 variables without approved independent ordering proof -> AmbiguousVariable
    never choose first by alphabetic name sorting
Writer::saveNewVariable(RunId,name,data):
    validate fixed internal output root and isolated RunId directory
    build matvar_t via Mat_VarCreate (type/shape checked)
    create temporary MAT path through Mat_CreateVer
    Mat_VarWrite; Mat_Close; verify write result
    publish to previously absent final filename without overwriting
    reopen through Reader and compare data + record SHA
~~~

以上为 API 调用责任示意，具体 matio 函数的错误返回、压缩 flag、版本宏及 read-next-info 的所有权需对**选定的真实 matio 版本**核验，不能按未经核实的头文件约定实现。第一个变量的确定顺序也不能仅假设为字母序。

### 7.2 只读路径与写出路径的 TOCTOU 处理

1. 先记录 origin 根目录的 canonical 路径、设备/目录身份，拒绝在其中写出；并将所有期望输出路径限定在独立的新 RunId 目录内。
2. 检查路径组件是否包含符号链接、父目录逃逸、绝对路径形式的 RunId 或异常文件名。Linux 上只用 lexical_normal/canonical **并不能防御并发替换符号链接**。
3. 若安全目标要求抵御并发目录替换，实施者必须评估并使用 dirfd、openat/O_NOFOLLOW、O_EXCL 或等价的 Linux 文件描述符约束；matio 创建临时文件的路径式 API 与最终不可覆盖发布（例如受控制的原子 link/rename-no-replace）须组合审查。**在此验证之前不得宣称写出流程抗竞态。**
4. 写入临时文件后校验新 MAT 再发布到最终名称。发生部分写入、崩溃或验证失败时记录临时文件身份；按 AGENTS.md 保护失败证据，不清理未知用户文件。
5. 对用户传入的中文/空格路径使用 C++17 std::filesystem 在 Debian/Ubuntu 上进行实际测试；对输入编码、非法路径、只读用户身份及不可信软链接分别建负例。

### 7.3 形状与 MAT 类型扩展策略

| 验证点 | 可审查的计算 | 当前证据 |
|---|---|---|
| rank/dims | 维度为非负整数，逐项检查，使用乘法溢出保护计算元素总数；N×6 不等于 6×N | 待 P1 测试 |
| 非复杂 double | MAT dtype=double；元素数×8 与 data_bytes 匹配，需保持列主序 | **582/582 实测归档；生产 Eigen 映射尚未测** |
| single/int/logical | 不允许默认 reinterpret_cast<double*>；只能显式类型化读取或拒绝 | **归档出现次数 0；拒绝负例待测** |
| complex/struct/cell/sparse | 不与普通密集实数同接口误读，必须有类型校验和独立失败语义 | **归档出现次数 0；扩展无需预先实现** |
| MAT 压缩 | 582 个压缩变量实际完成 Mat_VarRead 解码 | **582/582 zlib READ_OK** |
| 读后生存 | 关闭 MatFileHandle、MatVariableHandle 后，返回的 MatDenseArray 仍能正确访问 | 合同测试待写 |

MAT 类支持范围的**最终**记录应由 docs/MAT_DATA_SCHEMA.md 单一维护；本表只描述决策和检查方法。

### 7.4 `readFirstCompatible` 的单/多变量与后缀规则（有界兼容，候选）

现有原作者 582 个 MAT **每个恰好一个变量**，能够在不执行 MATLAB 的前提下直接使用 L2 数据验证这一**有界兼容模式**。原 `getflowfield.m` 对多变量 `fieldnames(load(...))` 的完整运行顺序目前没有独立证据，不能凭本归档的单变量成功推断一般多变量顺序。

| 输入类别 | 候选处理 | 证据/错误及覆盖范围 |
|---|---|---|
| 显式文件 `sample.mat`，内部正好 1 个变量 | 保留实际变量名并读取这个变量，调用 `readNamed` 校验 class/type/rank/dims | L2 原作者 582 单变量 MAT；`TC-F-MAT-003` |
| 无 `.mat` 扩展名 `sample` | 在候选解析中先识别明确存在的原路径；若无此文件且 `sample.mat` 存在，**仅补全一次 `.mat`**；两者均不存在返回 MissingFile | 解析优先级/路径等价写法仍受 `cli.md` 与 DEC-008 审批，不宣称与所有 MATLAB load 搜索路径相同 |
| 空 MAT 或 0 个可枚举变量 | 返回 MissingVariable；`inspect` 可如实列出空元数据 | L3 构造的独立负例；`TC-F-MAT-003` |
| 同一 MAT 有 2 个及以上变量，**无经独立核验的 fieldnames 顺序** | 返回 `AmbiguousVariable`，**绝不擅自选字母序最小变量**，也不默认返回第一个磁盘存储变量 | 目前归档没有这样的正例；独立 L3 多变量负例与明确非成功状态 |
| 多变量且**未来有独立证据证明特定顺序** | 仅在 DEC-008 批准顺序来源、例子与适用 MAT 格式后，才按获准顺序选择首变量；否则保持 `AmbiguousVariable` | 未来候选，不得称当前已实现或完全复现源 MATLAB 的 fieldnames |
| strict `readNamed(path,name)` | 始终按指定的 `name` 精确查找；不能因首变量存在而忽略缺失的指定变量，也不自动采用 compat 的多变量推断 | `TC-F-MAT-001/003`；MissingVariable / UnsupportedClassOrType / InvalidShape |

这些规则旨在保证**单变量归档可复现、未知多变量行为不伪造成功**。若未来需要完整多变量 `fieldnames` 语义，应先补齐 L3 独立例证及 CLI/错误合同，再评审扩展；**不能恢复 MATLAB/Octave 运行作为必需条件**。


## 8. 可复现正/负例与计划测试实体（均未建立/未执行）

| 用例类别及 L2/L3 来源 | 输入与预期行为 | 非成功分类（如适用） | TC / 未来测试文件 |
|---|---|---|---|
| L2 正常 MAT | 代表性压缩 double[10×1]、double[10×3]、double[35×1]、double[600×1]、double[600×4]，指定至少两个线性索引与独立只读原值比较；文件 582 个身份/长度/SHA-256 核查 | — | TC-F-MAT-001/002、TC-D-MAT-001/002；`tests/golden/test_mat_archive.cpp` |
| L3 单变量兼容/省略后缀 | 一份单变量 sample.mat，精确读/compat 读/省略 `.mat` 均按 §7.4 候选规则正确返回 | — | TC-F-MAT-003、TC-I-MAT-001；`tests/golden/test_mat_archive.cpp` |
| L3 不存在、不可读、无效文件头或损坏压缩副本 | 分别返回错误，保留源路径、变量与解码阶段信息；**禁止修改源 MAT** | MissingFile / PermissionDenied / InvalidMatHeader / CorruptCompressedData | TC-F-MAT-001/002；`tests/golden/test_mat_archive.cpp` |
| L3 缺少指定变量/空变量/多变量无批准顺序 | strict 必须失败；compat 的多变量不猜字母序 | MissingVariable / AmbiguousVariable | TC-F-MAT-003；`tests/golden/test_mat_archive.cpp` |
| L3 非 double、complex/logical/sparse、错误 rank/dims、零尺寸 | Reader 不暗中转换；inspect 可如实列出元数据；`MatDenseArray::fromColumnMajor` 不允许不满足必要形状的数据 | UnsupportedClassOrType / InvalidShape | TC-F-MAT-002、TC-D-MAT-002；`tests/golden/test_mat_archive.cpp` |
| L3 维度乘法、元素长度或字节数溢出 | 在分配/Map 之前拒绝；长度不等于维度乘积时不得访问越界数据 | SizeOverflow / InvalidShape | TC-D-MAT-002、TC-DC-IO-001；`test_mat_archive.cpp` / `test_mat_raii.cpp` |
| L3 关闭句柄后仍读取返回数组、拷贝/移动与析构 | MatDenseArray 保持自拥有；MatFileHandle/MatVariableHandle 恰好释放一次；失败后 ErrorDetail 不丢失 | 合同属性检查，不产生伪成功 | TC-DC-IO-001；**候选** `tests/contracts/test_mat_raii.cpp` |
| L3 输出目标为 origin、路径穿越、符号链接指回原源 | Writer 不允许任意输出路径；不得更改 45 .m/582 MAT | OutputPathForbidden | TC-NF-SAFETY-001；`tests/contracts/test_output_safety.cpp` |
| L3 已有 RunId、目标 MAT 已存在或并发冲突 | RunContext 新建目录失败或 Writer 拒绝覆盖已存在变量 | OutputExists | TC-F-MAT-004、TC-NF-SAFETY-001；`test_output_safety.cpp` |
| L3 单独的新 RunId + 有效值写回读 | 创建 `output/<RunId>/<variable>.mat`，比对变量名/type/rank/dims/列主序值与 SHA，不覆盖已有文件 | WriteFailed / ReadbackMismatch（失败时） | TC-F-MAT-004、TC-I-RESULT-001；`test_mat_archive.cpp` / `test_adaptive_experiments.cpp` |
| L3 临时文件失败、空间/权限异常、读回不一致 | 返回可识别错误与 RunId/临时文件身份，不擅自清除未知用户产物 | WriteFailed / ReadbackMismatch | TC-NF-SAFETY-001、TC-F-MAT-004；`test_output_safety.cpp` |

**测试执行边界**：所有合成坏文件、写入、并发冲突与文件系统安全测试仅可在后续明确授权的隔离测试目录进行；`origin/**` 作为只读输入，绝不在真实原文件上制造损坏负例。上述表格是 **L3 可独立构造预期** 与 **真实 C++/CTest 尚未执行** 的分离说明，不能以表格存在冒称 TC 通过。也不以历史探针的 582/582 成功推断全部业务值/异常合同已完成。

## 9. 构建 target、头文件与依赖边界（拟议）

- **CMake 归属**：本模块由 [ARCHITECTURE.md §4](../ARCHITECTURE.md) 中的候选 `egomotion_matio` target 实现，拟依赖 `egomotion_contracts`、`egomotion_domain`、`matio` 与 `zlib`；实际 target、链接属性和最小依赖版本**尚未创建/编译/冻结**。
- **头文件**：公共值对象、Reader/Writer 窄接口及候选 `MatDenseArray` 放在 `include/egomotion/io/`，不向外包含 `matio.h`；具体 `MatFileHandle`、`MatVariableHandle`、`mat_t*`、`matvar_t*`、MAT 头/压缩处理均留在 `src/io/` 或此 target 内的私有实现。
- **依赖方向**：`egomotion_experiments` 和 `egomotion_analysis` 只依赖 `egomotion_contracts` 的 `IResultWriter/IResultReader` 与 domain 自拥有值对象，**不能反向链接 `egomotion_matio`**；仅 `egomotion_cli` 的 Composition Root 注入具体 matio 实现。MAT I/O 层不能反向引用 estimators/experiments/plotting。
- **编译前检查**：真实公共头文件的 C++17 自包含性、无 matio 类型泄漏、Result/Status 与 ErrorDetail 失败上下文、目标链接无反向依赖、RAII 释放及 Eigen copy/map 生命周期均需要未来构建/CTest 证据，当前只能静态审阅。
- **冻结条件**：按 P0.8/DEC-008 审查 MatDenseArray 的 double-only vs 未来类型化策略、零长度与 rank、变量枚举、输出路径、签名以及测试文件/CTest；DEC-005 仅用于**数值对照字段**，不将其宽松值套在类型/形状/源文件安全上。**无需 MATLAB 运行或 L1 Golden**。

## 附录 A：本模块无需运行 MATLAB 的验收参考

按 [长期无需 MATLAB 执行政策](../golden/NO_MATLAB_EXECUTION_POLICY.md) 与 [L2/L3/L4 来源矩阵](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md)：原 MATLAB .m 文件仅作为**只读公式、数据布局和顺序规范**；**不要求执行 MATLAB/Octave、采集 L1 或以缺少 L1 阻塞冻结/编码/验收**。

582 个历史 MAT 已通过独立探针的读取报告，当前 L2 需要只读逐值导出与独立 SHA-256，生产 Reader/Writer 仍需安全与回读验证；不要求 MATLAB 运行、L1 中间量或任何 MAT 原地改写。错误负例需隔离副本，且 H-10 暂缓真实写入。

本附录不取消真实 C++/CTest、Golden 独立复核、DEC-005/008 以及源数据保护门禁；当前无新的软件测试通过证据。
