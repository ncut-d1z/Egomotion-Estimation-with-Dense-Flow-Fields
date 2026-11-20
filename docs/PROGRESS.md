# 实施进度与证据登记（PROGRESS）

## 当前接续：R6 四项指标决定与方案 B（2026-11-20）

**本轮实测**：远端检查点 `594a1b11f91028cb7549fef0ab1cd512ee4f7edb`，Debian 13.7 / GCC 14.2.0 / Eigen 3.4.0，29 个生产文件与 11 个测试程序编译成功，有限子集 **12/12 通过、0 失败/阻塞**；另 13 文件语法检查通过，日志无诊断。源码/依赖头摘要前后不变，原资料树 `70ee155...` 未改。RunId `r6-metric-subset_20261120T043907Z_594a1b11f910_2`；实际命令与明细见 [本轮记录](reviews/R6_METRIC_DECISIONS_IMPLEMENTATION_2026-11-20.md)。环境缺少 CMake/matio/Sophus，完整 CMake/CTest、真实新 MAT/CLI 未执行；没有安装依赖、没有继承旧73/72、没有运行完整科学实验。

`[HUMAN-EXECUTE/APPROVE]` 所有者已确认 DEC-R6-01～04 并要求继续实现。源码覆盖稳定 atan2 平移方向角、SO(3) 旋转矩阵测地角、求解失败回滚/指标无定义继续、八轮固定分母与逐轮缺测证据、显式掩码 MAT、v2 Manifest、分析 JSON null 与 SVG 缺口。旧 Adaptive 写入接口不能绕过新指标声明；方案 B 成功交付返回实际 Complete/Incomplete，CLI 退出 0 表示交付成功，求解/输出故障仍分别走 5/6。逐项范围和当前实际验证见 [本轮记录](reviews/R6_METRIC_DECISIONS_IMPLEMENTATION_2026-11-20.md)，决定见 [所有者记录](decisions/PROJECT_OWNER_DECISIONS_2026-11-20.md)。下文旧 SHA 的完整 CTest 结果保留历史身份，不充当新版本通过证据。完整科学实验、最终容差/性能资格、冻结和发布继续延期，R6 总体验收未结案。

## 本次代码与构建验证请求交付复核（2026-11-18）

`[AI-EDIT]` 核对已推送e2275d0及Draft PR #6：D1–D5已列实现缺口均有修复/有限回归，当前62个生产翻译单元全部接入CMake和ON实际编译清单；OFF仅排除两个SVG实现。受测代码3985b05至当前无变化，重新读取四份原始JUnit/meta确认ON73/73、OFF72/72及构建退出0，739项输入SHA不变，98项P/66项规范TC集合一致。本次用户要求的代码补齐、编译和CTest验证已交付；本段不是R6总体、科学实验或REQ/AC的验收签字。三Adaptive原场景仍Incomplete，完整Bias/L4、病态/跨环境资格及已延期审批继续开放。未新增生产代码或重复运行测试，原始失败记录保留；见[交付范围复核](reviews/R6_DRAFT_DELIVERY_2026-11-18.md#本次代码与构建验证请求的交付范围复核)，只读证据`build/r6-scope-audit-20261118-RdRFcN3p/`。

## 最新接续：P10.8阶段Draft PR交付（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 已创建并API回读核实[Draft PR #6](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/pull/6)，Open/Draft/未合并，已附加到当前任务。创建时head `9c0022d2d7c3d19890cc46988f63b0544823d599`、base `eb08c6ba006727c3361af9ea8cb424ece99ba4e0`，main为祖先；353文件/1081提交的既有历史保留。PR按模块给出审查入口、固定受测3985b05的ON73/OFF72证据、Issue #1–#5修复链及原轨迹Incomplete范围。重新读取四份JUnit逐名匹配，并核对739项当前输入SHA；本轮仅文档/只读交付审计，没有重新构建/CTest。P10.8的创建动作完成，审查与正式验收未通过；不变更延期的Ready/合并/发布。详见[交付记录](reviews/R6_DRAFT_DELIVERY_2026-11-18.md)，证据目录`build/r6-draft-handoff-20261118-5nZ8yTP0/`；原资料和生产代码不变，R6仍未结案。

## 最新接续：R6真实Adaptive完整交付链独立参考（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 受测源码`3985b056aa19935c8348f0fefa5f5292f9fbd3b4`两模式配置/编译无诊断，**SVG ON73/73、OFF72/72 CTest**；先MAT7项，再66/65项，零失败/跳过/禁用。新增合成固定输入→真实Adaptive Step/Session→完整35/600帧及8轮聚合→MAT交付独立对照：3676721项、17441次Step、12个MAT/8540值通过，保存值最大差5.406439324458240113e-10，沿用原1e-9工程余量。两配置Step JSONL与MAT数值载荷相同，739项输入SHA及21组既有摘要不变。独立标量公式从自身状态演进，Comparison连续172224次normal、四错误数组固定除8、ratio取末轮，三合成Manifest Complete。RunId `r6-real-delivery-20261118-ghz37p6I`保留完整逐节点JSONL、MAT/Manifest、四份JUnit及通道日志，见 [专项记录](reviews/R6_REAL_STEP_DELIVERY_L3_2026-11-18.md)。合成场景不等于原科学实验成功；原轨迹三入口仍Incomplete、无完整科学MAT，L4、病态资格及正式验收未完成。Issue #1–#5开放，无PR，R6未结案。

## 最新接续：R6含噪Session独立多帧参考（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`928a8071c7c4ff70b7e38a4ed91c13a31c770323`两模式配置/编译无诊断，**SVG ON72/72、OFF71/71 CTest**；先MAT7项，再65/64项，零失败/跳过/禁用。新增固定噪声→真实Session独立参考：0/±0.1系数、两流252帧、9072次normal、24回滚，290923项检查通过；噪声场最大差3.06178693509906452164e-16，Step/状态最大差1.94913529760754045128e-14。两配置含噪JSONL对应SHA相同，原无噪声多帧JSONL与493ba86完全相同，737项输入SHA及21组既有摘要不变。真实均模长噪声与独立long double投影分别产生观测，参考从自身状态演进；原无噪声299236项/252帧保留。新增756成功帧Step+24失败比较Step，另18默认Session首帧Step；不是完整科学样本。RunId `r6-noisy-session-20261118-FnQ0oODZ`，逐帧输入/状态/节点JSONL、原始通道及四份JUnit保留，见 [专项记录](reviews/R6_NOISY_SESSION_L3_2026-11-18.md)。仅补SN-G06/AD-G05固定输入子范围；生产公式和余量未变，完整科学实验、病态资格及正式验收未完成，真实Adaptive仍Incomplete、无完整MAT。Issue #1–#5开放，无PR，R6未结案。

## 最新接续：R6 Adaptive Session独立多帧参考（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`493ba86917a8147a0ddf63fad214751f1b33dd9c`两模式配置/编译无诊断，**SVG ON71/71、OFF70/70 CTest**；先MAT7项，再64/63项，零失败/跳过/禁用。新增Adaptive Session独立long double多帧参考：三模式/两流共252帧、24次真实角度失败回滚、299236项检查通过；最大数值绝对差1.38144704009413032963e-14，比较角度最大差5.82190545772576228956e-14度。两配置逐帧参考/实际JSONL对应SHA相同；736项输入SHA及21组既有数值/合同摘要不变。参考从自身状态演进，使用独立标量消元与三维多项式旋转，不从生产结果重置Expected；沿用单步/角度工程余量。成功帧924个真实Step、失败尝试24个比较Step，另六默认Session首帧共22个Step；不将调用数当科学样本。RunId `r6-session-l3-20261118-UF1B2GwG`，完整输入/状态/逐节点JSONL及四份JUnit保留，见 [专项记录](reviews/R6_SESSION_MULTIFRAME_L3_2026-11-18.md)。本批补多帧/周期有限L3证据，未改变生产公式或工程余量；完整科学实验、病态资格和正式验收仍未完成，真实Adaptive仍Incomplete、无完整MAT。Issue #1–#5开放，无PR，R6未结案。

## 最新接续：R6进程记录关联实际应用身份（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`ff8aaa723d979642cca30b6fc79953fc8f2963ed`两模式构建无诊断，**SVG ON70/70、OFF69/69 CTest**；先MAT7项，再63/62项，零失败/跳过/禁用。进程记录已关联实际RunId、选定seed、有效配置与sealed MAT快照；各224项上下文检查、464项/26次进程检查、29次真实CLI调用通过。733项输入SHA及21组既有数值/合同摘要不变。首轮798c84f ON69/70，发现导出观察失败诊断遗漏已预留RunId，登记Issue #5；ff8aaa7补全身份并保留断言，双配置复测通过。日志RunId `r6-context-journal-final-20261118-TbQO9zck`，旧失败证据保留，见 [专项记录](reviews/R6_CONTEXT_JOURNAL_2026-11-18.md)。D4所列身份关联缺口已补有限实测；REQ-NF-LOG整体及R6未验收，完整科学实验/最终资格未完成，真实Adaptive仍Incomplete、无MAT。Issue #1–#5保持开放，无PR。

## 最新接续：R6独立进程记录与终止状态（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`617c7f8a21966ee8f8f9974bfd8ab0f123c50eda`两模式构建无诊断，**SVG ON69/69、OFF68/68 CTest**；先MAT7项，再62/61项，零失败/跳过/禁用。新增显式egomotion-record在私有build-cli-records目录保存全程开始/结束与原始通道，区分正常退出和信号终止；各399项/23次受监控调用、12组/24次真实CLI对照通过。首轮300aae5的碰撞夹具因build0775先被安全检查拒绝，登记Issue #4；f8d556a修复夹具，e16b27e延长父进程SIGPIPE保护至故障诊断结束，ON69/69通过。最终617c7f8改用独立私有日志根，保留0775现有build权限、符号链接/共享日志拒绝和全部失败证据。各732项输入SHA不变，21组既有数值/合同摘要不变，RunId `r6-process-records-private-20261118-d91C3PbC`，详见 [专项记录](reviews/R6_PROCESS_RECORDS_2026-11-18.md)。外层尚未关联应用RunId/seed/实际MAT快照身份，不能将进程记录等同整个REQ-NF-LOG验收；完整科学实验和正式资格仍未完成，真实Adaptive仍Incomplete、无MAT。Issue #1–#4开放，无PR；继续D4上下文关联与剩余P/TC义务。

## 最新接续：R6流场构造与方向独立拟合（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`b2628a97d1ca74844a2b57fab035387ce847670c`两模式构建无诊断，**SVG ON67/67、OFF66/66 CTest**；先MAT7项，再60/59项，零失败/跳过/禁用。FlowField已封装N×6有限存储、只读访问、检查复制与严格所有权移交；新增1245项布局/有限性/寿命/分配故障检查通过。HarmonicFitter方向入口保留前三列投影语义，球谐服务494项通过。空/零/非单位/有限非切向存储保留原消费端判断，不新增投影/容差。NaN噪声产物在工厂拒绝并保留Runner行号/错误链，原36091项注入、172962入口及88121场景检查保留。各730项输入SHA不变，RunId `r6-flow-value-20261118-dcfqcXP8`，见 [专项记录](reviews/R6_FLOW_VALUE_2026-11-18.md)。D1已列明四类构造/存储缺口补齐有限实测；继续D4只读/预留前记录和剩余R6义务。完整实验/正式资格未完成，Issue #1/#2/#3开放、无PR，R6未结案。

## 最新接续：R6方向集合构造与所有权（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：最终源码`7f4d248255a1502b65e1f8210071a5d47b919839`两模式构建无诊断，**SVG ON66/66、OFF65/65 CTest**；先MAT7项，再59/58项，零失败/跳过/禁用。DirectionSet已封装N×3有限存储/只读访问，复制工厂和显式所有权移交接入采样/光流/球环境；新增901项形状/非有限/值寿命/分配故障检查及移交类型编译断言通过。默认空集合、有限零/非单位向量在消费端沿用原拒绝，未改归一化/门槛；内部复制分配失败和复制赋值保留旧值、const右值复制/普通右值移交均已测。首轮`20f15b5`ON66/66后，静态发现Eigen隐式转换入口并以新提交收紧，日志独立保留。各728项输入SHA不变，RunId `r6-direction-final-20261118-SiB24fFB`，见 [专项记录](reviews/R6_DIRECTION_VALUE_2026-11-18.md)。D1现已补运动/系数/方向集合，继续FlowField及D4边界；完整科学实验/正式资格未完成。Issue #1/#2/#3开放、无PR，R6未结案。

## 最新接续：R6球谐系数构造与存储（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`9e4b60f065ecb5625bc30429d59607d9b002cfda`两模式构建无诊断，**SVG ON65/65、OFF64/64 CTest**；先MAT7项，再58/57项，零失败/跳过/禁用。HarmonicCoefficients采用检查工厂、私有3×5列主序存储及只读访问，全部15项有限且保留六个未用位置的有限非零值；默认全零、源初始化零阶项为1。新增178项形状/非有限/逐位保存/值寿命检查通过。拟合/旋转/Session/策略及夹具已迁移；工厂和原始叶子独立保留非有限负例，有限大基线旋转溢出及策略失败回滚/同帧重试通过，不改数学公式/相位/容差。各726项输入摘要不变，RunId `r6-coefficient-value-20261118-SQJ9gcFR`，见 [专项记录](reviews/R6_COEFFICIENT_VALUE_2026-11-18.md)。D1的MotionEstimate与HarmonicCoefficients已补；FlowField/DirectionSet及D4记录边界、完整实验/正式资格仍未完成。Issue #1/#2/#3开放、无PR，R6未结案。

## 最新接续：R6运动值构造与寿命（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`1588ebb97b0c3b0984e5b21335f1e67af9184e62`两模式构建无诊断，**SVG ON64/64、OFF63/63 CTest**；先MAT7项，再57/56项，零失败/跳过/禁用。MotionEstimate采用私有有限3维分量、检查工厂/只读getter、明确静止默认值和自拥有右值访问，Franz/KvD/Adaptive/Runner生产路径及夹具已迁移；无归一化/新数值门槛。新增839项构造/维度/非有限/值寿命检查，两模式原172962入口隔离/88121场景/独立数值回归保留。RunId `r6-motion-value-20261118-9j2NQZaD`，各724项输入摘要不变，见 [专项记录](reviews/R6_MOTION_VALUE_2026-11-18.md)。D1的MotionEstimate缺口已补，FlowField/DirectionSet/HarmonicCoefficients及D4记录边界仍待处理；完整实验与正式资格未完成。Issue #1/#2/#3开放、无PR，R6未结案。

## 最新接续：R6三Adaptive入口编排（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`34fe509240167a357afc43da71c588534bee005c`两模式构建无诊断，**SVG ON63/63、OFF62/62 CTest**；先MAT7项，再56/55项，零失败/跳过/禁用。P8.9的InitializationRunner、AdaptiveComparisonRunner、UpdateRateRunner已接管各自Config/Context检查、进度及发布，Production只装配分派，旧内存入口保留兼容；共享计算函数体保持。新增入口隔离两模式各172962项，六次交错完整运行、12错误Context、非法Config及第五轮失败后同Runner重新完成八轮；原88121项场景及真实数值回归通过。RunId `r6-adaptive-entries-20261118-gBryoJ8Q`，各722项输入摘要不变，见 [专项记录](reviews/R6_ADAPTIVE_ENTRIES_2026-11-18.md)。D2已列明源码职责缺口已补，总体对象/数值验收仍未签署；D1构造不变量、D4记录范围、完整科学实验及正式资格未完成。Issue #1/#2/#3开放、无PR，R6未结案。

## 最新接续：R6 P9.8分析组件分离（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`03f7f81b6605a4492890a44e9e28290b8f3d785d`两模式构建无诊断，**SVG ON62/62、OFF61/61 CTest**；先MAT7项，零失败/跳过/禁用。BiasResultAnalyzer/AdaptiveResultAnalyzer分别拥有原分析逻辑，MatResultAnalyzer只分派，原函数体及异常分类保持。新测试各727项，覆盖四入口两次交替、40读取故障、结果自拥有及导出失败不改值；历史独立曲线/平滑和原回归保留。RunId `r6-analyzer-services-20261118-0WFb9nRp`，各722项输入摘要不变，见 [专项记录](reviews/R6_ANALYZER_SERVICES_2026-11-18.md)。P9.8源码职责缺口已补，D1、D2其余Runner职责、D4记录边界及完整实验/资格仍未完成，R6未结案；Issue #1/#2/#3开放、无PR。

## 最新接续：R6 P7.10球谐职责分离（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`a19d2d82ab9b7b27f9b77061e9f4880c8d8944bf`两模式构建无诊断，**SVG ON61/61、OFF60/60 CTest**；先MAT7项，零失败/跳过/禁用。RealSphericalHarmonics/HarmonicFitter/HarmonicRotator分别拥有源基值/重建、逐项拟合及两种旋转动作，实际单步/Session/策略已接入；9个迁移函数体仅改辅助命名空间限定，叶子公式不变。新测试各482项，最大绝对差1.7763568394002505e-15；完整原回归保留。RunId `r6-harmonic-services-20261118-e9E0GmUL`，各721项输入摘要不变，见 [专项记录](reviews/R6_HARMONIC_SERVICES_2026-11-18.md)。P7.10源码缺口已补，D1、D2其余Runner/Analyzer职责、D4记录范围和完整实验/资格仍未完成，R6未结案；Issue #1/#2/#3开放、无PR。

## 最新接续：R6 Adaptive场景注入与八轮合同（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`74dbccd01afe23465d7914444917f7f357e69f9a`两模式构建无诊断，**SVG ON60/60、OFF59/59 CTest**；先MAT7项，零失败/跳过/禁用。内存Runner显式借用轨迹/运动学/球环境，生产默认装配及原数学函数体保持。各88121项检查覆盖三入口合成完整场景、Comparison八轮状态重置/固定分母/末轮比值、54个服务故障及第5轮失败不发布部分结果。RunId `r6-adaptive-scene-20261118-BLAFVtQC`，各720项输入摘要不变，见 [专项记录](reviews/R6_ADAPTIVE_SCENE_2026-11-18.md)。真实Adaptive仍按原数值行为Incomplete、无MAT；D3已列明的装配源码缺口已补，D1/D2/D4及完整实验/资格仍未完成，R6未结案。Issue #1/#2/#3开放、无PR。下方较早记录保留当时状态。

## 最新接续：R6 Adaptive采样/噪声注入（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`0cd5439b7195cd6b7355deb5f7eb951a649be8b1`两模式构建无诊断，**SVG ON59/59、OFF58/58 CTest**；先MAT7项，零失败/跳过/禁用。内存Runner显式借用采样/噪声服务，生产默认装配及RunContext RNG保持；噪声success的形状、有限性和方向列在Session前检查。新测试各36088项，含24个服务故障和三个生产入口默认/显式逐位对照，真实数值回归保留。RunId `r6-adaptive-injection-20261118-o87077mF`，各719项输入摘要不变，见 [专项记录](reviews/R6_ADAPTIVE_INJECTION_2026-11-18.md)。Issue #1/#2/#3开放、无PR；D3其它轨迹/环境装配、D1/D2/D4及完整实验/资格未完成，R6未结案。

## 最新接续：R6源球面角度接口（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`cce0bf2c66506e32dd5400e22a6b8d95b5ae6929`两模式构建无诊断，**SVG ON58/58、OFF57/57 CTest**；先MAT7项，零失败/跳过/禁用。P7.1新增`sourceAnglesFromDirection`，保留atan2/acos顺序、轴及弧度约定，不归一化/裁剪，现有Cartesian球谐路径不变。18个解析方向及边界共1137项，最大绝对残差1.3877787807814457e-16；此前SH/Session/KvD/MAT/CLI回归保留。RunId `r6-source-angles-20261118-ksdFXz5p`，各718项输入摘要不变，见 [专项记录](reviews/R6_SOURCE_ANGLES_2026-11-18.md)。Issue #1/#2/#3仍开放、无PR；其它职责/不变量/装配/记录范围、完整实验及正式资格未完成，R6未结案。

## 最新接续：R6 Session系数更新策略（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`5318cd817aa781345c159bd4ba22ee00fe926ee2`两模式构建无诊断、**SVG ON57/57、OFF56/56 CTest**，先MAT7项，零失败/跳过/禁用。P8.8已实现Reset/Inherit/Periodic的`ICoefficientUpdatePolicy`及Session注入，主/基线状态仍由Session原子提交；策略测试各48523项、真实Session交错9579项及既有回归通过。首轮测试修改Result只读载荷编译失败、第二轮trace未初始化警告均已修复，失败证据及 [Issue #3](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/3)保持开放供复核。RunId `r6-coefficient-final-20261118-A1AX7yeP`，见 [专项记录](reviews/R6_COEFFICIENT_POLICY_2026-11-18.md)。输入SHA不变；D2的Session策略缺口已补，其它D1/D2/D3/D4与完整实验/资格仍未完成，R6未结案。

## 最新接续：R6 KvD引擎与平移策略（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`df511f9924b75ded66b437fd533ca888918420c1`两模式构建无诊断、**SVG ON56/56、OFF55/55 CTest**，先MAT7项，零失败/跳过/禁用。P4.9的`KvDIterationEngine`与`IKvDTranslationRule`已实现，两具名入口组合各自规则；保持原公式、旧运动更新次序与停止条件。新增策略测试各2712项，独立L3/完整trace、506次停止参考及真实Bias回归通过。RunId `r6-kvd-strategy-20261118-iQclVtbH`，见 [专项记录](reviews/R6_KVD_STRATEGY_2026-11-18.md)。输入SHA不变；D2的KvD结构缺口已补，其它D1/D2、Adaptive装配与只读/预留前记录边界仍保留。Issue #1/#2仍开放、无PR；完整实验/资格/冻结未完成，R6未结案。

## 最新接续：R6 plot/SVG导出记录（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`00d174258f502d6c458fb26b8e0727d148b98672`两模式构建无诊断、**ON55/55、OFF54/54 CTest**，零失败/跳过/禁用。plot和SVG插图保留真实参数/来源/输入快照/交付状态，开始记录未确认不导出；运行记录专项394项/38次调用，实际插图CLI各29次。首轮54/55的测试夹具负order错误已修复，失败证据及 [Issue #2](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/2)保持开放供复核。RunId `r6-artifact-retest-20261118-f3PrRXse`，见 [专项记录](reviews/R6_ARTIFACT_RECORDS_2026-11-18.md)。原始SHA不变；D4已分配RunId的导出记录缺口已补，只读/预留前/信号边界、D1/D2及Adaptive其它装配仍待收敛，完整实验/资格/冻结未完成，R6未结案。

## 最新接续：R6 MAT v7.3拒绝（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：修复源码`f1a7ab2aed7795877e070260ebfb1b30a89d6f33`两模式构建无诊断、**ON55/55、OFF54/54 CTest**；MAT73专项32场景/308项通过，版本拒绝不再调用Mat_Open，原582MAT/33340逐位值和快照回归通过，输入SHA不变。旧Reader`0d73ed1`的29/32场景失败记录保留，修复追踪 [Issue #1](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/issues/1)仍开放。RunId `r6-mat73-rejection-20261118-3L89aVqh`，详见 [专项记录](reviews/R6_MAT73_REJECTION_2026-11-18.md)。D5缺口已补；继续D4日志、D1/D2结构和Adaptive其它装配边界，完整实验/资格/冻结未完成，R6未结案。

## 最新接续：R6 Bias依赖注入（2026-11-18）

`[HUMAN-EXECUTE/APPROVE]` 实测：受测源码`909fd57dbafa4ebf680e5edd5fa718adb30b4c51`两模式构建无诊断、**SVG ON54/54、OFF53/53 CTest**，零跳过/禁用。Bias三个求解槽及RunContext RNG注入、两次重置、精确失败前缀/交错隔离/内存交付新增668项检查通过；既有真实Bias数值17695项与MAT发布5560项通过，最大残差6.6613381477509392e-16，输入SHA不变。RunId `r6-bias-injection-20261118-ptpZAp0s`，见 [专项记录](reviews/R6_BIAS_INJECTION_2026-11-18.md)。D3的Bias缺口已补；D5真实MAT v7.3负例、D4日志、D1/D2结构及Adaptive其它装配边界继续实施，完整实验/资格/冻结状态不变，R6未结案。

## 最新接续：98项P任务与数值证据汇总（2026-11-18）

`[AI-EDIT]`：从本地/远端一致的`e24e759`继续，已将PLAN全部98项逐一对应当前实现、有限证据和未完成义务，新增 [任务复核](reviews/R6_PLAN_TASK_AUDIT_2026-11-18.md) 与P10.6要求的 [NUMERICAL_EQUIVALENCE](NUMERICAL_EQUIVALENCE.md)。核对98个编号/顺序/唯一性、66 TC集合、两新报告65个文件引用及53/52项JUnit与CMake逐名映射；本轮只改文档，没有重跑构建/CTest。发现Bias内部创建求解器/RNG仍不满足REQ-DC-EXPERIMENT-001组合注入，另缺真实MAT v7.3拒绝用例；下一步优先补依赖注入与固定替身隔离测试、版本负例。其它候选结构差异、非生成日志、完整Adaptive/L4及正式资格继续保留。实测代码仍为`4c2973b`，R6未结案。

## 最新接续：R6 五实验运行记录（2026-11-18登记，11-17实测）

`[HUMAN-EXECUTE/APPROVE]`：受测源码`4c2973b19c148f640613b03309e8756006adb053`两模式增量构建无诊断，**ON 53/53、OFF 52/52 CTest**，无跳过/禁用。五入口实际argv/Config/环境/seed/通道与条件退出记录已验证；新增106项检查/12次调用，发布及发布后fsync失败、Complete保留、拒覆盖和SIGPIPE缺结束记录均已测，原始输入SHA不变。RunId `r6-execution-records-20261117-C85CvSON`，见 [运行记录专项](reviews/R6_EXECUTION_RECORDS_2026-11-17.md)。继续非生成入口/分配RunId前的记录边界及P/TC核对；三Adaptive完整成功/L4/资源/正式资格仍未完成，R6未结案。

## 最新接续：R6 独立MAT探针输出（2026-11-17）

`[HUMAN-EXECUTE/APPROVE]`：受测源码`56317e2215fca608b6afa2ffd1a144dd3444b4a6`两模式增量构建无诊断、**SVG ON 52/52、OFF 51/51 CTest**，无跳过/禁用项。独立探针每模式165项检查/40次入口调用/17次真实进程调用，write/close故障实际命中并退出2；状态失败保留报告，原582份MAT摘要不变。RunId `r6-dictionary-output-20261117-JDef6yWA`，完整复现、日志与边界见 [探针专项记录](reviews/R6_DICTIONARY_OUTPUT_2026-11-17.md)。下一步补正式运行参数/环境/交付身份记录并核对P/TC；三Adaptive完整成功/L4/资源/正式资格未完成，R6未结案。

## 最新接续：R6 CLI 输出流故障（2026-11-17）

**最终实测 `[HUMAN-EXECUTE/APPROVE]`：`5a2cedc47b59787f441aef377feea2f6c11c2f8c` 两模式无诊断构建，ON 50/50、OFF 49/49 CTest。** 各模式848项/123次装配调用、11次真实`/dev/full`调用及默认SIGPIPE子进程验证通过；失败链在stderr通知之前保存，成功产物不因通知失败而覆盖或删除。输入SHA不变，首轮夹具失败记录保留。RunId `r6-stream-sigpipe-20261117-dPSEjNjc`，见 [最终证据](reviews/R6_CLI_STREAM_OUTPUT_2026-11-17.md)。继续独立探针输出、运行身份记录及P/TC覆盖复核；三Adaptive完整成功/L4/资源/正式资格仍未完成，R6未结案。

**第二轮 `[HUMAN-EXECUTE/APPROVE]`：`a24ac91` 无诊断构建、ON 50/50、OFF 49/49 CTest。** 各模式841项/122次装配及11次真实进程调用通过。再补默认SIGPIPE下先保存失败Manifest的顺序和子进程负例，待新检查点复测；不改变信号策略，见 [证据及边界](reviews/R6_CLI_STREAM_OUTPUT_2026-11-17.md)。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`7c444ec` 无诊断构建、SVG ON 49/50 CTest。** 11次真实`/dev/full`调用通过；唯一失败是最后第二诊断场景的夹具将阻塞目录放错层级，未触发Manifest失败。现补真实RunId路径并保留所有断言，待新检查点复测；失败/输入SHA证据保留，见 [诊断记录](reviews/R6_CLI_STREAM_OUTPUT_2026-11-17.md)。

**准备 `[HUMAN-EXECUTE/APPROVE]`，新代码尚未执行。** 旧二进制向`/dev/full`写help/插图JSON仍返回0，真实复现已保存；现补write/flush/ostream异常检查及独立失败Manifest保存。新增流故障矩阵与实际`/dev/full`进程测试，预期ON 50项、OFF 49项，待远端检查点后验证；见 [缺陷与准备](reviews/R6_CLI_STREAM_OUTPUT_2026-11-17.md)。不改变原始数据、数值判据、持久化计算结果或正式资格，R6未结案。

## 最新接续：R6 可选 SVG 独立构建（2026-11-17）

**实测 `[HUMAN-EXECUTE/APPROVE]`：`f3398b099c70233e55e150ecf41c2dd90f282c00` 两种配置无诊断构建、ON 48/48及OFF 47/47 CTest。** OFF使用全新构建目录，唯一缺席项为SVG专用画幅测试；真实CLI9次合法绘图请求明确拒绝且不预留输出。14份有限Bias MAT载荷和3类插图JSON跨模式一致；输入SHA不变，依赖图无环、OFF实际二进制无SVG实现。RunId `r6-headless-20261117-Joe73gEd`，见 [证据](reviews/R6_HEADLESS_BUILD_2026-11-17.md)。继续日志/输出合同及P/TC覆盖复核；三Adaptive完整成功、L4/资源/正式资格仍未完成，R6未结案。

**准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 默认开启 `EGOMOTION_ENABLE_SVG`；关闭后不构建两个 SVG 库，保留全部数值入口。真实CLI补缺后端错误/不预留输出断言，CTest统一无显示环境；预期ON 48项、OFF 47项，仅少SVG专用画幅测试。见 [范围与准备](reviews/R6_HEADLESS_BUILD_2026-11-17.md)。待远端检查点后分别构建及MAT优先CTest；不改变原始数据、数值判据或正式资格，R6未结案。

## 最新接续：R6 45源现行实现映射（2026-11-17）

**`[AI-EDIT]` 静态复核完成。** [迁移矩阵](MIGRATION_MATRIX.md) 已按真实符号/文件更新45条，明确50个声明的承接、融合及不使用范围；45源blob/字节/工作区摘要一致，48个CTest名称逐项匹配固定130a82成功JUnit。SRS §9.2保留原路径/REQ/TC，仅改过时状态，候选对象模型/全部98项P及66项TC义务不变，见 [记录](reviews/R6_SOURCE_IMPLEMENTATION_MAP_2026-11-17.md)。本轮仅文档、未重跑构建；受测代码仍130a82、48/48。下一项检查交付依赖、可选绘图与日志/输出合同；三Adaptive完整成功、L4/资源/正式资格仍缺，R6未结案。

## 最新接续：R6 MAT封存读取与摘要（2026-11-17）

**实测 `[HUMAN-EXECUTE/APPROVE]`：`130a82ba0090e186fcf43bc28bb98c282e68c96c` 无诊断构建、48/48 CTest（MAT6/6，其余42/42）。** 快照256项检查/30数值场景及metadata通过；582固定SHA/33340逐位值、历史Bias986贡献身份、32条有限分析曲线的独立数值+SHA完整JSON通过。原始SHA不变，RunId `r6-mat-snapshot-20261117-wfxpgjMh`，见 [实测](reviews/R6_MAT_SNAPSHOT_2026-11-17.md)。MAT读取字节与分析来源摘要已绑定；下一项为45源现行映射，完整Adaptive成功/L4/资源/正式资格仍未完成，R6未结案。

**准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 完整数值Reader通过只读源fd捕获匿名封存副本，校验/摘要/解码使用同一字节；Analyzer从实际数组传递已有JSON摘要字段，合成未知来源保持空。新增30种named/compatible快照场景及metadata例，扩展582固定SHA、历史Bias与有限分析摘要核验，登记48项待远端检查点后构建/CTest。详见 [范围与准备](reviews/R6_MAT_SNAPSHOT_2026-11-17.md)，不改原MAT/数值策略或正式资格，R6未结案。

## 最新接续：R6 失败诊断的RunId（2026-11-17）

**实测 `[HUMAN-EXECUTE/APPROVE]`：`9eb831531ce04f2bfab8bf813011c36a451ee409` 无诊断构建、47/47 CTest（MAT5/5，其余42/42）。** 8次装配调用/63项检查、独立RunContext和真实Adaptive失败身份断言通过；stderr与Manifest保留完整错误链及真实RunId。三个Adaptive仍因既有数值边界Incomplete且无MAT，原始输入SHA不变。RunId `r6-runid-diagnostics-20261117-ccfLwnss`，见 [实测记录](reviews/R6_RUNID_DIAGNOSTICS_2026-11-17.md)。继续45源当前映射与MAT输入稳定摘要等独立工作；R6未结案。

**准备 `[HUMAN-EXECUTE/APPROVE]`，未执行本轮修改。** 真实Adaptive失败日志缺少已分配RunId，现补生产Runner、CLI、RunContext与Manifest序列化的既有身份字段；保留内层错误链和已有身份，不伪造未预留RunId。新增8次装配调用的故障合同测试，扩展真实Adaptive CLI及库层断言，CTest登记47项待远端检查点后运行，见 [缺陷/修复记录](reviews/R6_RUNID_DIAGNOSTICS_2026-11-17.md)。R6未结案，原数值边界与完整科学成功缺口不改变。

## 最新接续：R6 Adaptive真实CLI（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`2438a8039a905c7f3d8ab74aba1faeaa9a2387cf` 无诊断构建、46/46 CTest（MAT5/5，其余41/41）。** 6次CLI及1次真实Step诊断通过；初始化第14帧独立标量cosine为1+2^-52，38次真实求解/最后输入按位重放确认现有acos越界拒绝。比较/更新率第151帧零旋转拒绝，三者都只发布Incomplete审计、没有MAT；**通过的是错误合同，不是科学结果成功**。原始SHA不变，RunId `r6-adaptive-cli-trace-20261117-Ks76fbQ0`，见 [记录](reviews/R6_ADAPTIVE_CLI_2026-11-17.md)。继续源码/追踪及可独立证据复核；完整Adaptive成功、L4/资源/正式数值验收仍未完成，R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`3e39a14` 无诊断构建、45/46 CTest通过。** 比较/更新率实际进程均在第151帧按合同拒绝零旋转角度，初始化则第14帧因acos参数越界提前失败（12有效帧）；三者均仅写Incomplete审计，无MAT。新测试的初始化成功预期错误，现追加真实Step记录与独立标量边界复核，并严格核对实际失败前缀；生产公式/阈值不变。日志和原始SHA保留，见 [记录](reviews/R6_ADAPTIVE_CLI_2026-11-17.md)，待新检查点复测，R6未结案。

**首轮准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 新增三个实际生成入口各一次的CTest装配/求解/发布回归，保留35/600帧、八次分母及原错误处理；初始化产物另做独立MAT逐位复核与历史L2诊断。46项仅登记，待远端检查点后构建/CTest，见 [验证记录](reviews/R6_ADAPTIVE_CLI_2026-11-17.md)。不把替身预期当真实结果、不改变数值阈值，不运行L4/完整科学复现；R6未结案。

## 最新接续：R6 三份插图（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`8c808be58e25de16da7b3d0c66ce3c0764e74e76` 无诊断构建、45/45 CTest通过（MAT5/5，其余40/40）。** 插图536225项检查、真实CLI28次调用通过；原始SHA不变。实际SVG目视确认径向面不再裁剪、两轨迹图不变，首轮图像/日志保留；RunId `r6-illustrations-fit-20261117-mxoyaHiL`，见 [验证记录](reviews/R6_ILLUSTRATIONS_2026-11-17.md)。三份展示入口已实现；下一项为Adaptive真实生成CLI剩余覆盖，完整科学/L4、病态数值资格、资源及正式验收仍未完成，R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`80a1ce2` 无诊断构建、45/45 CTest通过。** 独立几何492946项、真实CLI28次调用通过，原始SHA不变；但目视发现径向面上下裁剪，正在调整SVG基础画幅并补全部投影面片边界测试。源缩放1.5、几何与阈值不变，首轮图像/日志保留，见 [记录](reviews/R6_ILLUSTRATIONS_2026-11-17.md)。修正待新检查点复测，R6未结案。

**首轮准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 三份原插图接入纯数值JSON与可选SVG，保留600点轨迹、z-only球面遮罩及未归一化勒让德径向面；新增独立Rodrigues全网格和真实CLI两项CTest，登记45项不等于通过。候选CLI/schema未冻结，原始资料只读、端点/灯光差异显式记录，见 [专项记录](reviews/R6_ILLUSTRATIONS_2026-11-17.md)。待远程检查点后构建/CTest；Adaptive真实生成CLI、完整科学/L4、数值/资源资格与正式验收仍未完成，R6未结案。

## 最新接续：R6 历史Bias全曲线与平滑来源（2026-11-17）

**实测 `[HUMAN-EXECUTE/APPROVE]`：`d68422cddc5de1b40baed5c1b5c57de5e4402074` 无诊断构建、43/43 CTest（MAT5/5，其余38/38）。** 历史Bias3323项通过，400文件/10400值→26曲线/208对x-y与独立宽精度参考最大绝对差约2.64e-14；平滑来源JSON/SVG及原582文件测试均通过。RunId `r6-bias-archive-20261117-0q1aV120`，原始SHA不变，见 [验证记录](reviews/R6_BIAS_ARCHIVE_ANALYSIS_2026-11-17.md)。下一项为三份illustration及Adaptive真实生成CLI的剩余覆盖；完整科学/L4、数值/资源资格和正式验收仍未完成，R6未结案。

**首轮准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 新增两组40-run历史Bias的独立MAT标签/宽精度均值参考，覆盖全部26曲线/208对x-y及400份实际输入；将既有LLD的平滑辅助来源字段落实到曲线/JSON/SVG并补相关测试。43项CTest仅登记，待远程检查点后实测，见 [验证记录](reviews/R6_BIAS_ARCHIVE_ANALYSIS_2026-11-17.md)。原始资料只读，不运行完整科学实验、不改变正式数值阈值或冻结状态。

## 最新接续：R6 Session / RunContext 交替隔离（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`b05a28b42a9e499f2c67c91af67a1c482f2f730e` 无诊断构建、42/42 CTest（MAT5/5，其余37/37）。** Session9579项、RunContext282项和误差703项检查均通过，后者新增acos双端舍入越界拒绝；所有输入SHA不变。RunId `r6-interleaving-acos-20261117-MqJnnIm2`，完整环境/命令/日志/合成输出位置见 [验证记录](reviews/R6_INTERLEAVING_2026-11-17.md)。下一项为历史Bias曲线独立重算、平滑来源及插图入口；完整科学/L4、跨环境数值资格和正式验收仍未完成，R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`412f6aba63ee08b5390604008b01728819ea2300` 无诊断构建、42/42 CTest（MAT5/5，其余37/37）。** 真实三模式Session的9579项检查、252成功/12拒绝与两个RunContext的282项检查、96个RNG样本/2540个MAT回读值全部通过；原始输入SHA不变，见 [验证记录](reviews/R6_INTERLEAVING_2026-11-17.md)。另补固定`(1,1,1)`正/反方向的acos舍入越界负例，生产公式/阈值不变，待新远程检查点复测。交替隔离不是同实例多线程资格，完整科学/L4与正式验收未完成；R6未结案。

**首轮准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 新增真实三模式Session的独立/交替重放、失败回滚与不同步帧时钟；两个RunContext使用独立RNG和真实MAT/Manifest，检查一方写失败不污染另一方。42项CTest仅登记，待远程检查点后构建/测试，见 [验证记录](reviews/R6_INTERLEAVING_2026-11-17.md)。合成合同数据不代表科学实验，同进程交替不代表同实例多线程资格；R6未结案。

## 最新接续：R6 Adaptive conditioning 参考（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`07f37e2b4db3e34512715c69a805342f4e6e8add` 无诊断构建、40/40 CTest（MAT5/5，其余35/35）。** conditioning1077项通过：十二个内部运动阶段独立参考、十二次完整投影拒绝及四早期负例，未改生产门限。RunId `r6-conditioning-final-20261117-9wu1dzWw`，原始SHA不变、失败保留，见 [验证记录](reviews/R6_ADAPTIVE_CONDITIONING_2026-11-17.md)。窄锥h=0.001前向误差约4.16e-6只作诊断，不代表正式数值通过。下一项为交替Session/RunContext隔离；acos越界、其余分析/插图、完整科学/L4与正式验收仍未完成，R6未结案。

**第二轮 `[HUMAN-EXECUTE/APPROVE]`：`7bbf8df` 无诊断构建、40/40 CTest；conditioning1077项通过。** 十二个运动阶段有独立参考，公共整步十二例仍拒绝零范数投影；四早期负例正确。h=0.001窄锥前向误差约4.16e-6仅作诊断，不当正式数值通过。静态检查另为内部未拟合系数补显式零占位，避免移动未初始化成员，待新检查点复测；见 [记录](reviews/R6_ADAPTIVE_CONDITIONING_2026-11-17.md)。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`3ef34a2` 无诊断构建、39/40 CTest。** 十二个运动系统可解场景在最终SH投影因零通道范数拒绝，四个早期负例符合预期；测试错误地期待整步成功。现提取内部运动阶段以核对独立参考，同时保留公共整步十二例拒绝与完整错误链断言；源公式/门限不变、失败日志与原始SHA保留，见 [诊断记录](reviews/R6_ADAPTIVE_CONDITIONING_2026-11-17.md)。待新检查点复测。

**首轮准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 接入既有独立均值/条件数参考族，12个正常输入及4个定义域负例；保留完整矩阵/RHS与实际解诊断，不改生产门限。40项CTest仅登记，待远程检查点后构建/测试，见 [验证记录](reviews/R6_ADAPTIVE_CONDITIONING_2026-11-17.md)。窄锥前向误差仅诊断，不冒充正式数值资格；R6未结案。

## 最新接续：R6 误差函数与 KvD 停止分支（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`bcee2ca02ef100b569da3ee2a54137b6875bb0af` 无诊断构建、39/39 CTest（MAT5/5，其余34/34）。** 误差函数685项、KvD75080项全部通过；506次真实KvD停止轮数/继续标志与独立解析降维参考一致，两算法均覆盖默认200轮上限、两类独立门限退出及最少6轮。RunId `r6-motion-stopping-fix-20261117-sQGrT7nI`，首轮失败保留、原始SHA不变，见 [验证记录](reviews/R6_MOTION_STOPPING_2026-11-17.md)。下一项为conditioning及交替Session/RunContext隔离，其余分析/插图、完整科学/L4与正式验收仍未完成；R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`54e3ac1` 无诊断构建、38/39 CTest。** 误差函数503项通过；500次KvD求解的独立停止轮数/继续标志一致，唯一失败为Modified缺单动态门限场景。现保留全部断言/失败日志，补六次带固定切向扰动的解析参考；待新检查点复测。原始SHA不变，见 [验证记录](reviews/R6_MOTION_STOPPING_2026-11-17.md)。

**首轮准备 `[HUMAN-EXECUTE/APPROVE]`，尚未执行。** 提取内部原始误差函数并直接测解析几何/异常边界；新增独立标量降维参考，检查两种 KvD 的默认停止条件。39 项 CTest 仅登记，待远程检查点后真实构建与测试；见 [验证记录](reviews/R6_MOTION_STOPPING_2026-11-17.md)。不改变算法门限或正式验收义务，R6 未结案。

## 最新接续：R6 原脚本绘图视窗（2026-11-17）

**最终补充复测 `[HUMAN-EXECUTE/APPROVE]`：`65d59fe2640aa59768d4bb86386886f1692874d6` 无诊断构建、37/37 CTest（MAT5/5，其余32/32）。** 六个有效SVG情境与14个直接Exporter负例共103项检查全部通过：合法全零图成功，empty/NaN/Inf明确InvalidSeries且零发布，输入不变。最终RunId `r6-svg-guard-20261117-zp9qajzR`，见 [视窗记录](reviews/R6_SVG_WINDOWS_2026-11-17.md)；原始SHA不变，未改变阈值/CLI/公共schema。后续缺口见 [66 TC映射](reviews/R6_TC_EVIDENCE_MAP_2026-11-17.md)，R6仍未结案。

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`7fc83c1a828b84318b382ad2e052ba735e489ec4` 无诊断构建、37/37 CTest（MAT5/5，其余32/32）。** 恢复Bias固定x/y窗口、Comparison固定右轴及SVG裁剪；全零Bias可出空图，分析数据不变。六情境61项SVG几何检查通过，原始SHA不变，失败图像/日志保留，见 [视窗证据](reviews/R6_SVG_WINDOWS_2026-11-17.md)。另新增 [66 TC实际证据映射](reviews/R6_TC_EVIDENCE_MAP_2026-11-17.md)，未更改既有验收义务。下一项为直接误差函数边界、KvD迭代上限/退出分支、Adaptive conditioning及交替Session/RunContext隔离；插图/其余来源与L4、完整科学实验和正式验收仍未完成，R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`**：`80ae1acf7caf083fa97aeff76f4f228a56f6dc03` 无诊断编译、36/37 CTest；新SVG几何测试60项断言中25项失败，确认Bias固定x/y范围、Comparison固定右轴及viewport裁剪缺失，全零Bias被错误拒绝。现修复Exporter显示范围/clipPath，待新检查点复测，见 [视窗验证](reviews/R6_SVG_WINDOWS_2026-11-17.md)。并核对既有66 TC的真实测试对应关系，R6不以测试数量替代完整验收。

## 最新接续：R6 有限MAT与非线性分析（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`cecd19ada9e162701992960b0b7cb076e0deed3e` 构建无诊断，36/36 CTest通过（MAT5/5，其余31/31）。** Comparison/UpdateRates有限输入的32条曲线、5578对x/y、3460个完整平滑值与独立三次多项式参考精确一致；修复了UpdateRates纵轴，并验证4份SVG的轴标记/结构。原始及合成MAT摘要不变，首轮失败证据保留，见 [有限输入验证](reviews/R6_ANALYSIS_FINITE_VALIDATION_2026-11-17.md)。下一项为66 TC实际证据映射及源绘图视窗/三份illustration缺口核对；原始NaN、默认轨迹UndefinedAngle、剩余L3/L4与完整科学实验/正式验收仍未闭合，R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`**：`29e965c4e234dabe7cc40e55c51734ae559b2032` 构建无诊断，35/36 CTest。独立多项式参考确认两个有限MAT入口的32曲线/5578对x-y/3460个完整平滑值一致，但UpdateRates20条曲线的y_scale错误为log。已保留失败，现修复为原脚本linear并加SVG轴标记检查，待新远程检查点复测。范围、推导、固定边界及R6-ANALYSIS-001证据见 [有限输入验证](reviews/R6_ANALYSIS_FINITE_VALIDATION_2026-11-17.md)。原始数据只读，R6未结案。

## 最新接续：R6 真实CLI与历史分析集成（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`c42f12f0be89646fd39b93dbb61a79a0ee28e05f` 构建无诊断，35/35 CTest通过。** 先MAT5/5，再其余30/30；新增实际CLI合同42次进程、六组历史输入15次进程。两种Bias及Initialization输出28条曲线/3份SVG，另外三组因独立确认的原始NaN按合同拒绝且不发布图像；该负例通过不代表科学结果成功。原始输入SHA不变，完整证据见 [CLI验证](reviews/R6_CLI_PROCESS_VALIDATION_2026-11-17.md)。下一项为有限合成MAT下Comparison/UpdateRates真实CLI成功路径及非线性独立平滑参考；原始NaN限制、其余L3/L4、完整实验与正式验收仍待处理。

**独立归档事实 `[HUMAN-EXECUTE/APPROVE]`：`8fad292` 构建无诊断，34/35。** 独立MAT字节解码确认历史proportional末轮xwerte为128…2097152，与当前生成脚本不同；7个旋转误差文件共26个NaN、0个Inf。六组历史输入均实际执行：两种Bias及Initialization成功，三个600帧Adaptive目录analyze/plot均按既有合同拒绝非有限值。现仅纠正测试的输入事实预期，继续严格验证失败且不生成伪造曲线；原文件/阈值/生产分析逻辑不变，待新检查点复测。

**第二轮 `[HUMAN-EXECUTE/APPROVE]`：`3986ed5` 无诊断构建，34/35 CTest。** 帮助及真实CLI的42次调用通过；历史proportional横轴与当前生成公式不同，无噪声Adaptive被报告含非有限值。正在通过独立MAT解码记录原值/无定义位置，并补齐其余归档实测；不改变原文件或非有限拒绝规则。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`3843a81` 无诊断构建，33/35 CTest。** 真实CLI的41次进程调用确认八个子命令帮助错误退出2；其余Bias发布/诊断通过。历史分析测试因新CMake脚本的变量名冲突在业务调用前退出，不能归因于Analyzer。现分别修复解析器与测试分派，原断言保留、待新检查点复测；证据见 [CLI验证](reviews/R6_CLI_PROCESS_VALIDATION_2026-11-17.md)。

**[HUMAN-EXECUTE/APPROVE] 新增两项CTest，尚未执行。** 固定构建SHA的实际 `egomotion` 进程覆盖解析/错误诊断、两种有界Bias和六组历史结果JSON/SVG；35项登记不能继承33/33通过。已核实子命令帮助的源码疑点，准备保存首轮实测；详细边界及后续证据见 [CLI验证](reviews/R6_CLI_PROCESS_VALIDATION_2026-11-17.md)。

## 最新接续：R6 Bias Runner编排与发布（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`c2187539b8cb16fe9d53e770d231a48c2451c0b5` 构建无诊断，33/33 CTest通过。** 先MAT5/5、再其余28/28；两种Bias真实level0..2编排17695项、发布5558项全部通过，R6-BIAS-001修复与复测闭合。RunId `r6-bias-fix-20261117-ZRqFNqCL`，原始输入SHA未变；完整命令/环境/时间/日志哈希见 [Bias验证](reviews/R6_BIAS_RUNNER_VALIDATION_2026-11-17.md)。此证据不替代40份level9或完整Adaptive/L4实验。下一具体覆盖缺口为真实CLI装配及历史MAT→Analyzer→JSON/SVG；其余L3/资源资格和正式批准仍待完成，R6未结案。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`ff18a3a` 无诊断构建，32/33 CTest。** 两种Bias真实七MAT发布5558项通过；编排17693项仅2个外层分类失败，定位R6-BIAS-001。流生成/RNG误报SolverFailed，现按既有合同区分IncompleteRun；原数值与断言不变，待新检查点复测。原输入SHA不变，失败日志见 [Bias验证](reviews/R6_BIAS_RUNNER_VALIDATION_2026-11-17.md)。

**首轮准备（历史） `[HUMAN-EXECUTE/APPROVE]`：新增两项CTest，当时尚未运行。** 固定seed7/11和原合法level0..2，验证真实Bias全链的采样/噪声/RNG重置、六指标、七MAT发布/回读及失败状态，不替代完整40份论文实验。当时33项仅登记；31/31属于上批固定SHA。实际复测以本节首段为准。

## 最新接续：R6 原球谐动作与Session集成（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`632139bb0d9dd0d3afa7a11afdb0578224a47f59` 构建无诊断，31/31 CTest通过。** 先MAT5/5、再其余26/26；新源动作适配4886项、Session10006项检查均通过，最大绝对误差4.44e-16/6.66e-16，保留旧物理合同并恢复业务原系数动作。R6-SH-001修复/复测闭合；RunId `r6-sh-source-fix-20261117-IPXj1uHL`，原始输入SHA不变，完整证据见 [源动作验证](reviews/R6_SH_SOURCE_ACTION_2026-11-17.md)。本批补足限定源相位/动作与集成证据，剩余独立L3、完整真实Runner/L4、资源边界及正式验收仍未完成；默认第151帧UndefinedAngle未消失。

**首轮实测 `[HUMAN-EXECUTE/APPROVE]`：`2d271ee` 构建成功但新替身Trace有未初始化警告；30/31 CTest通过。** 独立相位参考3302项通过，Session源动作10006项中1667失败，定位R6-SH-001：物理场适配器不能直接替代原 `transfor_koeff`。现保留物理叶子，增加命名明确的源系数适配并接入Session，初始化替身Trace；尚待新检查点后复测。失败记录见 [球谐源动作验证](reviews/R6_SH_SOURCE_ACTION_2026-11-17.md)。

**[HUMAN-EXECUTE/APPROVE] 两项新CTest已编写，尚未执行。** 起点 `6fcc010` 已核实，31项登记不能冒充通过。以既有独立IR96显式表/源九基多项式核对相位、R0/R3/R5和一般旋转，再检查真实Session非交换跨帧状态与回滚；保持物理叶子合同，不以绕z特例代表原组合完全等价。范围及后续证据见 [球谐源动作验证](reviews/R6_SH_SOURCE_ACTION_2026-11-17.md)。

## 最新接续：R6 MAT内部载荷边界（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：`bc51fddca2a56d0dae988086824266b68ff995c2` 构建无诊断，29/29 CTest通过。** MAT先5/5、其余24/24；94个raw载荷场景全部通过、旧29个Reader边界及582 MAT/33,340值精确对照仍通过。生产增加解码前固定缓冲校验，修复静默接受非法存储/长度及错误分类；独立查明旧char夹具根长度高估并保留对应损坏负例。RunId `r6-mat-payload-final-20261117-Rkq8pvHK`，输入SHA不变，失败与成功记录见 [MAT载荷验证](reviews/R6_MAT_PAYLOAD_VALIDATION_2026-11-17.md)。本地R6-MAT-002已有复测证据；原算法/原始数据不变，R6仍需球谐源动作、剩余L3/L4、完整Runner与正式验收。

**首轮实测**：`1b1349d` 构建无诊断，MAT5项中4通过、内部载荷1失败；新70场景有48失败，其中34个非法载荷被接受，14个错误分类不符。R6-MAT-002已保存日志、检索Issue并定位到解码前内部边界检查缺失，现以固定缓冲流式校验修复，并增补具名选择/跨块和matio调用前拒绝计数，90场景待复测；原始582 MAT仍通过且输入SHA不变。

**[HUMAN-EXECUTE/APPROVE] 新增70场景原始字节夹具与1项CTest，尚未执行。** 当前登记29项，既有28/28仅对应 `a5336b3`。将独立核对MAT内部实部长度/dims/溢出及single存储，不以matio返回的内存dtype冒充物理tag验证；子进程资源隔离、所有文件在build新RunId，原始数据只读。范围与后续失败证据见 [MAT载荷验证](reviews/R6_MAT_PAYLOAD_VALIDATION_2026-11-17.md)。

## 最新接续：R6 Adaptive Runner 跨帧合同（2026-11-17）

**最终复测 `[HUMAN-EXECUTE/APPROVE]`：源码 `a5336b3e9072be16dcf296b927a24de3e39cdc06` 构建无诊断，28/28 CTest通过。** 先MAT4/4，再其余24/24，原始输入SHA不变；RunId `r6-adaptive-errors-20261117-BxUi6ya8`。新增固定8组600行聚合/真实发布、完整600帧合成真值Session、各缺样本前缀及默认轨迹无定义失败；修复undefined漏计及噪声失败外层分类。**默认Comparison/UpdateRates在当前替身下第151帧中止仍存在，不把负例通过或固定结果当端到端科学成功。** 完整修改、命令/环境/日志哈希与剩余范围见 [Runner合同验证](reviews/R6_ADAPTIVE_RUNNER_CONTRACT_2026-11-17.md)。下面的25/27与构建失败是保留的历史证据；R6未结案。

**接续诊断 `[HUMAN-EXECUTE/APPROVE]`**：`cc2f668` 第二轮构建无警告、仍25/27。两个默认正弦轨迹模式在第151帧遇到零旋转真值，按既有UndefinedAngle合同中止；初始化和RNG负例通过。现新增独立默认路径负例，按已设计的8组600行固定结果检验原生产聚合/发布函数，并以非零合成真值核对完整600帧Session；不修改轨迹、阈值、帧数或分母，不冒充科学实验。修复漏记undefined计数，28项待远程检查点后验证。详见 [Runner合同验证](reviews/R6_ADAPTIVE_RUNNER_CONTRACT_2026-11-17.md)。

**[HUMAN-EXECUTE/APPROVE] 首次27项中25通过、2失败，待修正复测。** `e77eb402` 构建成功但替身Trace有未初始化警告；回执检查错误假定枚举顺序，已改为变量身份核对。第八轮故障用例在预定注入前遇到SolverFailed，原因待更完整诊断；不放宽任何断言或科学阈值。保持35/600帧、级别2/4及8次重复，真实Writer输出与失败记录均保留；不将替身称为科学实验。日志、修改和边界见 [Runner合同验证](reviews/R6_ADAPTIVE_RUNNER_CONTRACT_2026-11-17.md)。

## 最新接续：R6 Reader 类型与损坏负例（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 已修复并通过25/25 CTest，受测源码 `dffddf3b6a62b906a5b20eba365a2087e9c86662`。** 首次 `05a46c6` 的29个Reader边界场景中3失败：普通/压缩截断及zlib校验和损坏被错误接受。新增顶层长度/完整zlib校验并固定源fd，保持metadata-only路径和原失败断言；复测29/29场景通过，原始582 MAT/33,340值独立对照及其余21项全部通过。RunId `r6-reader-fix-20261117-USlbrReS`，原始输入哈希不变、编译无诊断。失败和成功证据均保留，本地缺陷R6-MAT-001的诊断/修复/复测见 [Reader边界记录](reviews/R6_READER_BOUNDARIES_2026-11-17.md)。R6尚有raw tag、完整Runner、剩余L3/L4与正式验收，未结案。

## 最新接续：R6 原始存储标签及独立 L2 Reader（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 源码 `68a5f684cc9fb45ee33a7b87b70525ac0510c8fa` 已编译，24/24 CTest通过。** 独立C++/zlib解析582个MAT的33,340个数值，与生产Reader的列主序binary64值逐位一致，全部固定SHA匹配。实际实部标签为502 miDOUBLE/80 miINT32（Bias全部xwerte），均为double class，纠正旧报告将matio解码dtype当物理dtype的事实错误。运行前检查点已推送并核实；RunId `r6-mat-storage-20261117-83tbiC2N`，先MAT3/3再其余21/21，原始输入哈希不变，无编译诊断。不更改数值阈值或REQ/AC。执行证据、候选参考身份及未覆盖的严格Reader负例见 [存储验证](reviews/R6_MAT_STORAGE_VALIDATION_2026-11-17.md)。R6未结案。

## 最新接续：R6 Linux 发布故障测试（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 已验证源码 `1e15b9c32342c96e2e553325a8870c0a276b31a4`：构建及23/23 CTest通过。** 新增仅测试链接的系统调用故障包装，直接验证真实 MAT/Text Writer 的23个场景及真实 RunContext/Manifest 的6个场景；覆盖指定 fd/阶段的同步、发布竞争和暂存清理失败，并检查精确调用次数、最终 inode/字节、未确认回执及独立错误链。使用构建目录内中文/空格合成沙箱，不写原始数据。复用缓存重新配置、无编译诊断，MAT2/2及其余21/21均通过，输入哈希不变。RunId `r6-io-faults-20261117-sm6Ovszn`；完整证据与剩余边界见 [发布故障验证](reviews/R6_IO_FAULT_VALIDATION_2026-11-17.md)。R6仍未结案。

## 最新执行批次：R6 原生验证（2026-11-17）

**[HUMAN-EXECUTE/APPROVE] 本轮已执行：干净全工程构建成功，21/21 CTest 通过。** 已验证源码 SHA 为 `84c86624954406b31e8a82d5e145c7f6ae30ee93`，RunId 为 `r6-20261117-KzjFHPDQ`。用户直接允许生产代码修改、构建/CTest 和方案文档修改，覆盖本页历史批次的 H-11 暂缓。起点工作区/远端为 `5fa2e65b09dcfad15a0e467383e14c185bf0e540`，全状态 Issue/PR 查询为空；沿用专用分支 `dev-plan-P0-P1-mat-dictionary-probe`。不改写历史审批或代签验收。

首个修复是 `RunMatWriter` 对 matio 1.5.26 可写 `void*` 形参的适配：保留公共 const 输入，复制局部可写缓冲区并继续使用 matio 复制所有权；现有 `r05_anchored_io` 增加真正 const 输入及成功/重复写/非法路径调用后元数据与数值不变的回归检查。预计额外一份 double 数组内存，未改变数值精度、类型、列主序或阈值。

实际执行顺序：每次提交/推送/核实检查点后配置及编译，先运行 MAT Reader 相关测试，再运行其余登记 CTest；保存环境、输入身份、命令、时间、退出码及分离的 stdout/stderr。最后一次使用全新空构建目录，全部步骤 exit 0。原始 `origin/**` 只读，合成 MAT 仅写测试沙箱；全部 628 个受控原始文件与本轮最初 SHA-256 一致。R6 的 L2/L3/L4 科学验收及 DEC-005/006 仍单独保留。

`r05_manifest_gate` 同时补上有状态 Publisher 替身：名称发布后不再允许覆盖，模拟已链接 `manifest.json` 但持久化未确认，验证没有成功 RunOutcome/内存 Complete，随后失败记录的 `OutputExists` 与原始 `ReadbackMismatch` 分别保留，已发布字节不被改写。此测试只覆盖 RunContext 合同，不能冒充真实 Linux fsync/unlink 故障注入或独立读取磁盘 Manifest 的最终资格协议。

**失败修复与复测**：前三轮修复静态断言拼写、Bias Runner 声明依赖、Reader 测试注入对象 const 不匹配及编译警告。第四轮 MAT CTest 失败已定位为元数据字段错误假设，保留真实未知字段并在完整解码结果上校验 double/实数/non-logical/字节数。第五轮在 Ubuntu 24.04.5、GCC 16.2.0、CMake 4.3.4、Eigen 3.4.0、Sophus 1.24.6、matio 1.5.26、zlib 1.3 上无诊断构建成功；MAT 2/2（0.52 秒）、其余19/19（0.44 秒）全部通过。582 MAT 均读取，未减少样本或改变数值门限。全部成功/失败日志、哈希、命令、实际源码文件/提交及未覆盖范围见 [原生验证记录](reviews/R6_NATIVE_VALIDATION_2026-11-17.md)。**这是本轮工程验证完成，不是 R6 科学验收结案。**

> 滚动证据登记 v0.6（截至E-DOC-069，2026-11-10），非正式基线批准。**这里的“本轮”是E-DOC-069历史数学批次，不是2026-11-11正在开展的全工程文档规范审计。** 该历史批次仅作理论推导和Wolfram验证，未实施生产C++；历史原生SH测试与数学运算的执行者、源码SHA及证据类型必须分开。本次文档审计最新状态见[AUDIT_PROGRESS.md](AUDIT_PROGRESS.md)，不得用本页旧97AF计数替代现行全工程审计的DOC/DAR口径；现行数字及延期分类只以[AUDIT_PROGRESS.md](AUDIT_PROGRESS.md)最新台账为准，历史9DAR属于旧批次而非当前总数。
>
> 为消除旧NOT_RUN、旧授权与动态计数反复堆叠，既有E-DOC-054～067详细记录完整保存在[9825375固定PROGRESS](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/982537556be1ac5a414af368a0082806a9179d32/docs/PROGRESS.md)。更早E-DATA-001、E-DOC-001～053保存在[98e95e2固定历史](https://github.com/ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields/blob/98e95e2f867dc7142fa0d5ac0f1c79f5d6429ed1/docs/PROGRESS.md)。未改写任何Git历史或历史审批；下表保留定位和关键事实，E-DOC-068为上一轮、最新证据为§5的E-DOC-069。

## 0. 2026-11-15 R0/R1 新编程批次（尚无原生运行权限）

**新授权**：项目所有者已解锁H-10的全工程C++代码编写/提交权限（参见[TODO](TODO.md)和[ROADMAP](ROADMAP.md)）；H-11仍单独延期，**本次只编写/提交代码、CMake和测试源码，不进行CMake配置、编译、CTest或业务CLI运行**。以下§1～6是原E-DOC-069历史数学批次记录，不随本批次追溯改写。

**本批次交付**：R0解析`inspect-mat`、2种Bias、3种Adaptive、`analyze`和可选`plot`命令，校验必需/重复/未知参数、uint32/level/有限噪声及安全RunId；现无真实后端时返回有明确消息的失败而非伪造Complete。R1提供C++17的Eigen自拥有值类型、Result/Status（禁默认构造、明确逻辑错误）、`std::forward_list<ErrorFrame>`完整包装链与JSONL、Reader/Writer/RNG、Runner和Analyzer抽象接口。根CMake保留原SH静态库/3测试，同时增加CLI程序、公共接口与7个INTERFACE占位模块、2个新增合同测试源码，未实际执行测试。未实现matio生产Reader/Writer、采样/KvD/Adaptive、真实RunContext/Manifest及结果写入，这些属于R2～R5。

**静态CMake检查**：源文件路径存在、11模块目标命名及依赖非环、旧3+新2测试对象均已登记；**不等于CMake configure/compile通过**。没有实际运行环境、工具链或日志的成功证据，不登记虚构PASS。详细批次与依赖以[ROADMAP](ROADMAP.md)为准。

## 0.1 R2 MAT Reader复用与旧探针重构（2026-11-15）

**已编写/已推送，不是已编译/已运行**：`src/mat_data_dict_probe.cpp`仅解析旧位置参数和`--expected-count/--source-sha/--replace-generated`并返回诊断/退出码，不再调用`Mat_Open/Mat_GetDir/Mat_VarRead`；`src/io/mat_data_dictionary.cpp`负责遍历输入、FNV64、旧格式Markdown报告和生成文件保护；`src/io/mat_result_reader.cpp`统一负责matio RAII及两条输入能力：`inspect`用`Mat_VarReadInfo`**仅读结构**，`readNamed/readFirstCompatible`用`Mat_VarRead`**完整解码、列主序拷贝到自拥有double数组**。字典扫描复用同一Reader的`VerifyDecoded`模式，仍在单个Mat_Open句柄内逐变量验证读取状态。公共`IResultReader`无matio类型；现有`egomotion inspect-mat`经Composition Root装配真实Reader，不回读或展示数值。

**工程与测试源码**：CMake的`egomotion_matio`由空INTERFACE改为实际STATIC，私有链接`PkgConfig::MATIO`与`ZLIB::ZLIB`；新`mat_data_dict_probe`目标以及`r02_mat_reader`只读原MAT夹具测试已登记，保留既有5项CTest。历史手动`g++ src/mat_data_dict_probe.cpp`**单文件编译已不适用**；需编译/链接三个源文件（含`src/io/`）和Eigen/matio/zlib，或通过CMake构建。详细命令见[ROADMAP R2](ROADMAP.md)。

**未运行的证据与限制**：H-11仍延期，本轮没有执行`cmake`、`g++`、CTest、旧582文件数据字典重建、独立L2逐值/SHA-256或任何MAT写入；当前原582/582扫描结果属于旧探针历史版本，不能自动继承为本次新代码PASS。CMake静态检查涵盖源文件存在、target命名及非循环边，不能保证编译/链接；正/负例和元数据压缩头行为需真实运行复核。**R2代码就绪待原生验证，R3尚未开始。**


## 0.2 最新顺序决定与R2代码闭合、R3源码推进（2026-11-15）

**所有代码先行**：用户本轮明确要求[ROADMAP](ROADMAP.md)把构建、测试、数值运行、测试源码收尾和验收全部集中R6。R0–R5只做**代码和静态设计复核**，不得以每阶段需先编译/CTest为前置；H-11仍没有原生运行及MAT写入授权。旧章节中“先构建/验证再做R3”的文字只反映形成时的历史建议，本段及ROADMAP的新顺序优先。

**R2代码复核与实修**：保留CLI分离、Reader单MAT句柄、结构`Mat_VarReadInfo`与数值`Mat_VarRead`独立职责；补上`readNamed`检测同名重复变量返回`AmbiguousVariable`及数值读取只接受MAT v5的拒绝门禁（结构元数据枚举仍可报告不同版本）。R2代码范围当前没有已知阻止继续实现R3的未处理缺陷；**这是静态源码判断，不是R2数值等价/原生通过**。

**R3已提交的计算源码**：
- `src/sampling/`：按源面表和A/B/C/D深度优先产生完整/缺损八面体与二十面体视线；`getvektor`两次uniform；独立`Mt19937RandomSource`支持seed重置。标准库normal分布**不承诺与MATLAB同seed逐样本一致**。
- `src/simulation/flow.cpp`：近度/理想流与分量平均、两种互斥三维噪声/切平面投影，严格N次uniform后3N次normal，零噪声系数仍消耗样本；结果保存理想/含噪流、全部近度和末次源标量，处理非有限/非法方向/内存失败。
- `src/estimators/franz_kvd.cpp`：Franz三轴响应/分母、Original与Modified KvD独立命名入口；near负和翻号、旧t算旋转、一次t归一、至少6/通常最多200次及终局mean(near)缩放，提供可选逐轮trace。
- `src/experiments/bias_runner.cpp`：通过构造注入**一组**采样器/噪声模型；每次Bias实验先生成truth t/ω再用同seed重置RNG，三档系数、0..level顺序、两种算法和六类`transfehler_div`归一误差保存至自拥有`xwerte/ywerte1..6`列主序内存，不触碰MAT文件。

**R3与R5的分界**：这一轮的`BiasExperimentRunner::compute`只返回内存`BiasRunData`，**没有**把新结果伪装为`RunOutcome::Complete`。CLI的`run-bias-*`到具体RunContext/Writer/Manifest装配与实际MAT保存由R5实现；本轮新C++文件还没有进入根CMake目标的编译源清单，按用户指令在**全部R0–R5代码完成后再于R6一次性接入并构建/测试**。当前不能报告R3/完整业务可运行或MAT结果正确；R4/R5未开始。

**执行证据**：所有新生产文件已提交、推送；本轮**没有**运行CMake、g++、CTest、程序、MATLAB、MAT读取/写入或任何数值/性能测试，也未修改`origin/**`。此前R2注册的只读CTest仍只是测试源码登记，不能算已通过。详细本轮Git提交见该分支提交历史。


## 0.3 R3完成静态源码缺口修复，进入R4实作（2026-11-15）

**范围判断**：按[ROADMAP](ROADMAP.md)定义，R3只要求**内存**Bias业务计算（不包含R5的持久化Writer、RunContext/Manifest及CLI完整装配）。从`ec0a3a74f9fe`复核后，对实际源码执行多次修正：

- `src/sampling/directions.cpp`/`src/simulation/flow.cpp`/`src/estimators/franz_kvd.cpp`：全局失败的`observation_index`不得默认伪造0；明确每个源uniform/normal失败的观测/分量与真实包装链，修正参数/资源超限分支并保留源MATLAB采样顺序。
- `generateBiasFlow`：距离抽样恢复源`entf=(rand*(4-1)+1)/2.5; near=1/entf`的浮点操作顺序，避免把代数等价的`2.5/(1+3*rand)`误当逐步骤复现；先N次uniform、再3N次normal不变。
- `BiasExperimentRunner`：明确`subdivision_level`和`noise_column_matlab_1_based`属于不同身份，不误用观测下标；失败带上`ywerte1..6`指标，零真值不记假0误差，资源分配失败保留完整数值根因。
- 仍由Composition Root持有/注入唯一相配采样器+噪声模型；R3源码范围已具备三种有序采样、两类互斥噪声、Franz、Original/Modified KvD以及两类Bias的六组内存输出。**“R3源码范围闭合”只是静态检查判断，不是已成功编译/运行；如果R6发现实际错误仍须重新打开修复。**

**R4新增生产源码**（都已提交推送、尚未编译）：`include/egomotion/trajectory/trajectory.hpp`与`src/trajectory/{trajectory,kinematics,sphere}.cpp`实现circle/sinusoidal源N分母、有效中间帧`Rᵀ`局部运动、Sophus SO(3) exp接口和严格球内正根；对于原`drehvektor_aus_zwei_vektoren`精确反向会返回伪零的问题，新路径采取**显式InvalidDirection失败**并标识这是C++安全边界而非MATLAB原有处理。

`src/harmonics/adapter.cpp`重用已有l≤2叶子的3×5↔9系数转换、重建、原source逐通道二阶拟合及SH旋转，不复制球谐公式。公共系数旋转参数的R/Rᵀ约定**必须由未来Runner按原调用方明确选择**，不可反向猜测。

`src/estimators/adaptive_step.cpp`按原`oneit_adaptiv_matched_filter`顺序计算near_raw/s/翻号与归一、6×6**一般非对称**M1–M4及RHS、FullPivLU求解、t单位化、原返回MSE、1e−11新near与二阶系数拟合；不用错误的LDLT对称化。FullPivLU与源`real(inv(M))*b`的浮点差异和病态阈值需在R6另行核验，DEC-005不能先虚设PASS。

`src/estimators/adaptive_session.cpp`将Initialization、Comparison、UpdateRates的主状态/比较状态分离：UpdateRates按MATLAB **1-based** i周期1/5/11/21刷新，比较调用的`dumi`新系数不提交；仅当全部比较与主求解完成后提交主系数及持久基线，失败保留之前状态与错误链。**R4尚未完成三类Runner的完整场景生成、八次聚合及实际内存输出；R5的MAT写入/CLI也未接入。**

**执行约束**：本批严格**没有**调用CMake、编译器、CTest、Wolfram/数值实验、MATLAB或读写任何MAT；`origin/**`原始文件未改。R0–R5仍只编写实现，R6最后统一准备构建/测试源码及执行；本次没有新PASS或代码验收结论。


## 0.4 R4内存计算源码闭合复核与R5第一批输出合同（2026-11-15）

**本轮起点**：`0ea0f0324514d7e06691fe45a155c8e774e1d3b3`。先读实际`src/trajectory`、球谐适配、Adaptive Step/Session，再逐行核对三份原只读MATLAB的默认场景、循环、当前`save`与错误统计。R4最大缺口为**三类Adaptive Runner完全不存在**，已新增：
- `include/egomotion/experiments/adaptive_runner.hpp`和`src/experiments/adaptive_runner.cpp`：Initialization用35点circle(0.6,0.3)/level2/单次；Comparison用600点sinus(.5,1,2,.3)/level4/八个完整run，依次累积四类角度误差，**仅全部八次完成后固定除8**，理想流平移/旋转平均模长之比`rottransratio`只取最后一轮，`nonpropfehler`即使scale=0仍消耗3N normal；UpdateRates用600点/level2，四周期1/5/11/21，对照求解器返回的`dumi`仍丢弃，当前save只有两组600×4误差及600×1的`akrot`（deg），绝不补造HistoricalOnly字段。所有输出为自拥有列主序double矩阵，首尾按原MATLAB的zeros保持0且不计有效估计。
- `include/egomotion/estimators/adaptive_session.hpp`与`src/estimators/adaptive_session.cpp`：新增MATLAB从i=2开始的严格连续帧资格检查，成功提交时才推进下一帧；所有比较及主求解完成前，主系数/四份周期基线均不改变。每次R4`compute*`只有完整内存结果才能返回成功，不允许在失败alternative假装返回完整`Complete`。
- 再修复Comparison**移动理想流后读取分量均值**的生命周期瑕疵，把`mean_rot/mean_trans`提前取得；累积完一轮后才增加`completed_runs`，第八次之外不提交末次比值。

**本轮已转入R5代码**（非R6的测试实施）：
- `output_contract.cpp`按实验类型拒绝旧归档`HistoricalOnly`及跨入口错字段；在任何Writer调用前核对完整当前save集合、内部变量名、MAT double/shape/bytes、有限数值；
- `output_values.cpp`将R3的7个Bias矩阵和R4的4/5/3个Adaptive当前字段转换为自拥有公共领域值；Comparison必须保留**8/8**才具备发布资格；
- `run_context.cpp`独占RunId/RNG、持有CompositionRoot保障生命周期的Writer引用，保留全部错误链和独立文件确认记录。先准备当前save字段和确认列表容量，再逐个交给IResultWriter；部分写入只标内存`Incomplete`草稿，**不得产生假的正式Manifest/RunOutcome Complete**。真实安全文件发布、SHA校验、Manifest持久化和CLI全链接仍属R5未完成事项。

**残余资格与代码状态**：当前R4的三种在内存中的实验计算流程已按原入口结构静态闭合，但由于H-11未授权，本轮**没有**执行编译、CTest、真实MAT读写、MATLAB、Wolfram或数值实验，也未更改`CMakeLists.txt`。该判断不是原源与新物理球谐旋转作用逐值等价的声明（旧`transfor_koeff/SHRotate`按[球谐LLD](design/harmonics.md)存在明确相位/动作差异），亦不保证数值病态、异常和性能行为；R6必须独立限定验证，不通过应重开代码修正。真实CMake目标/测试源码收尾全部留R6，`origin/**`保持只读。

## 0.5 R5代码范围完成静态核查，继续R6测试源码（2026-11-16）

**本轮核查起点**：`0d3b6071844ac5799ded0fed8553d31e661cb40b`；依据真实文件与近十次R5/R6提交复核，而非沿用§0.4和ROADMAP中早期R5快照。核实R5的五种业务Runner全部连接到真实CLI Composition Root，MAT v5 Writer带Linux dirfd锚定、禁止覆盖、回读和SHA-256证据；RunContext具有全字段白名单与shape校验、Complete Manifest资格及独立Failed/Incomplete失败记录。四类只读Analyzer分别处理Bias、Adaptive Initialization、Comparison、UpdateRates，输出结构化JSON；SVG适配只消费数值曲线，不阻塞独立`analyze`。**R5在ROADMAP定义的“代码实现部分”静态闭合，尚不表示已编译或原生验收**。源码发现仍须在R6用真实测试核对的安全竞态、数据等价和异常行为，若失败则重新打开R5修复。

**R6本轮真实提交的源码工作（3个新测试文件，1次CMake登记）**：

- `tests/integration/test_r06_sampling_flow.cpp`：三类采样器level 0/1/2计数、单位视线和缺损八面体逐行保序、getvektor两次uniform、Bias内存近度与末项标量、零高斯3N正态抽样、NoNoise区分、理想流分量抵消与component proportional噪声、RNG耗尽错误链。
- `tests/contracts/test_r06_session_contract.cpp`：三种Adaptive Session在非法帧次序/非有限旋转/空流求解失败时不推进帧、不提交主球谐系数或四组周期基线；验证根因ErrorFrame和Comparison非法参数不消耗RNG。
- `tests/integration/test_r06_analysis_segments.cpp`：合成600行ratio与误差数组，检查Comparison固定四段边界、阈值等值不触发跳变、平移/ratio的501个连续点、UpdateRates旋转局部分段窗口及零边缘、平移全600行平滑后裁剪，以及非法分段和NaN ratio的明确拒绝。
- `CMakeLists.txt`：将以上3项登记为`r06_sampling_flow_contract`、`r06_adaptive_session_transaction`、`r06_analysis_segments_smoothing`，原有9项CTest登记保持不变，**共12项登记，不等于通过12项**。

**严格未执行项**：未运行CMake configure、编译、CTest、CLI、真实MAT读写、MATLAB/Octave、Wolfram、L2/L3/L4、性能或数值测试；没有MAT生成或改动`origin/**`。H-11执行权限未得到本轮授权。R6下一阶段仍须在获许可后统一原生构建并修复发现的实际错误，执行真实Writer路径/竞态负例与完整数据来源验证，补齐独立L3逐节点、历史L2/必要L4，核验SHRotate相位/动作差异及DEC-005/006、冻结和最终验收。此前固定SHA的3项SH CTest与旧582/582探针不能冒充本HEAD新增测试的PASS证据。

## 0.6 R6源码覆盖继续补充（2026-11-16，所有新测试未运行）

**核查起点**：`bd229ecf75e5af432fba9720eae3d486768d58c6`。按[ROADMAP](ROADMAP.md) R6仍需完成统一CMake、单测/集成/独立L3候选源码及获授权后的原生验收。R5代码静态闭合的声明保持限定；**R6代码实现与原生验证均未完成，不存在已批准R7批次**。本次实作不是补充空壳计数，实际新增下列C++测试文件：

- `tests/integration/test_r06_kvd_l3.cpp`：将既有 `verification/kvd/six_direction_exact_trace.v1.json` 与 `near_sign_flip_exact_trace.v1.json` 的独立Wolfram结果组织成三组6/8方向固定流。检查Franz各轴分母与初态、Original/Modified首轮不同的原始t更新、非零近度、符号翻转、六轮迭代停止/continue、预测光流shape、MSE与RMS分类、末次近度均值缩放；不借被测算法生成参考数据。
- `tests/integration/test_r06_adaptive_l3.cpp`：将 `verification/adaptive/asymmetric_signed_near_one_step.v1.json` 的正/负均值九方向、3×5原CS系数、切向流及独立M1/M2/M3/M4、rhs、6维解、归一t、旋转、MSE、新near、九新系数写成固定输入。负均值仅作非物理有符号对照，不宣称正距离普适失败；阈值是未批准L3回归诊断而非DEC-005最终容差。
- `tests/integration/test_r06_archive_readonly.cpp`：对现有582 MAT只读扫描、单变量double及列主序自拥有数据、5种历史shape分布和SHA-256可计算性进行原生测试准备；**不触碰原文件写入、不作独立逐值Golden或声称已核验预期SHA**。
- `tests/integration/test_r06_sh_adapter.cpp`：验证AN-G08的90°物理旋转、`near_new(d)=near_old(R*d)`参数作用、反方向Rᵀ鉴别及往返与反平行失败；不把旧源SHRotate相位/作用差异抹去或虚报等价。

**CMake实修**：上述四项独立CTest均已接线，登记条目由12增为**16**，但**本分支新CTest实际通过项仍为0（没有执行）**。同时核查 `tests/integration/test_r01_cli.cpp`，发现旧R0占位消息`not implemented`和`plot`退出码的过时断言，与R5现行Composition Root实际错误映射不一致；实际修正3处断言，另外加入具有假Analyzer但无SVG后端时`analyze`成功、`plot`退出7的隔离合同断言。不是只修改PROGRESS记录。

**执行与结案门禁**：本轮没有执行CMake/g++/CTest/业务CLI/MAT读写/数值/性能实验，也没有新运行日志或PASS；没有修改`origin/**`。H-11仍待明确授权。R6余下包括未完成的原生编译修复、真实Reader/Writer安全/权限竞态、L2独立逐值及完整可信文件哈希、剩余L3多轮/多帧和SHRotate独立相位/动作验证、必要L4统计、DEC-005/006与最终冻结/发布；真实执行若失败需回到业务源码修复。旧固定SHA的SH 3/3和旧582/582探针绝不能冒充本轮16项通过。

## 0.7 R6静态源码复核及四项回归扩充（2026-11-16）

**检查起点**：`127a215a31ee6697eac3af4e30af1e9b76984265`。对真实 `ROADMAP.md`、`PROGRESS.md`、根CMake/构建脚本、R6已登记16项CTest、采样/轨迹/Adaptive Session/RunDirectory生产实现与独立方向参考进行了静态核查。结论：**R6代码/测试源码范围仍有缺口，继续R6，不进入R7**。此轮完成以下实质代码修改并推送：

1. `tests/integration/test_r06_direction_rows_l3.cpp`：首次将全部714个独立方向参考（2142个坐标）作为源文件中的C++运行时只读候选夹具输入，直接逐行/逐坐标比较0–2级三类采样器。依赖已有 `verification/geometry/source_direction_levels_0_2.v1.json`；来源既有独立Wolfram对照，不在测试里自算标准答案，不将候选设为批准Golden。
2. `tests/integration/test_r06_trajectory_sphere.cpp`：给circle/sinusoidal源分母、首末帧身份、三点直线相对姿态与局部平移、垂直/反平行方向、球内解析近度、相机旋转与边界失败新增静态手工数值预期；修复新测试中对`Result<T>::error() const &&`禁用重载的错误调用，先保留Result左值再读取错误。
3. `include/egomotion/estimators/adaptive_step.hpp`增加独立`IAdaptiveStepSolver`抽象；`AdaptiveMfaStepSolver`实现其`solve`；`AdaptiveMfaSession`构造接受可选**借用**求解器，不注入时仍创建真实默认求解器，算法、输出及源周期逻辑不改。Session禁复制/移动，避免默认求解器指针悬垂。`tests/contracts/test_r06_adaptive_periods.cpp`使用只在测试中出现的定值步进替身覆盖MATLAB i=2..22的四周期刷新、`dumi`丢弃、主状态提交、period5比较失败和主Step失败的原子回滚与同帧重试，另核初始化/比较模式真实调用顺序。**这是控制流测试，不冒称求解器独立L3数值验证。**
4. `tests/integration/test_r06_io_safety.cpp`在未来CMake二进制树独立沙箱构造源目录和拒绝路径，验证重复/非法RunId及仓库根、origin、output符号链接拒绝；不修改真实`origin/**`，不清理/覆盖用户结果。
5. `CMakeLists.txt`将上述四测试独立登记，CTest目标**16→20（+4）**，并纠正两处此前新增的`CMAKE_CURRENT_SOURCE_DIR`变量被反斜杠转义而无法展开的路径bug。新注册数不转为完成或通过数。
6. `build.sh`旧注释误称只编译低阶SH，但CMake实际已包含全工程R0–R6；修订为全项目编译/CTest，默认目录改`build/r06-full`，在真正进入配置/构建前设置显式H-11批准开关检查，修复错误消息换行。该开关不会自动授予权限，H-11决策仍由人类批准。

**本轮没有执行任何 native CMake、编译器、CTest、业务CLI、MAT读取/写入、Python/Wolfram数值实验或基准测试。** 本次引用的旧582/582只读探针与固定SH 3/3均属于其它固定SHA，不宣称是当前HEAD的通过证据；新20项CTest**登记20，运行0，验证通过0（未执行）**。未更改任何`origin/**`内容、不改历史签署或既有审计台账。R6未结案：真实编译与错误修复（需H-11）、MAT安全与SHA/L2逐值、R3/R4完整数值与8次聚合、SHRotate源作用独立等价/差异判定、必要L4/DEC-005/006/冻结批准都仍需证据。原`ROADMAP.md`没有R7，故本轮不新增。

## 0.8 R6输出暂存安全与Result/Manifest合同（2026-11-16；未执行）

**检查起点**：`f69a6220b644f1aa15e04c16f6fdfb7efc884eb5`。复核ROADMAP、PROGRESS、20项CTest与R5发布路径的实际代码后，发现现有MAT/文本Writer在多种prepublish错误或发布后核查失败时可能遗留随机暂存文件，且最终发布确认先于暂存硬链接清理。**这是真正的生产源码安全缺口，并非为增加计数而改台账**。

**已修改的生产代码**：`src/io/run_mat_writer.cpp`及`src/io/run_text_publisher.cpp`增加Linux RunId目录fd锚定的专用`StagingGuard` RAII清理，释放仅由本次`O_CREAT|O_EXCL`创建的暂存名字，不删除已发布的最终文件或其它用户文件。完成`linkat`后，如暂存名`unlinkat`或目录`fsync`失败，返回明确`ReadbackMismatch`且不声明成功；MAT receipt `durably_confirmed=true`仅在最终发布、暂存清理和目录持久性都确认后设置。失败时保留已链接最终文件和未确认receipt作为真实`Incomplete`审计，而不是回滚已发布数据或伪造`Complete`。

**新增/强化的测试源码**：

- `tests/integration/test_r05_io.cpp`：沙箱成功写入和重复写入后扫描独立RunId，断言无`.r5_*.mat-tmp`及`.text_*.tmp`遗留。
- `tests/contracts/test_r05_manifest.cpp`：在真实既有RunContext合同基础上注入无效MAT receipt哈希、Manifest TextPublisher发布失败以及`UNVERIFIED_BUILD_SOURCE`，验证`Complete`门禁、独立原因链和恢复后只能持久化`Incomplete`，不靠虚假签署或触碰MAT。
- 新增`tests/contracts/test_r06_result_lifetimes.cpp`：编译期类型断言与运行期所有权合同，覆盖临时`Result`右值自拥有提取、move-only对象、`Status`失败值和多层`std::forward_list<ErrorFrame>`原始错误信息/JSONL。`CMakeLists.txt`登记单独`r06_result_lifetimes_error_frames`。

**状态变化**：CTest登记**20→21（+1）**，并在原测试内扩展2类断言。R6生产代码发生2文件实质安全修复。此处只是源码静态完成工作记录，**本分支原生CMake/编译/CTest实际执行0项、通过0项（非失败0项）**；没有L2/L3/L4新数值实验或原MAT写入。H-11仍待人类明确授权，历史SH固定SHA的3/3、旧582/582探针不迁移为本轮PASS。R6尚未完成，真实运行/测试发现错误时重开代码；后续需真正编译、I/O安全竞态与回读验证、独立L2/L3/L4、源SHRotate方向/相位证据及DEC-005/006批准，不启动无定义的R7。

## 0.9 恢复当前分支后的 R5/R6 静态复核

**标签 `[AI-DRAFT + HUMAN-APPROVE]`；固定对象 `eeb30135a48f27b3eb9c6c289709e9e792770ec8`。** 已恢复本地落后的389个提交并核查当前ROADMAP及真实源码。59个src/tests CPP都出现在根CMake文本中、引用路径缺失0；21个测试名称无重复，**此为静态核对，未执行CMake/CTest**。

发现 `RunMatWriter` 将 const payload 的 `const double*` 传入本机matio 1.5.26声明为 `void*` 的 `Mat_VarCreate` 参数；应重开R5该项源码修复，不能继续将“源码闭合”解释为已可编译。局部可变副本及保持const输入的回归方案、发布后Manifest失败与真实暂存故障覆盖缺口、独立L2预期摘要缺口均记于 [R6接续复核](reviews/R6_READINESS_eeb30135.md)。本轮按用户最新直接提供的审阅/方案文档边界，不修改生产C++、测试或CMake，不将仓库旧代码许可扩张为本轮执行许可；H-11及延期的最终审批不变。

没有新原生运行、数值测试或PASS；未创建Issue/PR，未合并/发布，未改动origin。R6仍未完成，不新增R7；完整后续义务保持ROADMAP原范围。

**同一复核的接续增量（基线2615b82）**：按实际`RunTextPublisher → RunContext → CLI`分支确认“linkat成功、目录fsync失败”时，最终Complete文本仍可见但API失败；随后同名Incomplete发布因OutputExists再次失败，CLI应保留两个独立事件。现有FakePublisher只模拟发布前失败，未覆盖此路径。已在上述复核文档§6给出三个Publisher合同场景、九类真实I/O故障场景、具体源blob与观察规则；全部为静态推演/候选，**没有故障注入执行或新增CTest登记**。未选择新schema/恢复协议，未改变Complete/非覆盖/验收义务，限定源码授权仍待回复。

## 1. 当前边界

仓库 `ncut-d1z/Egomotion-Estimation-with-Dense-Flow-Fields`，分支 `dev-plan-P0-P1-mat-dictionary-probe`；不合并main、不发布。`origin/**` 原45份.m与582份.mat只读；不要求执行MATLAB/Octave或获得L1。既有66REQ/66TC/98P、27条HLD设计边不变；数学子案例不新增正式TC。

本轮无C++、头文件、CMake、CTest、构建脚本或MAT的修改和原生构建运行。Wolfram中的代数与高精度数值是实际执行的数学检查，不称作生产Adaptive实现、C++单测或批准Golden。动态批准事项仍归 [TODO](TODO.md) 与 [DOCUMENT_BASELINE_REGISTER](DOCUMENT_BASELINE_REGISTER.md)，原E-DOC-069历史批次尚未完成DEC-005/006及正式接口冻结；之后DEC-008-01～06已作出设计决定（见[TODO §2.2](TODO.md)），但公共接口实现、原生测试及正式冻结仍未验证。

## 2. 固定里程碑与历史检索

| 记录 | 已取得的限定证据 | 不扩张为 |
|---|---|---|
| E-DATA-001及早期记录 | 582/582独立MAT类型探针READ_OK，原数据只读 | 正式Reader/Writer或582份独立SHA-256/逐值L2验收 |
| E-DOC-054 | 1996/1998/1999论文材料、勘误和转置/相位核查 | 软件运行或全域数值稳定性证明 |
| E-DOC-055 | 九实基、IR/Choi/定义三路线一般SO(3)精确数学；SH叶子源码 | 全部业务模块或任意IEEE输入正确 |
| E-DOC-056 | 用户在8eab5dd原生1/1 SH CTest通过 | 后续提交自动通过 |
| E-DOC-057/058 | 用户在445ed151增强解析/相位回归1/1通过，随后扩大测试 | 全66TC通过 |
| E-DOC-059 | 用户在601c74de完成53,532检查/624非零四元数的1/1 SH CTest，双类残差有日志 | 全SO(3)浮点路径或最终DEC-005 |
| E-DOC-060/061 | AN-G06/07/08独立归档参考、原二阶逐系数投影实现与数学参考 | 当时尚未运行的新CTest通过 |
| E-DOC-062 | 用户固定5e522198的SH源与三个独立CTest全部通过，53,532+176+76检查 | Adaptive/KvD/采样业务软件通过 |
| E-DOC-063/064 | 三类采样i=0/1/2全714行参考，随后独立Wolfram逐元素核2142坐标，最大差1.11e-16 | 生产采样器/RNG或正式Golden批准 |
| E-DOC-065 | AN-G12对称20方向独立数学单步；+1e-11使near_new仅近似1 | 全部Adaptive输入、Session或真实CTest |
| E-DOC-066 | Original/Modified KvD三组输入共36轮精确数学trace；最少6/最多200的正常控制流界 | 真实模型可达200轮或生产软件验证 |
| E-DOC-067 | 非对称九方向正/负均值独立数学；正支恢复、负支M1与加权投影差2I，条件数有结果 | 所有物理正距离输入失败或可擅改负支 |

上述记录的全部原始数值、源Git blob、SHA-256、日志路径与纠偏过程均在本页顶部的固定历史版本及各 `verification/` 工件。当前不再使用旧“SH源尚未编译/三个CTest未运行”描述：用户已于固定 `5e5221988cf093dbfddb88f2f65f06632460f3a1` 在 Ubuntu 24.04/GCC16.2/CMake4.3.4/Eigen3.4、Release/C++17下完成 **3/3 CTest Passed**。这来自用户终端文本，不是代理在其机器执行。

## 3. E-DOC-068：零均值、抵消、几何退化和最终平移归一的分层证明（2026-11-09，历史批次）

### 3.1 固定起点和真实源

该轮起点 `982537556be1ac5a414af368a0082806a9179d32`。开始时97个唯一AF（74已整改、23未整改）；94个文件（62已审计、32正在审计、0空白）。重新读取 `AGENTS.md`、两份审计台账和原 `oneit_adaptiv_matched_filter.m`（Git blob `48e51bcd52fb8f27cf35b3fce38b28507af36e55`），只依据原先取均值、负支翻号、仍除旧均值、M1～M4、最后t归一的实际表达式开展推导。

### 3.2 该轮实际提交的三个工件

| 路径 | 内容与限定状态 |
|---|---|
| `verification/adaptive/verify_zero_mean_conditioning_families.wl` | 实际Wolfram精确恒等式及70位SVD验证脚本；固定完成版Git blob `36eedaf6477d0255b7ab0f12572506970da6847c` |
| `verification/adaptive/zero_mean_conditioning_evidence.v1.json` | 保留实际25/25检查、6组SVD样例、精确公式、原源blob与限制，非批准Golden |
| `docs/design/ADAPTIVE_MEAN_ZERO_CONDITIONING.md` | 现有Adaptive LLD的理论专题补充；推导定义域、扰动界、恒定行列式反例、正深度窄锥谱、纯旋转平移归一失败及TC映射 |

新增推导属于该轮从源代码出发的研究结论，不声称论文已经给出这些定理。没有以其它算法替换源行为，也没有实施新的阈值、求解器或错误处理。

### 3.3 原始near与翻号后near的准确区分

令r是原始raw near、s=mean(r)、r_flip=sign(s)*r，源最终近度mu=r_flip/s。对于s不为0，**mu=r/abs(s)**，均值为sign(s)。这不是把“已经翻号的near”再除以abs(s)：后者在负支确实会改变原结果。旧文档有关abs(s)的警告必须连同变量阶段解读。

在不跨过零均值的扰动域内，精确相减得到：`delta_mu=(delta_r-sign(s)*mu*delta_s)/(abs(s)+sign(s)*delta_s)`。据此可分离相对尺度与抵消。若所有raw近度非负且s>0，则归一mu在[0,N]；s绝对值小本身不等于发生正负抵消。无量纲chi=abs(s)/mean(abs(r))可辅助记录抵消程度，但本轮没有为它规定生产阈值。

### 3.4 精确参数族及实际Wolfram结果

1. **零均值输入有完整SH表示**：六轴方向与原CS系数 `C00=a*sqrt(4pi), C1,-1=-b*sqrt(4pi/3)` 重建raw near=a+b*y。a=0,b=1给非零向量(0,0,1,-1,0,0)但均值0；a=b=0给全零输入。两者的原均值归一都无定义。没有执行MATLAB，因此不推断其具体警告/异常传播。
2. **全正同比缩小**：b=0、a>0时M=(2/3)I6、条件数严格1；按原光流合成运动解出raw平移a*e_x。若raw平移被横向绝对扰动eta*e_y影响，方向角为atan(abs(eta)/a)。故矩阵良态与最终方向稳定性不是同一条件。
3. **有符号抵消**：a>0、b固定时源M的两个对角块均2I/3、左下块为(b/(3a))*[e_y]叉乘矩阵。det(M)=64/729、六个特征值恒2/3，但kappa2=((sqrt(gamma^2+4)+abs(gamma))/2)^2，gamma=b/(2a)，随a趋0按b²/(4a²)发散。b=1、a=1e-6时约2.50000000002e11。a<b时含负近度，明确不是严格正深度场景。
4. **严格正深度的窄锥几何**：四个单位方向采用有理半角参数h，raw near全1、s=1；在0<h<=1/2域，精确谱三对特征值为2h⁴/(1+h²)²、4h²/(1+h²)²、2/(1+h²)²，所以kappa2=h^-4。h=1/10、1/100、1/1000分别为1e4、1e8、1e12；h=0极限矩阵秩2。此例不靠负近度即可发生严重几何退化。
5. **纯旋转的后置失败**：六轴与raw near全1、M条件数1，纯旋转光流使六维解前三项精确为0；原源码随后t_raw/norm(t_raw)无定义。旋转解与无平移方向的定义域必须分别记录，不能把6×6可逆当成整个单步成功。

完成版Wolfram Language 15.0.1实际执行 **25项检查，25项为True**；包含完整特征多项式恒等式，不仅是有限样本。另对抵消族3组、窄锥族3组输入做70位SVD与公式交叉检查；以相对1e-35作为本次诊断阈值，JSON中差值显示0.0代表该工作精度下结果，不宣称任意机器逐位相同。初始未化简的极限曾产生中间Power::infy提示，完成版先化简增长比再取极限，最后重跑没有这些提示；失败/中间版本不算正式通过。

### 3.5 证据归口和剩余卡点

专题映射现有TC-F-ADAPT-001/002/003、TC-DC-ADAPT-001；没有新增正式TC，也未修改 `TEST_PLAN.md` 中已注册的实体或编写单元测试。新增数学证明补充AF-044/051/054/055/060的若干定义域和病态反例子义务；这些AF仍包括更多一般/近奇异/异常、Session原子提交、真实生产C++/CTest、独立参考覆盖和DEC-005/008批准，因此保持未整改。当前理论不能提供经过真实软件误差校准的阈值，不能自动修正原负支。

该轮只有理论文件与治理记录变动，所有历史原生测试仍绑定各自SHA。

## 4. E-DOC-068历史起止指标

| 指标 | 开始 | 结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 94 | 97 | +3 |
| 已审计文件 | 62 | 65 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

## 5. E-DOC-069：低阶SH离散重新拟合的往返与正值性（2026-11-10，历史数学批次）

### 5.1 核查起点、数学依据及新工件

本轮读取并固定远端起点 `21ad0c34f3c43fd1bbf33c847406dfd302e239ea`；97个AF（74已整改、23未整改），97个审计路径（65已审计、32正在审计、0空白）。重新核对原 `koeff_aus_near.m`、`near_aus_koeff.m`、`kugelflfunc2.m` 和相关采样/Adaptive源规则，原始文件不改。

新增[离散拟合理论专题](design/SH_DISCRETE_REFIT_CONSISTENCY.md)、[Wolfram完整脚本](../verification/harmonics/verify_discrete_refit_roundtrip.wl)、[实际数值证据](../verification/harmonics/discrete_refit_roundtrip_evidence.v1.json)。源3份SH文件Git blob依次为 `5ff91a2210a20e5acf79dec6e59ddd33b55529d3`、`7373c4376dfd5bbf281136ced39657d411a10ffa`、`edc0caa1f88bf91d2a189e4d3a283334f809b905`。完成版脚本blob `41a8d9c034d739702a7e867de1f52b1c1865c38f`，UTF-8 SHA-256 `6a212ec37201c741e33550b47a11d28b8381269df6a97d16745b7d69c6641c9e`。已从提交 `9c6b263903150742b452c0cfe0ff11d132194748` 重新读取并实际执行该脚本，而非只执行未保存的临时代码。

### 5.2 一般代数推导与实际35项辅助检查

原离散系数规则对每一通道分别除以平方范数。设 B 是原CS归一基的N×9采样矩阵，G=BᵀB、D=diag(G)；在D全部严格正的定义域，精确源拟合为c_hat=D⁻¹Bᵀr，系数往返T=D⁻¹G，样本算子P=BD⁻¹Bᵀ。因此对任意九系数精确往返，以及P为幂等投影，都以**当前离散G为对角矩阵**为充要条件。一般证明写入专题；连续球面正交不能替代该离散条件。

Wolfram35/35固定例/恒等式检查实际通过，包含以下结果：

- **24方向正向对照**：全部不同的(1,2,2)/3带符号排列形成秩9、离散G对角且分母正的采样；源逐项拟合对所有九系数精确还原。
- **短小精确正近度反例**：在24行后附加12个v=(1,2,2)/3，所有36个near均为1、每通道分母正且rank(B)=9；按原逐项规则拟合、再按原基重建，在已观测d*=(2,-2,-1)/3得到**−9/140**。多项式缩放系数beta=(1,−3/5,3/5,−3/7,9/10,−9/8,3/5,−9/10,−9/7)；beta=Sc，不能混称为原归一基系数c。全部样本的重建均值是383/210，不是输入均值1。
- **排除重复视线**：把12行换成有理立体投影psi(1/5+j/10^6,2/5)，j=1..12；36条方向全部互异且精确单位，包含rank9基集，每个分母正。d*处结果为精确有理负数，并严格位于(−65/1000,−64/1000)，近似−0.06427848307778179；分子2853位且负、分母2855位且正。结论不依赖浮点误差，也不依赖输入负近度。
- **最后投影阶段的独立定义域**：八个立方体角方向使Y20、Y22对全体样本为0，原平方范数分母为0，九基rank7；但常数近度下运动矩阵为(2/3)I6、条件数1。四方向窄锥则使Y2,-2恒为0，即使选择h=1/4令运动矩阵可逆，末端拟合仍有数学零分母。不能用前置运动矩阵条件替代最后每通道资格。

### 5.3 该发现的边界及真实卡点

本轮不是证明“球谐定义或IR/Choi旋转错误”，也不是推翻固定5e522198的3/3来源忠实性测试；它发现的是**正确基函数之上的离散逐项系数估计并不具有任意采样下的精确逆或保正保证**。正近度反例可对应所有物点距离均为1的输入，但不是现有实验默认方向集已出现相同失败的实测声明。

H=D⁻¹/²GD⁻¹/²的trace为9、对角线全1，G非对角时存在大于1的特征值；这只说明单独重复T存在放大模态，不外推完整Adaptive必然发散。精确零分母在浮点中可能表现为极小非零，不声称取得原MATLAB警告或新C++错误码。

本轮只新增数学材料并更新三份治理记录，未修改主harmonics/Adaptive LLD、原AN-G候选或正式TC注册；新专题明确映射已有SH/Adaptive/采样相关义务，不新增TC。AF-044/045/047/051/055/060仍需各自完整公共/软件/异常/批准条件，不能因该局部理论结案而关闭。没有替换联合最小二乘、裁剪负值、增加阻尼、改变权重或通过扩大容差掩盖差异。

## 6. 本轮E-DOC-069七项指标

| 指标 | 开始 | 结束 | 净变化 |
|---|---:|---:|---:|
| 整改目标总数 | 97 | 97 | 0 |
| 已整改 | 74 | 74 | 0 |
| 未整改 | 23 | 23 | 0 |
| 审计文件总数 | 97 | 100 | +3 |
| 已审计文件 | 65 | 68 | +3 |
| 正在审计文件 | 32 | 32 | 0 |
| 未审计文件 | 0 | 0 | 0 |

三份新数学文件仅在声明的数学范围内完成审阅；原97个路径/状态、97条AF义务/状态保留。第一轮仍23项开放，未启动条件性的第二轮全面审计。C++、CMake、CTest、build.sh未改，**不需要重跑旧SH构建**来验收本轮文档/数学更新。
