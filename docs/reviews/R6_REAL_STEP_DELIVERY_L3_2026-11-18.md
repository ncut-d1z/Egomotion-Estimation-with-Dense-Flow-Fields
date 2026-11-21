# R6：真实Adaptive Step至MAT交付链独立对照

`[HUMAN-EXECUTE/APPROVE]`，2026-11-18。受测源码 **`3985b056aa19935c8348f0fefa5f5292f9fbd3b4`**，分支`dev-plan-P0-P1-mat-dictionary-probe`；提交、正常推送并用GitHub branch API核实SHA后，在已有生产代码/构建/CTest/文档授权内执行。**SVG ON73/73、OFF72/72 CTest通过**；配置/编译无诊断，零失败/跳过/禁用。本批没有修改生产算法、阈值或原数据，L3参考仍为候选/未冻结，R6未结案。

## 已补缺口与边界

原完整Runner/Writer成功测试使用替身Step，已有独立数值Session测试未贯穿完整重复/聚合/磁盘交付。本批增加[测试源码](../../tests/integration/test_r06_adaptive_delivery_l3.cpp)、根[CMake登记](../../CMakeLists.txt)和[固定输入说明](../../verification/adaptive/real_step_delivery_reference.md)，实际运行真实FlowSynth、均模长噪声、Sophus上一帧旋转、Adaptive Step/Session、八轮聚合、MAT Writer/Reader及Manifest。

轨迹位置、真运动、近度、12视线和normal输入为显式合成夹具，保留原入口35/600帧、8次重复及周期1/5/11/21。采样器核查原请求level2/4后返回固定12视线；不是原采样规模。合成场含二阶表示之外的近度分量；位置、运动与近度不声称构成一致的物理轨迹。每个RunId有`SYNTHETIC_INPUTS.txt`，Manifest来源标明`synthetic-input:real-step-independent-delivery`。**合成成功不等于原科学实验成功，也不是L4统计证据。**

Expected由标准库long double独立理想光流/噪声、Rodrigues公式、源系数多项式作用、标量消元及自身Session状态演进生成，不拿生产流、状态或输出充当参考。独立角度采用atan2(cross范数,dot)，生产仍用未钳制acos。沿用flow绝对1e-12、Step混合2e-8及既有Runner输出绝对1e-9工程余量，没有新增/放宽数值阈值或签署DEC-005。

## 实测数据

每个SVG配置均执行完整三模式，检查总数 **3,676,721**：

| 合成入口 | 帧数/重复 | 有效内帧 | 真实Step | normal消费 | MAT文件/保存值 |
|---|---|---:|---:|---:|---:|
| Initialization | 35 / 1 | 33 | 99 | 0 | 4 / 140 |
| Comparison | 600 / 8 | 4784 | 14352 | 172224 | 5 / 3000 |
| UpdateRates | 600 / 1 | 598 | 2990 | 0 | 3 / 5400 |
| 合计 | 三个独占RunId | 5415 | 17441 | 172224 | 12 / 8540 |

Comparison的normal按每行x/y/z连续消费，跨八轮不reseed；四错误数组按固定8平均，理想场`rottransratio`只保存第八轮，输入特意使各轮比值不同。UpdateRates四列保持MATLAB列主序，akrot独立计算，端点严格0，未新增HistoricalOnly字段。三Manifest均Complete且attempted/completed/valid/failed/undefined符合完整合成日程；不将这些计数当作原科学样本。

| C++ long double比较项目 | 最大绝对差 |
|---|---:|
| Step/输入系数 | `3.09378390961345672849e-13` |
| 理想/含噪光流 | `9.96078219905882633611e-15` |
| 保存值（角度、ratio、akrot） | `5.406439324458240113e-10` |

17,441行Step JSONL每行113个数值节点（另含9输入系数）；12行保存值JSONL共8,540值。每个MAT分别经生产Reader和独立标准库/zlib v5解码，binary64逐位相同，再与独立最终参考比较；实际大小/SHA对应durable receipt及Manifest。

## 可复核执行记录

RunId `r6-real-delivery-20261118-ghz37p6I`。实际环境Ubuntu24.04.5、Linux7.0.0-43-generic x86_64、GNU16.2.0/C++17 Release、CMake/CTest4.3.4、Eigen3.4.0、matio1.5.26、zlib1.3、Sophus1.24.6；工作目录仓库根。ON时间2026-11-18 21:11:51–21:13:23，OFF为21:13:38–21:15:00，均+08:00。先MAT7项，再ON66项/OFF65项；configure/build/CTest/输入完整性均exit0，无依赖安装。

```bash
bash build/r6-real-delivery-20261118-ghz37p6I/run.sh 3985b056aa19935c8348f0fefa5f5292f9fbd3b4 ON
bash build/r6-real-delivery-20261118-ghz37p6I/run.sh 3985b056aa19935c8348f0fefa5f5292f9fbd3b4 OFF
```

复用独立ON `build/r6-20261117-KzjFHPDQ/cmake`、OFF `build/r6-headless-20261117-Joe73gEd/cmake-off`缓存，完整证据保存于本RunId的`evidence-ON/`及`evidence-OFF/`：环境、输入Git blob/文件SHA、命令/参数/起止/退出码、分离stdout/stderr、四份JUnit、Testing、依赖图、ELF、CMakeCache及compile_commands。739项origin/verification/tests输入前后SHA一致，原628文件/582MAT树仍为`70ee155131c24a50169346d0ffbaff0c01b1c547`。

原始合成运行沙箱在各缓存的`r06_adaptive_delivery_l3/`；全部JSONL、MAT、Manifest及来源标记另保留于本RunId的`saved-delivery-ON/`和`saved-delivery-OFF/`。具体随机RunId/路径与每个MAT摘要见`read-only-delivery-audit.json`，没有提交生成结果或日志。

| 证据文件 | 字节数 | SHA-256 |
|---|---:|---|
| 两配置step-nodes.jsonl完全相同 | 80360942 | `2cd25e0da6ddaec52cf3aab552e6b5ec0c7c9e164b98ce0ec62b4f85a076bdf1` |
| ON published-values.jsonl | 400476 | `9f0c4bbcfe008002d6a6f4a92f313bf1436ccf5672fecfd07b7f7491ff733a39` |
| OFF published-values.jsonl | 400476 | `d515dad8e6ed648c4b06b90ce1503032364bc07f28437fc440a4fb30a1861c58` |

保存值JSONL包含各MAT文件SHA，因此文件字节不同；独立逐文件比较证明12个MAT在128字节头部之后的压缩载荷完全相同，头部libmatio时间不同。两配置所有保存值Expected/Actual及Step节点一致。只读Node审计逐名匹配JUnit，重算保存节点的角度/八轮平均、末轮ratio、akrot及全部8,540值；与long double参考的重算最大差约1.42e-14。此binary64复核不替代C++原始误差。1,970,833个Step节点数字也全部核对，见`read-only-delivery-audit.json`及`read-only-retained-identity-audit.json`。

既有无噪/含噪Session各252帧、回滚及JSONL字节SHA与928a807时完全相同；21组旧数值/合同摘要保持，球谐494项最大差仍1.7763568394002505e-15。ON/OFF唯一CTest登记差别仍是`r06_svg_source_windows`；OFF无绘图实现源码/链接目标/符号。对应只读JUnit、regression、multiframe、noisy审计保留，旧运行和失败证据未覆盖。

文档只读核对12份文件的478条相对链接、98个P编号、66个TC编号及45源路径/50声明，全部保持有效；TEST_PLAN规范正文与Adaptive LLD除顶部事实接续外均不变。保存`read-only-document-audit.json`，不是Bash追踪脚本现场执行。后续证据文档提交不代表重新构建，受测代码SHA仍为3985b05。

## 仍未完成

本批补AD-G05/SN-G06及P8.1–P8.7的**合成完整日程数值交付**子范围；原始Initialization仍帧14 acos越界，Comparison及UpdateRates仍帧151真旋转为0，三真实入口仍Incomplete、无完整科学MAT。原场景同分布L2/L4、完整Bias、病态前向误差、跨环境/资源、正式Golden/DEC-005/006/008、冻结和发布未完成；不要求MATLAB/Octave/L1。本批未发现新的生产缺陷，Issue #1–#5保持开放，尚无PR，未合并/发布。
