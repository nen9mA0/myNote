# SLEIGH

SLEIGH是ghidra中对SLED（Specification Language for Encoding and Decoding）的实现，此外，SLEIGH为指令添加了一些语法描述以方便数据流和反编译分析，这方面的功能借鉴自SSL（Semantic Syntax Language），在该语言设计中，工作流如下

* 对机器码进行反汇编，并翻译到中间语言（IR）

* 将IR转换到更高级的表示
  
  * 进行数据流分析，包含死代码分析和拷贝传播（copy propagation）
  
  * 进行控制流分析

* 基于转换的表示生成更高级的反编译代码



## pcode简介


