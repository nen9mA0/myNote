[The LLVM Target-Independent Code Generator — LLVM 20.0.0git documentation](https://llvm.org/docs/CodeGenerator.html#abstract-target-description)

### 介绍

分成6个主要组件

* 抽象目标描述（Abstract target description），接口在`include/llvm/target`定义，用于记录目标架构多种重要属性

* 用于描述生成的代码的类，接口在`include/llvm/CodeGen`定义，这是一系列统一描述在任何机器上生成的代码的类

* MC layer，用于描述汇编层面结构的类，如标号、段等

* 目标无关算法，相关代码在`lib/CodeGen`，用于实现多种代码生成算法，如寄存器分配、栈空间表示等

* 针对特定目标的抽象目标描述接口实现，相关代码在`lib/Target`，这里针对特定目标实现了一些特定pass

* 目标无关的JIT组件，相关代码在`lib/ExecutionEngine/JIT`

### 代码生成器的两个必需组件

两个最重要的接口 TargetMachine和DataLayout 是LLVM后端必需的，一般的LLVM编译流程还有其他组件，但可以仅依赖这两个组件实现一些非标的代码生成，如对接GCC后端、生成FPGA等异架构代码等

### 代码生成器的顶层设计

设计用于生成标准的基于寄存器的处理器代码

* 指令选择  用于为指令从LLVM代码转换为机器代码选择一个高效的表达方式，这一步将LLVM代码转为目标指令的DAG

* 指令调度和构建   处理上一步产生的DAG，并发射对应架构的指令（MachineInstr类）

* 基于SSA进行机器代码优化

* 寄存器分配  用于将虚拟寄存器转换为实际的寄存器

* 插入函数序言和末尾

* 机器代码后优化

* 选择使用汇编还是二进制形式生成代码




