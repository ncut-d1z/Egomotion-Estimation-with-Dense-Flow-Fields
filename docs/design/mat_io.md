# MAT I/O、路径安全与原始数据合同——详细设计草案

> **LLD v0.4，未冻结。独立 matio 只读探针已实现并在 Ubuntu 24.04 上完成 582 文件全量变量读取；正式业务 Reader/Writer、负例、写回读及 Golden 仍未完成。** 依据 getflowfield.m、MATLAB 的 load/save 调用、原始归档样本文件头和 AGENTS.md。需求：REQ-F-MAT-001/002/003/004、REQ-D-MAT-001/002/003、REQ-I-MAT-001、REQ-DC-IO-001、REQ-NF-SAFETY-001；任务 P0.5–P0.6、P1.3–P1.10。

## 1. 原始行为、文件身份和支持版本

原 getflowfield(filename) 使用 MATLAB load(filename)，列出 fieldnames，并读取第一个字段。MATLAB 原分析入口经常省略 .mat 后缀，故必须区分：

- **严格读取**：显式 MAT 路径和变量名，变量缺失即失败。
- **旧脚本兼容读取**：按原脚本的路径/后缀解析规则取得数据，只有在确认 MATLAB 首变量枚举规则后才能冻结多变量行为。不能随意以名称字典排序代替原行为。
- **inspect**：列出全部变量元数据而不读取成算法领域类型或更改文件。

**现有归档的格式已经实测**：`src/mat_data_dict_probe.cpp` 在 Ubuntu 24.04/matio 1.5.26 以 `Mat_Open(..., MAT_ACC_RDONLY)` 和 `Mat_VarRead` 读取全部 582 MAT/582 变量，失败 0；每个源 MAT 是 v5/zlib 压缩、单变量 class=double/data_type=double，flags 均为 `-`。详见 [MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md)。当前数据不需要 HDF5；future 扩展到 v7.3 时再单独审核。

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
// 尚未创建、尚未编译；Result<T,E> 为计划中的项目内部结果类型。
// MatErrorCode 统一在 contracts.md 声明，不在此重复定义。
struct MatVariableInfo {
    std::string name;
    std::vector<std::size_t> dimensions;  // 按 MATLAB 原 rank/dims
    int original_class_type;              // 来自 matio，转换为稳定项目类型尚待定
    int original_data_type;
    bool is_complex;
    bool is_logical;
};
struct MatDenseArray;                     // 拥有数据类型、rank、dims、列主序缓冲区
class MatFileHandle;                      // move-only, 对应 mat_t* 资源所有权
class MatVariableHandle;                  // move-only, 对应 matvar_t* 资源所有权
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
    Status<MatErrorCode> saveNewVariable(
        const std::filesystem::path& output_root,
        std::string_view run_id,
        std::string_view mat_variable_name,
        const MatDenseArray& value) const;
};
~~~

C++17 的无值操作采用 contracts.md 中的 Status<E> 候选，具体实现须经项目审定；**这些是拟议的签名和枚举，不是已经冻结的 API**。对 matio 的资源操作建议使用 Mat_Open/Mat_Close 与 Mat_VarRead/Mat_VarFree 等 C API，但调用细节须根据项目选定的 matio 版本核准。

### 2.1 内存所有权、shape 与类型合同

- MatFileHandle、MatVariableHandle 应为 move-only；不允许跨已关闭的文件句柄或已释放的变量借用数据视图。
- MatDenseArray **拥有自己的缓冲区**，保存 MAT 原始 class/dtype、rank、dims、元素大小和列主序；需要转换为 Eigen 时由显式校验后的 copy/map 决策完成。
- 数据大小 = 元素数 × 元素字节数；计算时必须检查 size_t 乘法溢出、MAT rank 与各维是否异常；不静默将 MATLAB single 转 double 或将 N×6 隐式转置。
- 语义访问与原始物理存储分开。MAT 的线性索引与列主序严格保持，C++ 索引应标注 0-based 的边界。

## 3. 输入路径与输出路径的强制安全流程

**输入阶段**：
1. 验证输入路径存在、可读、常规文件，记录可审查的规范化路径与 SHA；
2. 以只读方式打开 matio 句柄，读取头/变量元数据并校验;
3. 对压缩块做出明确的解码结果；不能把“仅能读取头”当作“全部变量读取成功”；
4. 返回自拥有的领域数据或具体错误码；输入文件及 origin 目录保持不变。

**输出阶段（拟议保护合同）**：
1. **H-12 已指定仓库根 `output/` 为新数据的唯一规划隔离根**。CLI 的 `--output-root DIR` 只可作为指向**根 `output/` 本身**的候选参数，不可指向 `output/<RunId>/` 或其他任意输出根；RunContext 负责校验根身份、生成/接收经审核的 RunId，并在 `output/<RunId>/` 中独占创建新结果，Writer 不得自行更改隔离根。此处仍是 H-03/DEC-008 **待冻结的接口方案**，不可据此默认执行写入。任何 `origin/**` 目标、路径逃逸、符号链接覆盖均应拒绝；
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

## 4. MAT 数据字典和 Golden 来源

**正式 matio 全量字典已存在**：[MAT_DATA_SCHEMA.md](../MAT_DATA_SCHEMA.md)，逐文件列出 FNV64、版本、变量、class/type、dims、flags 和读取结果。以下表格区分已记录证据与仍需开发的业务 Reader/数值验收：

| 字段 | 预期内容 | 状态 |
|---|---|---|
| 相对路径、文件字节数与 FNV64（报告） | 已由探针生成 | **582/582 已记录；FNV 不是 Git SHA/密码学 SHA-256** |
| MAT 版本、压缩、只读加载 | Mat_GetVersion、Mat_VarRead | **582/582 MAT v5+zlib，READ_OK** |
| 变量名、MAT class、物理 data_type、rank/dims | Mat_VarRead 后读取 matvar_t 元数据 | **582/582 double/double，五种 shape** |
| 数值范围、NaN/Inf、逐元素 MATLAB Golden/加密哈希 | 生产 Reader 按值遍历及独立对照 | **尚未验证**；探针成功读取不能替代数值等价 |
| 严格/兼容读取策略、分析依赖 | 源 load/getflowfield 和数据字典映射 | 待冻结 |
| 解码状态、错误码、是否受版权限制 | 成功/失败/不支持/权限阻塞 | 待验证 |

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

## 5. 接口与测试矩阵

- TC-F-MAT-001/002/003/004：抽样与全量压缩读取、变量枚举、严格/兼容首变量规则、MAT 结果写回读。
- TC-D-MAT-001/002/003：582 文件字典、MAT 型别和矩阵布局与原始变量名。
- TC-I-MAT-001、TC-DC-IO-001、TC-NF-SAFETY-001：RAII 句柄所有权、无悬垂 Eigen Map、目录/符号链接/RunId 冲突和拒绝覆盖。
- 对非 ASCII 路径、损坏 MAT、类型不支持、维度/长度溢出、读后资源释放要有实际负例。输入源哈希与各失败日志必须保持。

## 6. 待决与冻结门槛

实测 matio 版本 **1.5.26**、归档 class/dtype/rank/dims 已确认。**仍待**：完整业务 Reader 的数据复制/映射、CMake target、异常与 CLI 合同、Result<T,E>、文件名/首变量兼容规则、Golden 逐值对照、输出写入与路径安全、DEC-005 容差和 P10.4 独立复核。**不把候选接口当作已经建成的 C++ 程序。**


## 7. API 的前置/后置条件与错误映射（编码候选）

公共类型与错误枚举的单一维护位置为 [contracts.md](contracts.md)。第 2 节的示例签名是拟议 API；本节补充**可用来审查方法的完整行为合同**，并不表示实现已创建。

| 方法 | 前置条件 | 后置条件 | 非成功分类 | 关联 TC |
|---|---|---|---|---|
| inspect(path) | 输入为存在的普通文件，且 Reader 不持有写权限 | 按 MAT 文件真实变量记录返回 name/class/dtype/rank/dims/complex，绝不变更输入 | MissingFile、InvalidMatHeader、CorruptCompressedData | TC-F-MAT-002 |
| readNamed(path,name) | name 非空；所需 class 已受支持 | 对**唯一指定变量**读取完整自拥有数据，源文件句柄销毁后结果仍有效 | MissingVariable、UnsupportedClassOrType、InvalidShape | TC-F-MAT-001/003 |
| readFirstCompatible(path) | 路径可按原 MATLAB load 规则解析；变量顺序已核验 | 首变量数据与 getflowfield MATLAB 输出一致，省略后缀时按确认后的规则补全 | AmbiguousVariable（规则未确定时）、MissingVariable | TC-F-MAT-003 |
| saveNewVariable(root,RunId,name,data) | root 限定在经 H-12 指定的 `output/`（待 CLI 参数合同批准），与 origin 不重叠；RunId 和变量名合法；输出不存在 | 新建结果 MAT，写后回读同名变量的 dtype/rank/dims/数据，并保存 manifest | OutputPathForbidden、OutputExists、WriteFailed、ReadbackMismatch | TC-F-MAT-004 |
| 句柄析构/移动 | 只允许单一所有者持有 mat_t*/matvar_t* | 释放恰好一次；move 后源句柄变为无所有权状态 | 不通过异常伪装成功；记录资源失败 | TC-DC-IO-001 |

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
Writer::saveNewVariable:
    validate destination and obtain isolated RunId directory
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

## 8. 可复现测试样例定义（真实文件与数值尚未登记）

- 正例：一个含压缩实数数组的源 MAT；必须记录源相对路径、Git blob SHA、MAT 变量名、rank/dims/dtype、至少两个指定线性索引处的值。
- 负例：不存在文件、损坏压缩数据（从**测试副本**制造）、未支持 class、shape 冲突、多个变量的兼容选择不确定、读后析构、只读文件。
- 写路径：新 RunId 成功回读、重复 RunId、输出位于 origin、输出经过符号链接回指 origin、相对路径穿越、中文空格路径、模拟写入失败。不得用真实 origin 作为破坏性测试目标。
- 证据：TC-F-MAT-001…004、TC-D-MAT-001、TC-I-MAT-001、TC-DC-IO-001、TC-NF-SAFETY-001；未取得 Golden 元素值/批准容差前，状态一律“未验证”。

**模块就绪状态：独立 matio 1.5.26 探针已在 Ubuntu 24.04 运行，582/582 MAT 文件/变量解码成功，格式与维度合同明确。** 但业务 Reader/Writer、接口最终签名、异常负例、列主序值对照、Golden、CTest、P10.4 独立审核仍未完成。

## 附录 A：MAT 文件参考数据为 L2，非迭代 L1

[Golden L1–L4 分级清单](../golden/GOLDEN_EVIDENCE_LEVELS_AND_AC_GAPS.md) 把现存 582 MAT 的**最终变量**视为 L2 来源。此前 matio 探针记录了 582/582 文件及变量成功读取，证明已报告的格式/类型/尺寸子能力；**没有在此轮生成原 MATLAB 逐轮中间状态 L1，也尚未完成业务 Reader 逐元素参考导出和 582 文件独立 SHA-256 复核**。只有在将来独立只读提取并保存源身份后，才能将具体列主序 double 值作为逐元素 L2 参考使用。

损坏 MAT 负例属于独立构造的 Reader 合同测试输入，不应被描述成原 MATLAB Golden；生产 Writer 回读、RAII/TOCTOU 与源文件身份不变检查也不是 L1 取证。即便 H-12 规划 `output/<RunId>/`，H-10 暂缓且 `permissions.writeMat=false` 仍禁止本轮运行写回读。
