**注：由于这本跟GPU高性能编程是一起看的所以很多重复内容我写在那了**

## 使用cuda进行异构并行编程

### 并行化计算

分为数据并行化和任务并行化

![](pic\PCCP_1.png)

### 异构计算

#### API

CUDA的API主要分为drvier API和runtime API

![](pic\PCCP_2.png)

一般来说runtime API提供更高层的功能，但需要注意的是两个函数集是互斥的，即不能同时使用

#### 编译

![](pic\PCCP_3.png)

nvcc编译器使用LLVM

## cuda编程模型

### cuda编程架构

![](pic\PCCP_4.png)

### 内存管理

* cudaMalloc
* cudaMemcpy
* cudaMemset
* cudaFree

### 组织线程

```c
//假设数据量大小为n，一个block内thread数指定为m（均为一维）
size = m;
dim3 block(3);
dim3 grid( (size + block.x - 1) / block.x );
```

注意这里的运算相当于向上取整

#### 主要流程

（很多内容可以见GPU高性能编程）

确定大小的主要流程：

* 确定一个block中的thread个数
* 根据数据量来计算block个数

确定维度的主要流程

* 根据kernel的特性
* 根据GPU的资源

### 验证kernel

除了调试工具外有两个好办法

* 在kernel内使用printf
* 将启动kernel的维度设置为`<<<1,1>>>`，来验证单次计算的结果

### 处理错误

书里定义的CHECK宏与**CheckCudaError**相同

### 计算kernel运行时间

#### nvprof

可以使用nvprof工具来计算包括内存传输时间和kernel运行时间等

#### 理论速度计算

例子为书中的单精度浮点矩阵加法

器件为Tesla K10

```
FLOPS: 每秒浮点运算次数
  745MHz *   2   *   (  8     *     192  )                  * 2 = 4577280M  约为与4.58TFLOPS
  主频   板载2块GPU  一个GPU8个SM  每个SM 192个浮点运算单元   每个时钟周期2次操作
  
Bandwidth: 内存带宽
  2     *    256     *     2500M    *    2      /       8  = 320GB/s
  2个GPU  每次传输256bit   内存主频    DDR所以速度翻倍  1B = 8bits
  
instruction/bytes:
4.58T FLOPS / 320G B/s = 13.6
即每13.6个周期1byte数据
```

这里最后得出的13.6即若kernel能工作在每13.6条指令处理1B数据的情况下，可以获得最大的性能。可以以此为指标评价kernel效率