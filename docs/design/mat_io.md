# MAT I/O、路径安全与原始数据合同——详细设计草案

> **LLD v0.2，未冻结、未实现、未完成 MAT 全量字典。** 依据 getflowfield.m、MATLAB 的 load/save 调用、原始归档样本文件头和 AGENTS.md。需求：REQ-F-MAT-001/002/003/004、REQ-D-MAT-001/002/003、REQ-I-MAT-001、REQ-DC-IO-001、REQ-NF-SAFETY-001；任务 P0.5–P0.6、P1.3–P1.10。

## 1. 原始行为、文件身份和支持版本

原 getflowfield(filename) 使用 MATLAB load(filename)，列出 fieldnames，并读取第一个字段。MATLAB 原分析入口经常省略 .mat 后缀，故必须区分：

- **严格读取**：显式 MAT 路径和变量名，变量缺失即失败。
- **旧脚本兼容读取**：按原脚本的路径/后缀解析规则取得数据，只有在确认 MATLAB 首变量枚举规则后才能冻结多变量行为。不能随意以名称字典排序代替原行为。
- **inspect**：列出全部变量元数据而不读取成算法领域类型或更改文件。

目前仅**抽样**发现 MATLAB 5.0 MAT-file 和 miCOMPRESSED；这意味着需要 matio+zlib，但不能据此断言所有 582 文件完全同构。是否需要 HDF5、MAT v7.3 由 DEC-001/002 全量扫描确定。

### 1.1 MAT 类型支持决策矩阵（尚未验证）

| MAT class / 元素形态 | 当前要求的行为 | 当前证明 |
|---|---|---|
| 普通实数 double | 读取形状、数值、列主序；优先验证 Golden | MAT v5 压缩头抽样，仅元素类型未全部验证 |
| 普通实数 single | 如实际存在则明确读取原类型，不能静默升级 | 待 582 文件全量检查 |
| 有符号/无符号整数 | 如存在须保留宽度和符号或显式拒绝 | 待全量检查 |
| 逻辑 logical | 如存在须在接口中明确布尔语义或显式拒绝 | 待全量检查 |
| 复数 complex | 实/虚部、数据类型、内存布局须核验；未实现不得伪造实数 | 待全量检查 |
| struct / cell | 递归字段与子项需独立合同，确认实际需求后实现 | 待全量检查 |
| sparse / 高维数组 | 索引、稀疏模式、rank/dims 与列主序须明确；未实现则拒绝 | 待全量检查 |
| MAT v7.3/HDF5 | 仅在真实源文件存在时评估是否加入 | 未发现证据；不得宣称不可能存在 |

上述矩阵是**类型判定策略**，不是“已支持的功能列表”。真实支持状态必须在 docs/MAT_DATA_SCHEMA.md 中逐类型更新。

## 2. 候选 C++17 类接口与资源所有权

~~~cpp
// 尚未创建、尚未编译；Result<T,E> 为计划中的项目内部结果类型。
enum class MatError {
    MissingFile, PermissionDenied, InvalidHeader, DecompressionFailed,
    MissingVariable, AmbiguousVariable, UnsupportedClass,
    InvalidRankOrShape, SizeOverflow, InvalidEncoding,
    ForbiddenOutputPath, OutputAlreadyExists, WriteOrReadbackFailed
};
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
    Result<std::vector<MatVariableInfo>, MatError> inspect(
        const std::filesystem::path& source) const;
    Result<MatDenseArray, MatError> readNamed(
        const std::filesystem::path& source, std::string_view variable) const;
    Result<MatDenseArray, MatError> readFirstCompatible(
        const std::filesystem::path& source) const;
};
class RunMatWriter {
public:
    Result<void, MatError> saveNewVariable(
        const std::filesystem::path& output_root,
        std::string_view run_id,
        std::string_view mat_variable_name,
        const MatDenseArray& value) const;
};
~~~

C++17 的 Result<void,E> 需要项目自定义特化或选用其他经批准的结果方式；**这些是拟议的签名和枚举，不是已经冻结的 API**。对 matio 的资源操作建议使用 Mat_Open/Mat_Close 与 Mat_VarRead/Mat_VarFree 等 C API，但调用细节须根据项目选定的 matio 版本核准。

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
1. 要求用户给出独立 output-root 和合法 RunId，不允许把 origin/** 设成输出路径；
2. 对路径绝对化并解析已存在父目录及符号链接；拒绝 canonical 祖先关系下覆盖原始输入。需要考虑 ../、符号链接、同一路径多种表示、UTF-8/空格及软链接中途改变问题；
3. 对最终文件使用**不覆盖**的独占创建/安全发布策略；不能仅靠“先 exists 再 open”保证无竞态。实际 matio 写出/临时文件/原子发布的组合在 Debian/Ubuntu 上需验证；
4. 写后独立回读比对变量名、rank/dims、类型、数值，记录结果 SHA；失败时保留日志与不完整输出状态，但不得覆盖任何已有结果；
5. 输出目录和原始目录隔离为设计及测试合同，具体软链接与竞态防护实现需要 P1.9 通过后才可声称安全。

### 3.1 输入/输出错误与回滚

- 输入损坏：报告文件、变量、错误类别；不能通过跳过错误字段伪造成功。
- 输出已存在或路径不合法：明确拒绝，默认不覆盖。
- 写入过程中失败：报告不完整的产物和 RunId；该行为不等同于已完成的原子事务。是否清除临时文件须遵守 AGENTS.md 禁止擅自删除/清理用户数据的要求。
- 内存释放由 RAII 析构负责；不得在异常状态产生悬垂指针。

## 4. MAT 数据字典和 Golden 来源

docs/MAT_DATA_SCHEMA.md 在 P0.5/P1.8 形成前只能作为待填模板。每个源文件应记录：

| 字段 | 预期内容 | 状态 |
|---|---|---|
| 相对路径、Git blob SHA、文件字节数 | 基于 origin/** 的实际树 | 目录已知，逐文件表未生成 |
| MAT 版本、是否压缩、字节序 | 逐文件头和解压检查 | 部分样本头已检查 |
| 变量名、MAT class、元素类型、rank、dims | matio 原始数据 | 尚未 582 文件全量读取 |
| 数值范围/非有限/元素 hash | 在可读类型上实际遍历 | 尚未执行 |
| 严格/兼容读取策略、分析依赖 | 源 load/getflowfield 和数据字典映射 | 待冻结 |
| 解码状态、错误码、是否受版权限制 | 成功/失败/不支持/权限阻塞 | 待验证 |

输出合同：Bias 每目录 xwerte、ywerte1…6；Adaptive 使用初始化四类结果、600 点比较五类结果和更新频率三类当前源码结果。**历史 MAT 的额外文件属于归档证据，不得视为当前脚本已经写出。**

## 5. 接口与测试矩阵

- TC-F-MAT-001/002/003/004：抽样与全量压缩读取、变量枚举、严格/兼容首变量规则、MAT 结果写回读。
- TC-D-MAT-001/002/003：582 文件字典、MAT 型别和矩阵布局与原始变量名。
- TC-I-MAT-001、TC-DC-IO-001、TC-NF-SAFETY-001：RAII 句柄所有权、无悬垂 Eigen Map、目录/符号链接/RunId 冲突和拒绝覆盖。
- 对非 ASCII 路径、损坏 MAT、类型不支持、维度/长度溢出、读后资源释放要有实际负例。输入源哈希与各失败日志必须保持。

## 6. 待决与冻结门槛

精确的 matio 版本、CMake target、class 类型清单、数组拷贝/映射方法、错误枚举、CLI 错误码、Result<T,E> 实现、具体 Golden 文件及哈希、输出安全方案、峰值内存测量、DEC-001/002/005 均未完成审核。**不把候选接口当作已经建成的 C++ 程序。**
