## 项目链接

[Valgrind: Current Releases](https://valgrind.org/downloads/)

VEX相关代码在Valgrind的VEX目录下

## VEX相关代码

### VEX类型

定义在`VEX/pub/libvex_ir.h` 部分实现在 `VEX/priv/ir_defs.c`

#### 数据类型

* 整数
  
  * I1
  
  * I8
  
  * I16
  
  * I32
  
  * I64
  
  * I128

* 浮点
  
  * F16
  
  * F32
  
  * F64
  
  * F128

* 十进制浮点
  
  * D32
  
  * D64
  
  * D128

* SIMD
  
  * V128
  
  * V256

#### VEX类型

##### 数学运算

* Add8/16/32/64

* Sub8/16/32/64

* Mul8/16/32/64

##### 布尔运算

* Or8/16/32/64

* And8/16/32/64

* Xor8/16/32/64

* Shl8/16/32/64

* Shr8/16/32/64

* Sar8/16/32/64

* Not8/16/32/64

##### 比较

* CmpEQ8/16/32/64

* CmpNE8/16/32/64

* 
