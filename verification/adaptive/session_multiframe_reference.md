# Adaptive Session 多帧独立标量参考

`[AI-DRAFT + HUMAN-APPROVE]`，2026-11-18，L3候选、未冻结、未批准DEC-005。此文件说明固定测试输入与独立计算方法；实现/运行事实另记PROGRESS。不是MATLAB执行结果，也不是原35/600点轨迹或完整科学实验。

对应AD-G04/05/06、P7.7/P7.9/P8.1/P8.2/P8.3/P8.5/P8.9，以及TC-F-ADAPT-002/003、TC-NF-SESSION-001。目标是在已有重放/替身时序测试之外，用独立数值计算核对跨帧主状态、比较运动与四组周期基线。参考不以生产Session、Step、SH递推、拟合或生产历史输出作为Expected。

## 参考来源与独立性

只读原始目录为`origin/egomotion_estimation_dense_flow_fields_matlab_scripts/`。本批静态阅读的源Git blob：

| 相对路径 | blob SHA |
|---|---|
| help_functions/oneit_adaptiv_matched_filter.m | 48e51bcd52fb8f27cf35b3fce38b28507af36e55 |
| help_functions/near_aus_koeff.m | 7373c4376dfd5bbf281136ced39657d411a10ffa |
| help_functions/koeff_aus_near.m | 5ff91a2210a20e5acf79dec6e59ddd33b55529d3 |
| help_functions/transfor_koeff.m | 7f26019cab53666539e1961dac67a2e07dae614c |
| help_functions/SHRotate.m | 7b73000411a9294f6cbed2e111bf74ce7fb7d4df |
| adaptive_matched_filter_test/data_generation/agent_in_sphere_initialization_test.m | a3154ae391be82ccf48c9f3a03c55109dde85f65 |
| adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_test.m | c9548c83fd0a2ce13d96394e98f6c7573119a7d5 |
| adaptive_matched_filter_test/data_generation/agent_in_sphere_adaptive_matched_filter_many_parameter.m | 4629901c1233cdee5d6d22c917603720ffd1fab4 |

[标量参考头](../../tests/support/adaptive_scalar_reference.hpp)仅使用标准库`long double`、数组与显式标量循环；没有Eigen或任何egomotion生产头。6×6使用部分选主元Gauss-Jordan消元，生产使用Eigen FullPivLU。按源CS九个Cartesian多项式重建；逐通道拟合为`sum(Y*near)/sum(Y*Y)`，解析约去共同的`4π/N`，不改成联合最小二乘。

旋转独立写成`f(d)=c00*Y00+l·d+dᵀQd`，Q为对称无迹矩阵。源组合动作的参数矩阵为`A=S R S`、`S=diag(-1,-1,1)`，所以`l'=Aᵀl`、`Q'=AᵀQA`，再恢复九系数。没有调用IR递推、低阶旋转块或生产TensorDirect。`2+z`在Rx90后ey处的源值1、物理值3作独立相位鉴别。源动作推导的先前参考见[源SH验证](../../docs/reviews/R6_SH_SOURCE_ACTION_2026-11-17.md)。

参考自身先用已有[正/负均值Wolfram固定数值](asymmetric_signed_near_one_step.v1.json)校准六维解、MSE与九个新系数；该参考的来源脚本和局限不改变。负均值校准保持`mu=raw/abs(s)`与原M1，不强制变正。逐帧参考从自身上一帧的long double状态推进，绝不拿生产输出替换期望状态或重新起算。

## 固定输入与原索引

每种模式分别执行两条流，MATLAB帧号`i=2..43`，每条42帧。视线为12个二十面体顶点：`φ=(1+sqrt(5))/2`，`L=sqrt(1+φ²)`；cycle依次0、1、2，每cycle按sign1=-1,+1及sign2=-1,+1的嵌套次序，`d[(cycle+1)%3]=sign1/L`、`d[(cycle+2)%3]=sign2*φ/L`，其余为0。视线和生成光流在入口明确舍入为binary64，然后参考和生产都消费同一组值；没有RNG或自选种子。

令`h=i/128`。stream0的`t=(1+h,1/4,-1/2)`、`w=(1/8,1/4+h/8,1/2)`；stream1的`t=(1/4,1+h,1/8)`、`w=(-1/2,1/8,1/4+h/8)`。固定正近度：

`mu=1+stream/4+h+0.11x-0.07y+0.05z+0.03xy-0.02yz+0.04xz+0.015(3z²-1)+0.025(x²-y²)`。

光流独立由`p=-mu*(t-(t·d)d)-w×d`计算。误差比较轴stream0取`truth_t=ex, truth_w=ez`，stream1取`truth_t=ey, truth_w=ex`；这些轴故意不与信号共线，仅验证有限角度计算，**不是原轨迹科学真值**。

每帧输入的前一帧旋转矩阵按六项循环：Rx90、Rz90、Ry90、Rx(-90)、Rz(-90)、有理q=(1,2,2,0)旋转。最后一项逐行为`(1,8,4)/9`、`(8,1,-4)/9`、`(-4,4,-7)/9`，入口同样舍入binary64。包含非交换次序和源/物理相位差异；不通过生产Rodrigues生成参考。

三模式初始主/基线系数均只设C00=1：Initialization不旋转主，比较为常数与当前主；Comparison先旋转主再作同样比较；UpdateRates先旋转主和所有旧基线，再用`i mod {1,5,11,21}`决定是否复制本帧已旋转、尚未求解的主系数。比较求解的新系数`dumi`不提交；只有主求解推进主状态。该帧区间的刷新次数必须精确为42/8/3/2，覆盖i=5/11/21/42前后及21周期二次刷新。

在每条流的i=5、11、21、42前额外尝试真旋转比较轴为零的错误帧，要求生产比较已求解但角度报UndefinedAngle；状态逐位不变，也与独立上帧状态相符；然后重试相同正确帧。参考模型只推进成功序列，不假装模拟MATLAB异常事务。

## 检查与证据边界

[测试](../../tests/integration/test_r06_session_l3.cpp)以透明记录器调用真实生产Step，原样返回结果；它没有数值替身。每个比较和主求解的输入系数、raw/翻号/归一near、原均值、M1–M4/6×6、rhs、raw解、归一t/旋转、MSE、新near及九系数均与独立参考比较；还核对比较角度、提交状态、六个未用布局位置精确为0。每种模式/流另验证首帧的默认未注入Session。固定数据必须能区分错误物理相位、0-based周期偏移、把dumi写回各基线等错误。

数值工程余量沿用既有单步L3的`2e-8*(1+abs(reference))`；角度沿用既有运动误差参考的`1e-9*(1+abs(reference))`。索引、数量、刷新、失败前后状态字节均精确检查。没有新设或放宽生产门限/正式AC；通过仍不代表DEC-005已批准，不能据小型固定序列推断全部输入、完整原轨迹、病态前向误差或L4成功。

每次测试创建独占沙箱，保存`reference.jsonl`与`actual.jsonl`的全部252成功帧，包含每帧输入、比较后主调用次序、逐节点数值、主/基线状态及角度；失败沙箱保留。外层受授权运行记录保存分支/SHA、源与测试文件hash、工具链、命令、stdout/stderr/退出码和JUnit。参考数据/文件身份是本轮候选证据，不是已冻结Golden。
