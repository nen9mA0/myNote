## Lines of Malicious Code: Insights Into the Malicious Software Industry

### System Description

BEAGLE有四个模块

* Execution Monitoring  用于记录每个样本运行时行为的沙箱（如调用栈、system call、socket、注册表的修改、文件创建等）
* Behavior Extraction  分析这些log并使用一系列启发式算法和规则提取高层次的行为
* Binary Comparison  反汇编并脱壳二进制代码，并分析每个样本对程序CFG的增加、删除、修改，以及CFG相同的部分，并将相同的部分做上标记并提取行为
* Semantic-Aware Comparison  检测标记的代码是如何演变的

#### Execution Monitoring

这里使用了一个比较有趣的方法来获取同一个病毒样本的不同版本，方法就是让这个病毒在机子上一直运行，并且保存每次升级的文件

##### Stateful Sandbox

一般的sandbox实现都会在一个干净的环境下运行恶意软件，每次运行时系统都会被重置到某个snapshot处以保证之前的运行结果不会影响当前样本的运行。但因为Beagle需要模拟恶意软件安装和升级的行为（因为通常恶意软件在第一次运行时会首先试图驻留在系统中，之后可能才会展现出其他恶意行为），所以这里的做法是每次都以一个patch的形式记录恶意软件对文件系统和注册表的修改（相对于原本干净的环境）。每次需要重新执行恶意软件时就将这个patch应用到沙箱上，使得沙箱可以模拟上次被恶意软件修改后的环境；此外作者发现样本的驻留机制就是对几个常见的auto-start位置进行监控，因此触发一次上述驻留机制，就可以模拟一次reboot，进而分析升级后的样本。

由此，该环境可以在第一次执行样本时让样本安装并记录原始样本的功能，在之后的执行中则可以分析升级后的变化，以及其他后继的升级版本

##### Generic Unpacking

Beagle会在样本将要退出时dump下整个内存的镜像，包括该样本启动的其他进程以及注入代码的进程的镜像。

##### Sandbox Implementation

Beagle使用的sandbox是基于qemu的Anubis。它除了可以提供API级别的行为检测、记录API调用和系统调用（包括参数），还提供了一套动态污点分析算法以获取上述调用间的数据流依赖关系。

此外为了更好地分析行为与对应代码的关系，Beagle扩展了记录每个API调用栈的功能

### Binary Comparison

#### Code Fingerprints

用的是CFG匹配，算法是k节点的同构子图，但为了效率，这里近似成了提取CFG的一个k节点连通子图并归一化，每个被归一化的子图都可以视作代码片段的简洁表示；对其计算哈希，生成一个指纹，再通过对比不同样本代码片段的指纹来检测相似代码片段。

对于一个脱壳后的样本M，按上面步骤计算其CFG的指纹集合 $F_M$ ，对于每个指纹f，保存其对应basic block的地址 $b_M(f)$ 。由此可以生成一个指纹与basic block绑定的集合 $B_M(F) = \bigcup_{f \in F} b_M(f)$ 。

为了比较样本M和N，首先取指纹的交集 $I_{M,N} = F_M \cap F_N$ ，则对应这些指纹的basic block被认为是共享的basic block

所以对于样本M，共享的basic block为 $S_M(M, N) = B_M(I_{M,N})$ ，同理对于样本N，为 $S_N(M, N) = B_N(I_{M,N})$ 

若N是M的变种，则 样本N相对M加入的basic block可以表示为 $A_N(M, N) = B_N \setminus S_N(M,N)$ ，样本M相对N被移除的basic block可以被表示为$R_M(M, N) = B_M \setminus S_N(M,N)$

（吐槽：有点服气了这么简单的道理要搁这形式化成集合然后bb那么多一堆，做完笔记发现不如不做，不过看在下面相似度度量函数要用到勉为其难记一下吧）

#### Code Whitelisting

如上所述，由于Beagle会保存被注入的进程的内存dump，且一些恶意样本会通过动态加载DLL的方式来使用一些API。若将这些标准库的函数也列入分析中会干扰结果、降低效率。Beagle的做法就是对干净的环境中各个exe和dll的basic block先做一个指纹库作为白名单，若之后匹配到白名单内的代码则直接略过

#### Similarity and Evolution

使用Jaccard集合相似度算法来计算相似性
$$
J(M, N) = \frac{|S^*(M, N)|}{|S^*(M, N)| + |A(M, N)| + |R(M, N)|}
$$
注意，由于同一个二进制文件里可能出现相同的k-node子图，因此 $|S_M(M,N)|$ 与 $|S_N(M,N)|$ 可能不一致，因此上面带星号的表示取两者的最大值

此外为了比较样本间不同的部分，假设样本M经历了t代变种，则计算前t-1代的指纹库，并识别新的basic block为
$$
N_{M_t} = B_{M_t} \setminus B_{M_t}(F_{M_1} ... F_{M_{t-1}})
$$

### Behavior Extraction

#### Unlabeled Behavior

这里指代的是不符合之前其他行为分析相关的方法和论文中列出的pattern的行为。

检测的方法是通过数据流依赖关系图找到的一系列连续的API调用或系统调用。此外由于数据流依赖关系可能会因为文件写入而丢失（如一段程序写一个cfg文件，而另一段程序读cfg文件，两者实际上存在数据依赖），因此Beagle会将对同一资源操作的函数视为存在数据流依赖

#### labeled Behavior

当一系列具有数据流依赖的API调用或系统调用满足一些人工写的规则时，就是labeled behavior

### Semantic-Aware Comparison

#### Mapping Behavior to Code

Beagle先记录API的log，然后通过调用栈等信息，结合静态分析找到各个API对应的调用函数，并对这些函数打上tag。其中这里一个函数可能有多个tag，因为同一个函数可能调用不同的API

#### Behavior Evolution

通过比较不同版本的恶意软件间的行为，可以获取恶意软件在功能上具体的升级。这里用于比较各个版本代码区别的方法就是 [Binary Comparison](#Binary Comparison) 中提到的算法

#### Dormant Functionality

Beagle与其他动态分析工具一样存在着覆盖率的问题，比如一些恶意软件的功能只有在特定条件下才会被触发，这可能会导致无法收集恶意软件在升级前后的信息（因为可能触发条件较苛刻，一些功能只有在某个版本中被触发过）。因此Beagle使用了这样一种方法：在执行的时候也对当前的 [Code Fingerprints](#Code Fingerprints) 进行检查并记录执行过的函数的指纹，若发现某个函数只被执行过一次，那么可以通过这个函数对应的指纹来在其他版本的恶意软件中定位到对应的函数，从而进行进一步分析

### Evaluation

#### Setup

这里的测试方法是让恶意软件每天在沙箱中运行15分钟，且每次运行都会保留该恶意软件之前的修改，在 [Stateful Sandbox](#Stateful Sandbox) 中提到过

这里为了防止运行恶意软件时产生危害，采用了类似限制带宽和连接、将特定协议的数据重定向到蜜罐等措施

#### Code Behavior

根据规则和先验知识，恶意代码行为的分类主要有下面几种

##### Install

* AUTO_START  对一些auto start的位置的修改

##### Networking

* SPAM  垃圾邮件，这里所有在25端口上的数据都被视为SPAM
* HTTP_REQUEST
* HTTPS_REQUEST
* DNS_QUERY
* TCP_TRAFFIC
* UDP_TRAFFIC
* LOCAL_CONNECTION
* OPEN_PORT

##### Download & Execute

* DOWNLOAD_EXECUTE  下载文件并执行
* DOWNLOAD_INJECT  下载一段数据并注入到其他进程

##### Information Stealing

* FTP_CREDENTIALS  从注册表或文件系统获取FTP凭证
* EMAIL_HARVESTING  从注册表或文件系统获取邮箱地址
* BITCOIN_WALLET  搜索比特币钱包
* INTERNET_HISTORY  获取浏览器历史记录

##### Fake AV

恶意软件故意修改一些系统配置使得用户需要购买一些软件来修复这些问题

* DISABLE_TASKMGR  
* HIDE_STARTMENU
* HIDE_FILES

##### Browser Hijacking

修改浏览器的代理

* IE_PROXY_SETTINGS
* FIREFOX_SETTINGs

##### Anti AV

* RESTORE_SSDT  通过修复ssdt使得anti-virus软件失效

##### AutoRun

* AUTO_RUN  修改autorun.inf

##### Lowering Security Settings

* FIREWALL_SETTINGS  针对防火墙设置，如阻止新的防火墙规则，甚至直接停用防火墙
* IE_SECURIT_SETTINGS  修改注册表中IE phishing filter设置
* CHANGE_SECURITY_POLICIES  修改系统中检测从网络或邮件中下载文件的威胁度的设置

##### Miscellaneous

其他各种

* INJECT_CODE
* START_SERVICE
* EXECUTE_TEMP_FILE
* ENUMERATE_PROCESSES
* DOWNLOAD_FILE  

##### Unpacking

为了对付packing，Beagle认为在恶意软件本身中发现的行为和label都不可信，因为这些可能都只是packer本身的行为

### Limitations And Future Work

* Beagle中使用的Generic Unpacking技术较为简单
* 恶意软件运行时可以通过一些方法检测沙箱
* 动态分析方法面临的覆盖率问题
* 一些行为无法被沙箱环境分析，比如比特币钱包窃取的行为，因为沙箱中没有对应环境所以无法实现
* Beagle只能分析版本间的区别，但无法分析语义



### 感兴趣的论文

* Anubis 论文中使用的sandbox

  U. Bayer, C. Kruegel, and E. Kirda. TTAnalyze: A Tool for Analyzing Malware. In EICAR Annual Conf., 2006

  U. Bayer, P. Milani Comparetti, C. Hlauscheck, C. Kruegel, and E. Kirda. Scalable, Behavior-Based Malware Clustering. In NDSS, 2009.  

* binary comparison的算法

  C. Kruegel, E. Kirda, D. Mutz, W. Robertson, and G. Vigna. Polymorphic Worm Detection Using Structural Information of Executables. In RAID, 2005  

