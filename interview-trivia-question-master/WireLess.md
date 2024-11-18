# 信通相关

# 天线的传输距离跟什么因素相关

1. [**频率**：无线电波的频率会影响传输距离。较高频率的信号（如微波）通常传输距离较短，因为它们对衰减和环境障碍更敏感](https://zhuanlan.zhihu.com/p/651238403)[1](https://zhuanlan.zhihu.com/p/651238403)。
2. [**天线特性**：天线的设计和增益对传输距离有重要影响。高增益天线可以扩展覆盖范围，从而实现更长的传输距离](https://zhuanlan.zhihu.com/p/345414916)[2](https://zhuanlan.zhihu.com/p/345414916)。
3. [**发射功率**：发射机的射频输出功率越大，传输距离越远](https://zhuanlan.zhihu.com/p/345414916)[2](https://zhuanlan.zhihu.com/p/345414916)。
4. [**接收灵敏度**：接收机的灵敏度越高，能够捕捉到的微弱信号越多，从而增加传输距离](https://zhuanlan.zhihu.com/p/345414916)[2](https://zhuanlan.zhihu.com/p/345414916)。
5. [**环境条件**：物理环境如建筑物、树木和地形等障碍物会影响无线电波的传播，从而影响传输距离](https://zhuanlan.zhihu.com/p/651238403)[1](https://zhuanlan.zhihu.com/p/651238403)。
6. [**干扰**：来自其他设备的电磁干扰会降低传输距离。抗干扰能力强的系统可以在干扰环境中保持较远的传输距离](https://zhuanlan.zhihu.com/p/345414916)



# 信道容量

$$
C = B * log_2(1 + \frac{S}{N})
$$

$$
C = B * log_2(1 + \frac{S}{n_0} * B)
$$

- B：带宽
- S/N：信噪比

带宽无限大，信道容量并不能无限大！上式表明，当给定S/ n0时，若带宽B趋于无穷大，**信道容量不会趋于无限大，而只是S/ n0的1.44倍。这是因为当带宽B增大时，噪声功率也随之增大**。



# 无线通讯标准

[IEEE 802.11标准是为无线局域网（WLAN）制定的一系列技术规范，涵盖了媒体访问控制（MAC）层和物理层（PHY）协议](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)[。这些标准为Wi-Fi设备提供了基础，使得设备可以在不同频段（如2.4 GHz、5 GHz、6 GHz和60 GHz）上进行无线通信](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。

以下是一些主要的802.11标准及其特点：

- [**802.11b**：工作在2.4 GHz频段，最大传输速率为11 Mbit/s](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。
- [**802.11a**：工作在5 GHz频段，最大传输速率为54 Mbit/s](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。
- [**802.11g**：将OFDM技术引入2.4 GHz频段，最大传输速率为54 Mbit/s](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。
- [**802.11n（Wi-Fi 4）**：支持MIMO技术，工作在2.4 GHz和5 GHz频段，最大传输速率为600 Mbit/s](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。
- [**802.11ac（Wi-Fi 5）**：工作在5 GHz频段，支持MU-MIMO技术，最大传输速率可达6933 Mbit/s](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。
- [**802.11ax（Wi-Fi 6）**：工作在2.4 GHz、5 GHz和6 GHz频段，最大传输速率可达9608 Mbit/s](https://en.wikipedia.org/wiki/IEEE_802.11)[1](https://en.wikipedia.org/wiki/IEEE_802.11)[2](https://standards.ieee.org/standard/802_11-2020.html)。



# MIMO技术

MIMO技术：多输入多输出技术

### MIMO的主要特点和优势

1. [**提高系统容量**：MIMO技术可以在不增加带宽的情况下，通过多条独立的信道同时传输多个数据流，从而显著提高系统的传输速率](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[1](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[2](https://zhuanlan.zhihu.com/p/356120062)。
2. [**增强信号覆盖范围**：利用多天线的空间分集技术，MIMO可以在不同路径上发送和接收信号，提高信号的可靠性和覆盖范围](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[1](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[2](https://zhuanlan.zhihu.com/p/356120062)。
3. [**改善信噪比（SNR）**：通过多天线的协同工作，MIMO系统可以有效地减少干扰和噪声，提高信号质量](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[1](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[2](https://zhuanlan.zhihu.com/p/356120062)。

**空间分集**：通过对同一数据流制作不同版本，分别在不同天线进行编码、调制，然后发送。**空间分集技术可以更可靠地传输数据**。

**空分复用**：空分复用技术是指将需要传送的数据分为多个数据流，分别通过不同的天线进行编码、调制，然后进行传输，从而提高系统的传输速率。



### MIMO的类型

1. [**SU-MIMO（Single-User MIMO）**：单用户MIMO，指的是在一个通信链路上使用多天线进行数据传输](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[1](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[2](https://zhuanlan.zhihu.com/p/356120062)。

2. [**MU-MIMO（Multi-User MIMO）**：多用户MIMO，允许多个用户同时共享同一个无线信道，提高了系统的整体效率](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[1](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[2](https://zhuanlan.zhihu.com/p/356120062)。

   > 允许发射端同时和多个用户传输数据。[Wi-Fi](https://info.support.huawei.com/info-finder/encyclopedia/zh/WiFi.html) 5标准开始支持4用户的MU-MIMO，[Wi-Fi 6](https://info.support.huawei.com/info-finder/encyclopedia/zh/WiFi+6.html)标准将用户数增加到了8个。

3. [**Massive MIMO**：大规模MIMO，使用大量天线（如64、128或更多）来进一步提升系统容量和传输效率，是5G技术的核心组成部分](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[1](https://info.support.huawei.com/info-finder/encyclopedia/zh/MIMO.html)[2](https://zhuanlan.zhihu.com/p/356120062)。

