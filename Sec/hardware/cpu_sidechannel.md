[Survey of CPU Cache-Based Side-Channel Attacks: Systematic Analysis, Security Models, and Countermeasures (hindawi.com)](https://www.hindawi.com/journals/scn/2021/5559552/)

### Introduction

主要贡献

* 归纳总结了先前对CPU cache的侧信道攻击的workflow，并分为几类通用的攻击模型

* 攻击模型由4个向量描述：vulnerability, cache type, pattern, range，这4个向量可以较为清晰地描述安全威胁

* 对防御阶段进行了划分，并分析了哪些防御阶段可以更好地提供防护

### Side-Channel Attacks

#### Attack Workflow

攻击主要可以分为4个阶段

* 定义攻击者和受害者间的连接渠道。在基于cache的侧信道攻击中，cache就是攻击者与受害者间的连接渠道，因此需要进一步寻找两者在cache中的联系
  
  * 如OpenSSL在作为函数库跨进程共享时，在一些情况下会占据完全相同的cache
  
  * 如VMWare Xen等会使用large page管理guest的物理内存，攻击者可以通过large page中物理内存与虚拟内存的关系进行攻击

* 攻击者在受害者运行时收集cache的动作，以此来推断受害者的状态。一般来说攻击者会在初始化攻击时对cache设置一个初始状态，如使用CLFLUSH将某些内容请出缓存

* 


