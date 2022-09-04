### zeroevil.c

#### 获取syscall table

##### get_sct

这里提供了两种方法

* kporbe  （内核版本大于等于4.17）
  
  * 版本大于5.7
    
    * 初始化kprobe结构体的symbol_name为“sys_call_table”
    
    * 调用register_kprobe注册kprobe
    
    * 由kprobe结构体的addr成员获取调用表地址
  
  * 版本小于5.7
    
    * 调用 `kallsyms_lookup_name("sys_call_table")` 获取系统调用表地址

* 搜索内存
  
  * 从PAGE_OFFSET开始搜索，将当前内存地址假设为syscall_table入口，进行判断
    
    `entry[__NR_close] == (unsigned long *)sys_close`

#### 修改CR0寄存器

可以通过修改CR0寄存器对无Write属性的页面进行写入

##### disable_wp

* preempt_disable  将内核变为非抢占式的

* read_cr0  读取CR0寄存器

* clear_bit  第一个参数为X86_CR0_WP_BIT，即将写保护位清零

* 这里使用了一段内联汇编，直接调用了 `mov cr0, reg` ，这是因为内核提供的write_cr0函数有一套pinned_bits机制作为保护，防止攻击者使用rop链修改cr0（见 [这里](../../../Sec/linux_kernel/bypass_smep_kaslr_kptr_restric.md)），也可以参照[源码](https://elixir.bootlin.com/linux/latest/source/arch/x86/kernel/cpu/common.c#L421)发现write_cr0存在的pinned_bits机制，这里会检查write protection位

* preempt_enable  恢复内核为抢占式

##### enable_wp

与disable_wp基本一致，但给cr0赋值的部分可以换成正常的 write_cr0 函数
