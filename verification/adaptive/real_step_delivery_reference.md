# 合成输入下真实Adaptive完整交付链参考

`[AI-DRAFT + HUMAN-APPROVE]`，2026-11-18，L3候选/未冻结；关联AD-G05、SN-G06、P8.1–P8.7、TC-F-ADAPT-001/002/003、TC-F-EXP-001/002/003与TC-I-RESULT-001。真实编译/执行另行登记，不把源码检查记为通过，也不改变TC/AC或DEC-005。

本测试组合此前分离的真实数值Session和完整Runner/磁盘交付链。`ProductionExperimentRunner`使用显式合成轨迹、运动、近度、12视线及固定RNG输入；内部真实FlowSynth、MeanMagnitudeGaussianNoise、Sophus上一帧旋转、Adaptive Step/Session、八轮聚合、Writer/Manifest均保留。Step注入器只记录并转交真实求解结果，不代造运动。它不是原始单位球轨迹、原实验采样规模或L4统计实验；输出有`SYNTHETIC_INPUTS.txt`及合成provenance，不能用于宣称原科学实验成功。

## 源顺序和独立性

初始化仍35帧/1轮，Comparison仍600帧/8轮，UpdateRates仍600帧/1轮；有效内帧为33、4784、598。每帧分别2/2/4次比较和1次主求解，预期真实Step共17441次。采样器检查原请求level2/4，再返回明确的12视线固定输入；没有改生产采样器、循环或原实验参数。

原`.m`仅静态核对：三入口blob依次为`a3154ae391be82ccf48c9f3a03c55109dde85f65`、`c9548c83fd0a2ce13d96394e98f6c7573119a7d5`、`4629901c1233cdee5d6d22c917603720ffd1fab4`；`nonpropfehler.m`为`c25d4743f5dc99b4825b738d822457d541813350`。Step、投影、原系数动作和独立Session来源沿用[多帧参考](session_multiframe_reference.md)；未运行MATLAB/Octave。

Expected使用标准库long double标量独立参考：自己计算理想场、投影噪声、Rodrigues旋转、原系数多项式作用、Gaussian消元、每轮自身系数状态、角度和最终聚合。没有读取生产流/状态/输出作为Expected。输出角度独立采用`atan2(norm(cross),dot)*180/pi`；生产仍是未钳制acos。主状态按每轮首帧重置，四基线周期1/5/11/21、主求解前刷新与比较dumi弃置均继承独立Session。

## 可复现固定输入

以下`r`为C++内帧索引1…N−2，`j`为0-based重复序号。位置为`(r/1024,1/8,1/4)`，绝对旋转0；真平移`(1+(r+1)/128,1/4,-1/2)`，真旋转向量`(1/8,1/4+(r+1)/1024,1/2)`。第0帧运动为0；上一帧旋转自行从`w(r−1)`计算。这些位置、运动和近度是解耦测试输入，不声称物理轨迹一致。

视线取十二个二十面体顶点：`phi=(1+sqrt(5))/2, L=sqrt(1+phi²)`，循环c=0,1,2及a/b=−1,+1，置`d[(c+1)%3]=a/L,d[(c+2)%3]=b*phi/L`，其余为0；先舍入binary64，同值送两路。近度也先舍入binary64：

```text
mu(j,r,d)=1+j/4+(r+1)/128
          +0.11*x-0.07*y+0.05*z+0.03*x*y-0.02*y*z+0.04*x*z
          +0.015*(3*z*z-1)+0.025*(x*x-y*y)+0.06*x*z*z+0.025*y*x*x
```

末两项使场含二阶表示以外的成分，避免测试只覆盖可完美拟合的场。Comparison使用binary64系数0.1，固定normal样本`g[k]=(((17*k+13) mod 29)-14)/8`；每帧36次，起点`36*(j*598+r-1)`，8轮连续共172224次，无uniform/reseed。它们是可辨识固定输入，不宣称Gaussian分布或MATLAB同seed。

独立聚合累加每帧比较角度，Comparison四错误数组最后除以固定8，`rottransratio`只保存第8轮的理想场分量比。近度随j变化并检查第1/8轮ratio有区别，以发现错误平均/首轮覆盖；没有缺样本改除数。UpdateRates四列按MATLAB列主序保存，akrot为真旋转范数转度。12个MAT含8540个数值，端点严格0；不保存HistoricalOnly字段。

## 检查与证据

CTest `r06_adaptive_real_step_delivery_l3`（[源码](../../tests/integration/test_r06_adaptive_delivery_l3.cpp)）在独占Unicode沙箱执行。流分量沿用绝对1e-12，Step/输入系数沿用`2e-8*(1+abs(reference))`，输出角度/ratio沿用既有Runner绝对1e-9工程余量；不新定/放宽正式数值阈值。原轨迹Incomplete/UndefinedAngle回归保留。

`step-nodes.jsonl`记录mode、1-based run/frame、0-based slot、9输入系数、113个Expected/Actual节点，依次为mean/MSE、四个12项near数组、6×6行序矩阵、rhs6、解6、平移3、旋转3、下一组9系数。每次真实Step之后先保存节点再比较。`published-values.jsonl`保存每个字段的维度、文件SHA和全部Expected/Actual值。磁盘MAT由生产Reader与独立标准库/zlib v5解析器分别读取，逐位比对，再按独立数值参考比较全部元素；核对durable receipt、文件大小/SHA、Manifest Complete/计数/源码身份。

本测试不替代完整原场景、L2同分布对照、病态资格、L4或正式Golden批准。真实原轨迹的帧14 acos越界与帧151零旋转仍需如实保留，不能用合成成功覆盖。
