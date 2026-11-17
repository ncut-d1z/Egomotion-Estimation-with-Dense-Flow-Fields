# R6：45份原源码的现行实现与证据复核

标签 `[AI-EDIT]`。本轮仅修正实现状态、名称、链接和证据追踪，不改变REQ/AC/TC、算法公式、数值门槛、接口批准或冻结状态。复核时间2026-11-17 UTC+08:00；静态核对完成于22:22，后续提交另见Git历史。

## 检查对象与结果

起始HEAD及GitHub分支API均为 `b3cc6eda6168f10102dd54e559222dca23c237c5`，分支 `dev-plan-P0-P1-mat-dictionary-probe`；工作区起始干净。它与受测 `130a82ba0090e186fcf43bc28bb98c282e68c96c` 之间仅有文档变更。关联任务“检查R5实现完成度”最新可见用户任务为继续检查/实现R6；任务状态idle。首次读取超时，第二次取得实际结果；未向其他任务发送消息。GitHub all-state Issue/PR集合为空，未创建或关闭远端事项。

[迁移矩阵](../MIGRATION_MATRIX.md) 的42行旧“未实现/未测试”已改为实际符号、文件、测试组及局限；原三行illustration亦统一纳入现行口径。对象模型中的候选名称并非源码API：实际四分析入口由 `MatResultAnalyzer` 分派，三Adaptive入口由 `AdaptiveExperimentRunner` 分模式计算，两Bias入口共享注入式 `BiasExperimentRunner`。原路径、源身份与函数数量保持不变。

| 核对内容 | 实际证据与结果 |
|---|---|
| 源路径/身份 | `git ls-tree -r -l HEAD -- origin/egomotion_estimation_dense_flow_fields_matlab_scripts` 与SOURCE_INVENTORY按路径核对：45份.m的Git blob、字节全部一致 |
| 工作区源字节 | `git ls-files -z 'origin/**/*.m'` 的路径顺序与 `git hash-object` 45项结果逐项对应HEAD；原始资料无diff，origin树SHA仍 `70ee155131c24a50169346d0ffbaff0c01b1c547` |
| 声明数 | `rg -c '^[[:space:]]*function\b' ... -g '*.m'`：44文件各1，SHRotate为6，合计50；矩阵45路径唯一、声明总数50 |
| 当前实现 | 读取对应生产源码/公共头、实际装配及相关测试；45行均给出实际承接位置或融合/非使用范围，没有凭候选名虚构类 |
| 文档引用 | 矩阵69处相对文件链接逐项解析；包含本报告的新引用，均有真实文件目标。此项不声称每个网页或HTML锚点均做浏览器验证 |
| CTest映射 | TC证据索引19个组名全部可解析；索引48个唯一CTest与固定130a82的两份JUnit逐名一致，均status=run且无failure/error/skipped。CMake有46个固定名及contract/archive两项foreach实例，总48；不是仅比较计数 |
| 需求范围 | SRS §9.2只改实现状态，45行路径/REQ/TC保持；TEST_PLAN的66个TC、PLAN的98个P及各AC义务不修改 |

上述是本地Git/文本及已保存JUnit的只读复核，不是重新运行CMake/CTest或 `scripts/check_docs_traceability.sh`。本轮没有生产/测试源码变更，因此不重复执行已有测试。最新48/48的RunId、命令、环境、stdout/stderr、退出码、输入身份仍见 [MAT封存实测](R6_MAT_SNAPSHOT_2026-11-17.md)，不将本轮文档提交冒称为新受测代码SHA。

## 明确保留的范围差别

- dot/cross直接由Eigen运算承担，`zellendefinieren` 融入Franz三轴累加，`winkelausvektor` 融入笛卡尔低阶基；没有新建对应公开包装函数，也没有原中间角度的运行轨迹证据。
- `SHRotate` 六个声明逐项说明：l≤2的M/U/P/V有对应实现或内联，W因该范围系数恒零不求值；高阶不是已实现。`transfor_koeff` 使用单独的源phase动作，不能改用物理参数旋转。
- `rodrigues` 只承接实际生产调用的向量→矩阵；原矩阵输入/Jacobian及return后例子没有移植或测试资格。球谐求解器九基与插图通用未归一化勒让德分离。
- 三Adaptive入口实际未成功：初始化第14帧acos越界、其余第151帧零旋转，均Incomplete无MAT。Complete替身合同与真实失败测试不是完整实验的数值成功；两Bias有界运行也不是40份完整统计实验。

**结论：本轮完成当前实现去向和有限证据映射，未完成全部数学分支/输入域资格或正式R6验收。** 仍有完整Adaptive成功、必要L4、病态前向误差、跨环境/资源和正式批准缺口；不引入MATLAB/L1前置条件。下一步先检查实际交付链接依赖、可选绘图隔离及日志/输出合同等可独立工作，不改变已记录的数值失败策略。
