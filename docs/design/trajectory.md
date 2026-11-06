# 轨迹、相机旋转与球内环境——详细设计草案

> **未冻结、未运行数值测试。** 来源：kreis_trajektorie.m、sinuskurve_trajektorie.m、trajektorie_aus_punkten.m、drehvektor_aus_zwei_vektoren.m、rodrigues.m、getnear_in_kugel_Drehung.m。对应 P3.6、P6.1–P6.7、REQ-F-TRAJ-001。

## 1. 轨迹点列与离散化

令 MATLAB 的 i=1…stellen，对应 C++ 0-based j=i−1。

**圆轨迹**（半径 r=parameter·kugelradius，源半径为 1）：

\[
w_j=2\pi j/\mathrm{stellen},\quad
\boldsymbol q_j=(r\cos w_j,r\sin w_j,\mathrm{hoehe}).
\]

**正弦轨迹**：

\[
w_j=2\pi j\,\mathrm{anzahlperioden}/\mathrm{stellen},
\quad
q_x=\mathrm{laenge}\,j/\mathrm{stellen}-\mathrm{laenge}/2,
\quad q_y=\mathrm{amplithude}\sin w_j,\quad q_z=\mathrm{hoehe}.
\]

注意分母是 **stellen**，不是 stellen−1，不能自行补上“重复的末点”。原函数绘图 line() 属于显示副作用，C++ 轨迹模块必须只返回点列。

## 2. 轨迹到局部运动

对应 trajektorie_aus_punkten.m 的有效索引 i=2…N−1：

\[
\boldsymbol v_1=\boldsymbol q_i-\boldsymbol q_{i-1},\qquad
\boldsymbol v_2=\boldsymbol q_{i+1}-\boldsymbol q_i.
\]

相对旋转向量由 drehvektor_aus_zwei_vektoren(v1,v2) 得到；绝对朝向旋转向量由起始前向 (1,0,0)^T 到 v1 得到。以该朝向的 Rodrigues 旋转矩阵 R，将世界平移/相对旋转转换到原脚本的局部坐标：
\[
\boldsymbol t_i=R^\top\boldsymbol v_2,\quad
\boldsymbol\omega_i^{rel}=R^\top\mathrm{rotationVectorBetween}(\boldsymbol v_1,\boldsymbol v_2).
\]

原始函数初始化数组为 N×3，首尾行未在主循环填写，C++ 不能擅自用外推补齐而声称原版等价。接近共线、反向或零方向的行为必须记录原 MATLAB 结果与拟议数值状态。

## 3. 单位球内射线近度

将视线 d 通过 Rodrigues 转动得到 n，位置 o 在单位球内。源码：
\[
b=\boldsymbol n^\top\boldsymbol o,\quad
c=\boldsymbol o^\top\boldsymbol o,\quad
\Delta=b^2-c+1,\quad
\rho=-b+\sqrt\Delta,\quad \mu=1/\rho.
\]
测试 o=0、接近球面、反向视线和非法球外位置；不能静默返回非有限 near。

## 4. 候选类型、失败及测试

~~~cpp
// 接口草案（没有对应源码）
struct TrajectoryPoints; // 自拥有 N×3 double 值数组
struct FrameKinematics {
    Eigen::Vector3d local_translation;
    Eigen::Vector3d absolute_rotation;
    Eigen::Vector3d relative_rotation;
};
class TrajectoryGenerator {
public:
    Result<TrajectoryPoints, GeometryError> circle(
        std::size_t count, double radius, double height) const;
    Result<TrajectoryPoints, GeometryError> sinusoidal(
        std::size_t count, double amplitude, double length,
        std::size_t periods, double height) const;
};
class TrajectoryKinematics {
public:
    Result<std::vector<FrameKinematics>, GeometryError>
      fromPoints(const TrajectoryPoints&) const;
};
~~~

责任边界：TrajectoryGenerator 不计算 MAT 数据或绘图；TrajectoryKinematics 不管理 Adaptive Session；Rodrigues 的第三方代码需做版权与算法来源核查。

TC-F-TRAJ-001：固定参数点列、局部 t/ω、首尾行为、奇异方向状态对照；实际 Golden 数值、精确签名与容差待审定。
