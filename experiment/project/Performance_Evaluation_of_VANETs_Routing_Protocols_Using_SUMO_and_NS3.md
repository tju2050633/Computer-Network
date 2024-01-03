### Abstract

评估比较面向VANET的路由协议，包括AODV、DSDV、OLSR、GPCR和GPSR。用SUMO仿真城市交通移动性场景，用NS3仿真VANET环境下的路由协议。研究节点密度对各种VANET路由协议性能的影响。

使用SUMO生成了一个摩洛哥Oujdacity的真实城市场景的交通和移动性模型,作为NS-3仿真的输入。

**SUMO**：一个用于模拟城市交通流的开源交通模拟工具。它是一个广泛使用的仿真平台，用于研究和分析交通系统、交通流、车辆行为以及交通管理策略的影响。

AODV、DSDV、OLSR是**基于拓扑**的路由，GPCR和GPSR是**基于位置**的路由：

1. **GPCR（Geographic and Opportunistic Routing）**：

   - **基于的路由算法**：地理路由和机会路由

   - **定义：** GPCR是一种地理和机会路由协议，结合了位置信息和机会式通信的概念。它充分利用节点之间的地理位置信息，并通过机会性地传递数据，以提高路由效率。

   - **工作原理：** GPCR利用节点之间的相对位置信息来选择下一跳节点。当节点之间有机会直接通信时，数据就会直接通过机会路径传输，而不依赖于先前建立的路由路径。这种机会式通信减少了路由维护的开销，并能够更好地适应动态网络环境。

2. **GPSR（Greedy Perimeter Stateless Routing）**：

   - **基于的路由算法**：贪心算法

   - **定义：** GPSR是一种基于贪心算法的位置无关路由协议，特别适用于自组织网络中。它不依赖全局网络状态，而是根据节点的相对位置进行贪心式的路由选择。

   - **工作原理：** 在GPSR中，当一个节点要发送数据时，它首先选择相邻节点中距离目标最近的节点作为下一跳。这个过程会一直重复，直到数据到达目标。如果直线路径上有阻碍，GPSR会选择绕过这些障碍物，采用沿着网络边缘的贪心周长路由。



### Experiment

1. 仿真环境

- 使用NS-3作为网络仿真器,版本为NS-3.25
- 使用SUMO作为交通流仿真器,版本为SUMO 0.25
- 从OpenStreetMap下载了一个摩洛哥Oujdacity的真实城市地图

2. 仿真配置

- 节点数:20、30...90
- 仿真时间:100秒
- 仿真区域:1.7km * 1.5km
- 数据包大小:512字节
- 传输协议:UDP
- 最高速度:20m/s
- MAC层协议:IEEE 802.11p

3. 评价指标

- 数据包递送比(PDR)
- 平均端到端延迟
- 吞吐量
- 开销

4. 仿真过程

- 使用SUMO生成节点的移动性模型
- 将SUMO的输出作为NS-3的输入进行网络仿真
- 比较不同节点密度下AODV、DSDV、OLSR、GPSR和GPCR这5种路由协议的性能指标

5. 结果分析

- 对比分析了上述指标随节点密度变化的性能曲线
- 总结分析了各协议的优劣势

**Conclusions**

1. **OLSR协议在该仿真场景下,在数据包递送比和吞吐量方面表现最好。**
2. **GPSR和GPCR这两种基于位置的路由协议,在平均端到端延迟和开销方面性能最佳。**
3. 当节点密度增加时,DSDV和OLSR的数据包递送比有所提高,而AODV的则下降。
4. OLSR的平均端到端延迟最高,而GPSR和GPCR的延迟最小,并且随着节点密度的增加变化不大。
5. 在低密度时AODV的延迟较低,但当节点密度增加时,DSDV的性能超过了AODV。
6. OLSR的吞吐量最大,可以达到18kbps,优于DSDV和AODV。
7. GPSR和GPCR的控制开销最小。
8. AODV的控制开销最大,因为它需要广播大量控制消息来维护路由。