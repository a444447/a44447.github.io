# LED物理世界对抗性样本

## Methods

设有$M$个可控 LED（编号 $j=1,\dots,M$），每个 LED 的物理可控参数为亮度与 RGB 三通道，共 4 个连续量：
$$
x_j = [b_j, r_j, g_j, b_j]^\top \in [0,1]^4,
$$
其中 $b_j$ 表示亮度（brightness），$r_j,g_j,b_j$ 三个为颜色通道。整个系统的参数向量记为
$$
\mathbf{x} = [x_1^\top, x_2^\top, \dots, x_M^\top]^\top \in [0,1]^{4M}.
$$
受攻击模型为黑盒，只能查询模型输出（例如检测器的平均置信度或分类器的 Top-1 置信度）。定义适应度（目标）函数 $F(\mathbf{x})$ 为我们要最小化的指标（例如目标检测平均置信度的加权和或分类 Top-1 置信度），即：
$$
\min_{\mathbf{x}\in[0,1]^{4M}} F(\mathbf{x}).
$$
为提高搜索效率，我们将优化分为两阶段：先用 PSO 在简化空间搜索“要点亮哪些 LED（子集）”，再用 CMA-ES 在所选子集上做连续参数细调。

### 阶段一 — Coarse：PSO 子集搜索（子集选择）

目标：在低维布尔空间中选出一个影响力大的 LED 子集 $S\subseteq\{1,\dots,M\}$。

#### 1.1 问题简化

为降低维数，我们将每个 LED 的颜色与亮度固定为一个预设值（例如紫色 $x^{(0)}$），仅优化每个 LED 的开关状态 $s_j\in\{0,1\}$。用向量 $\mathbf{s}=[s_1,\dots,s_M]$。

在实现上，可以用连续编码 $y_j\in\mathbb{R}$（或 $[0,1]$）并以阈值方式得到二值：
$$
s_j = \mathbb{I}[y_j > \tau_{\text{th}}],\quad \tau_{\text{th}}=0.5.
$$

#### 1.2 Ibest PSO 

我们采用 l-best PSO（局部邻域拓扑）以鼓励多样性，速度/位置更新为经典形式（使用 constriction 因子 $\kappa$）：
$$
\mathbf{v}_i^{t+1} = \kappa\big(\mathbf{v}_i^{t} + c_1 r_1(\mathbf{p}_i^{t}-\mathbf{x}_i^{t}) + c_2 r_2(\mathbf{\ell}_i^{t}-\mathbf{x}_i^{t})\big),
$$
其中 $i$ 表示粒子索引，$\mathbf{p}_i^t$ 为粒子历史个人最优（pbest），$\ell_i^t$ 为其邻域最优（lbest），$r_1,r_2\overset{\text{iid}}{\sim}\mathcal{U}(0,1)$。

#### 1.3离散化与子集输出

粒子位置 $\mathbf{x}_i$ （维度为 $M$，代表开/关概率）在每次评估时通过阈值/采样映射为二值 $\mathbf{s}$，并以固定颜色 $x^{(0)}$ 组合还原为物理参数用于仿真或真实评估。最终输出候选子集集合（取若干 top-k 粒子最优子集）供 Fine 阶段逐一细调。

### 阶段二 — Fine：CMA-ES 连续参数细调

目标：对候选子集 $S$ 中的 LED 做连续参数 $\in[0,1]^{4|S|}$ 的精调，最小化 $F$。

#### 2.1 实数域搜索与 Sigmoid 映射

为不直接破坏 CMA-ES 的自适应统计特性，在内部在无界实数域 $y\in\mathbb{R}^{d}$（$d=4|S|$）中搜索，并通过 sigmoid 映射到物理区间：
$$
x = \sigma(y) \equiv \frac{1}{1+\exp(-y)} \in (0,1).
$$
为让搜索覆盖近 $[0,1]$ 边界，可以在 $y$ 空间使用较大范围（例如初始均值可设在 [-1,1]\]，或更宽 \([-3,3]\]），并通过缩放使映射分布合适。实现细节：在文中我们建议搜索区间 \([-6,6]（经验值），以便 sigmoid 可实现接近 $[0,1]$的值。

#### 2.2 CMA-ES 基本步骤（简记）

采用标准 CMA-ES 采样与更新，核心为：

- 采样：

$$
\mathbf{y}_i^{t+1} \sim \mathcal{N}(\mathbf{m}^t, (\sigma^t)^2 \mathbf{C}^t),\quad i=1,\dots,\lambda,
$$

- 选择并更新均值：

$$
\mathbf{m}^{t+1} = \sum_{k=1}^{\mu} w_k \mathbf{y}_{k:\lambda}^{t+1},
$$

- 协方差矩阵与步长更新（rank-one 和 rank-$\mu$ 更新，CSA）：

$$
\mathbf{C}^{t+1} = (1-c_1-c_\mu)\mathbf{C}^t + c_1 \mathbf{p}_c \mathbf{p}_c^\top + c_\mu \sum_{k=1}^{\mu} w_k \mathbf{y}_{k:\lambda} \mathbf{y}_{k:\lambda}^\top,
$$

（以上变量符号与 Hansen 标准定义一致；）

#### 2.3 k-random-start（多次重启）

为了增强稳定性并减少对初值敏感性，对同一候选子集 $S$ 进行 $k$ 次独立 CMA-ES 运行（不同初始 $\mathbf{m}^0$、初始步长 $\sigma^0$ 或随机种子），取 $k$ 次中最优结果：
$$
\hat{\mathbf{x}} = \arg\min_{i=1,\dots,k} F\big(\sigma(\mathbf{y}^{(i)}_{\text{best}})\big).
$$
建议 $k=3\sim 7$（经验），默认k=3。

------

###  多任务（检测 + 分类）冲突检测与动态加权

若同时攻击检测器（det）和分类器（cls），定义两任务的适应度为 $F_\text{det}(\mathbf{x})$ 与 $F_\text{cls}(\mathbf{x})$。我们采用加权和作为单目标化策略：
$$
F(\mathbf{x};\alpha) = \alpha\, F_\text{det}(\mathbf{x}) + (1-\alpha)\, F_\text{cls}(\mathbf{x}),
$$
其中权重 $\alpha\in[0,1]$ 动态调整。

#### 3.1 适应度变化率与冲突判定

在迭代过程中记录两个任务在过去 $w$ 代的适应度变化向量：
$$
\Delta_\text{det} = [\delta^{t-w+1}_\text{det},\dots,\delta^t_\text{det}],\quad \delta^k_\text{det}=F_\text{det}^{k-1}-F_\text{det}^{k},
$$
同理 $\Delta_\text{cls}$。计算 Pearson 相关系数 $r$：
$$
r = \mathrm{corr}(\Delta_\text{det},\Delta_\text{cls}).
$$
当 $r < -\rho_{\text{th}}$ （例如 $\rho_{\text{th}}=0.3$）说明两任务变化率呈显著负相关，产生冲突。

#### 3.2 动态权重更新策略

若检测到冲突（$r<-\rho_{\text{th}}$），按下列规则缓解：将权重 $\alpha$ 向上一次在该任务上收益更高的一侧移动（或使用投影思想）：
$$
\alpha^{t+1} = \alpha^t + \eta \cdot \frac{\overline{\delta}_\text{det}-\overline{\delta}_\text{cls}}{|\overline{\delta}_\text{det}|+|\overline{\delta}_\text{cls}| + \epsilon},
$$
其中 $\overline{\delta}_\text{det}$ 与 $\overline{\delta}_\text{cls}$ 为窗口内的平均变化量，$\eta$ 为步长（例如 $\eta=0.1$），并将 $\alpha^{t+1}$ 裁剪到 $[0,1]$。此策略会把搜索轻微偏向近期改进更明显的任务，从而避免一边过度牺牲另一边。

（该黑盒近似方法受限于“变化率能否良好估计梯度冲突”的假设，但在实践中对稳定多任务黑盒优化常有正面效果。）