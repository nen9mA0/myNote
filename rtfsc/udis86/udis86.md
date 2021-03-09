## 数据结构
### 枚举
* ud_type  寄存器类型，包括8b-GPR 16b-GPR 32b-GPR 64b-GPR 段寄存器　控制寄存器（CR0~15） 调试寄存器（DR0~15） MMX寄存器（MM0~7） x87寄存器（ST0~7）　XMM寄存器（XMM0~15） RIP 后面是所有汇编指令的操作数类型（寄存器　内存　指针　立即数　跳转立即数　常量）
* ud_operand_code  所有操作数类型
* ud_operand_size  所有操作数size的枚举

#### ud_operand_size
```c
enum ud_operand_size {
    SZ_NA  = 0,     // 长度为0
    SZ_Z   = 1,     
    SZ_V   = 2,
    SZ_P   = 3,
    SZ_WP  = 4,     // pointer: word，这里指指针本身的长度16位（短跳指令中使用）
    SZ_DP  = 5,     // pointer: dword 指针本身长度32位（长跳）
    SZ_MDQ = 6,
    SZ_RDQ = 7,

    /* the following values are used as is,
     * and thus hard-coded. changing them 
     * will break internals 
     */
    SZ_B   = 8,
    SZ_W   = 16,
    SZ_D   = 32,
    SZ_Q   = 64,
    SZ_T   = 80,
};
```

### 结构体
#### ud_operand
```c
struct ud_operand 
{
  enum ud_type		type;
  uint8_t		size;
  union {
	int8_t		sbyte;
	uint8_t		ubyte;
	int16_t		sword;
	uint16_t	uword;
	int32_t		sdword;
	uint32_t	udword;
	int64_t		sqword;
	uint64_t	uqword;

	struct {
		uint16_t seg;
		uint32_t off;
	} ptr;
  } lval;

  enum ud_type		base;
  enum ud_type		index;
  uint8_t		offset;
  uint8_t		scale;	
};
```

#### ud
```c
struct ud
{
  int 			(*inp_hook) (struct ud*);
  uint8_t		inp_curr;           // 当前cache内元素的指针
  uint8_t		inp_fill;           // cache元素尾指针
  FILE*			inp_file;           // 输入为文件时指向文件指针
  uint8_t		inp_ctr;            // 当前读入的字节数
  uint8_t*		inp_buff;           // 输入为buffer时指向buffer
  uint8_t*		inp_buff_end;       // 指向buffer结尾
  uint8_t		inp_end;
  void			(*translator)(struct ud*);  // 语法翻译器，AT&T或Intel
  uint64_t		insn_offset;        // 偏移
  char			insn_hexcode[32];   // hex
  char			insn_buffer[64];    // 反汇编结果
  unsigned int		insn_fill;
  uint8_t		dis_mode;   // 位数
  uint64_t		pc;         // 程序指针
  uint8_t		vendor;     // Intel AMD
  struct map_entry*	mapen;
  enum ud_mnemonic_code	mnemonic;   // 该类型为itab.h中的枚举，对应的是当前指令的IClass（或可以说对应了助记符）
  struct ud_operand	operand[3];     // 操作数（一条汇编最多三个操作数）
  uint8_t		error;
  uint8_t	 	pfx_rex;            // 是否有各个prefix
  uint8_t 		pfx_seg;
  uint8_t 		pfx_opr;
  uint8_t 		pfx_adr;
  uint8_t 		pfx_lock;
  uint8_t 		pfx_rep;
  uint8_t 		pfx_repe;
  uint8_t 		pfx_repne;
  uint8_t 		pfx_insn;
  uint8_t		default64;
  uint8_t		opr_mode;
  uint8_t		adr_mode;
  uint8_t		br_far;             // 跳转指令，且为长跳
  uint8_t		br_near;            // 跳转指令，且为短跳
  uint8_t		implicit_addr;
  uint8_t		c1;
  uint8_t		c2;
  uint8_t		c3;
  uint8_t 		inp_cache[256];     // cache缓冲区，用于缓存输入
  uint8_t		inp_sess[64];       // 当前会话（session）的缓冲区
  struct ud_itab_entry * itab_entry;
};
```

#### ud_itab_entry_operand
```c
struct ud_itab_entry_operand 
{
  enum ud_operand_code type;
  enum ud_operand_size size;
};
```

#### ud_itab_entry
```c
struct ud_itab_entry 
{
  enum ud_mnemonic_code         mnemonic;
  struct ud_itab_entry_operand  operand1;
  struct ud_itab_entry_operand  operand2;
  struct ud_itab_entry_operand  operand3;
  uint32_t                      prefix;
};
```

## 几张表的构造
### ud_itab_list
ud_itab_entry* 指针数组，每个ud_itab_entry*指向一个ud_itab_entry表的数组

该数组的下标其实对应了另一个enum结构 ud_itab_index，一共119个枚举，与ud_itab_list的元素一一对应。

每个ud_itab_entry表数组都对应了一组指令的格式

### ud_mnemonics_str
每条指令对应的字符串表

这个表的下标也对应了一个enum， ud_mnemonic_code，不过这个枚举的个数比字符串表要长11个元素，主要是在末尾添加了下列关于操作数的enum

```
  UD_Id3vil,
  UD_Ina,
  UD_Igrp_reg,      // 表示同opcode指令根据reg字段区分
  UD_Igrp_rm,       // 表示同opcode指令根据rm字段区分
  UD_Igrp_vendor,   // 表示同opcode指令根据制造商区分（就是intel和amd指令不相同的情况，实际上都是一方支持一方不支持，不会出现冲突的情况）
  UD_Igrp_x87,      // 表示同opcode对应了不同x87指令
  UD_Igrp_mode,
  UD_Igrp_osize,
  UD_Igrp_asize,
  UD_Igrp_mod,
  UD_Inone
```
这些值是对于opcode（即除前缀外的第一个字节）相同时对应了多个指令的情况，如itab__1byte表的0x60对应的指令有pusha和pushad，这些情况下需要继续判断对应的指令

### ud_type

就是这张表

https://wiki.osdev.org/X86-64_Instruction_Encoding#Registers

```c
enum ud_type
{
  UD_NONE,

  /* 8 bit GPRs */
  UD_R_AL,	UD_R_CL,	UD_R_DL,	UD_R_BL,
  UD_R_AH,	UD_R_CH,	UD_R_DH,	UD_R_BH,
  UD_R_SPL,	UD_R_BPL,	UD_R_SIL,	UD_R_DIL,
  UD_R_R8B,	UD_R_R9B,	UD_R_R10B,	UD_R_R11B,
  UD_R_R12B,	UD_R_R13B,	UD_R_R14B,	UD_R_R15B,

  /* 16 bit GPRs */
  UD_R_AX,	UD_R_CX,	UD_R_DX,	UD_R_BX,
  UD_R_SP,	UD_R_BP,	UD_R_SI,	UD_R_DI,
  UD_R_R8W,	UD_R_R9W,	UD_R_R10W,	UD_R_R11W,
  UD_R_R12W,	UD_R_R13W,	UD_R_R14W,	UD_R_R15W,
	
  /* 32 bit GPRs */
  UD_R_EAX,	UD_R_ECX,	UD_R_EDX,	UD_R_EBX,
  UD_R_ESP,	UD_R_EBP,	UD_R_ESI,	UD_R_EDI,
  UD_R_R8D,	UD_R_R9D,	UD_R_R10D,	UD_R_R11D,
  UD_R_R12D,	UD_R_R13D,	UD_R_R14D,	UD_R_R15D,
	
  /* 64 bit GPRs */
  UD_R_RAX,	UD_R_RCX,	UD_R_RDX,	UD_R_RBX,
  UD_R_RSP,	UD_R_RBP,	UD_R_RSI,	UD_R_RDI,
  UD_R_R8,	UD_R_R9,	UD_R_R10,	UD_R_R11,
  UD_R_R12,	UD_R_R13,	UD_R_R14,	UD_R_R15,

  /* segment registers */
  UD_R_ES,	UD_R_CS,	UD_R_SS,	UD_R_DS,
  UD_R_FS,	UD_R_GS,	

  /* control registers*/
  UD_R_CR0,	UD_R_CR1,	UD_R_CR2,	UD_R_CR3,
  UD_R_CR4,	UD_R_CR5,	UD_R_CR6,	UD_R_CR7,
  UD_R_CR8,	UD_R_CR9,	UD_R_CR10,	UD_R_CR11,
  UD_R_CR12,	UD_R_CR13,	UD_R_CR14,	UD_R_CR15,
	
  /* debug registers */
  UD_R_DR0,	UD_R_DR1,	UD_R_DR2,	UD_R_DR3,
  UD_R_DR4,	UD_R_DR5,	UD_R_DR6,	UD_R_DR7,
  UD_R_DR8,	UD_R_DR9,	UD_R_DR10,	UD_R_DR11,
  UD_R_DR12,	UD_R_DR13,	UD_R_DR14,	UD_R_DR15,

  /* mmx registers */
  UD_R_MM0,	UD_R_MM1,	UD_R_MM2,	UD_R_MM3,
  UD_R_MM4,	UD_R_MM5,	UD_R_MM6,	UD_R_MM7,

  /* x87 registers */
  UD_R_ST0,	UD_R_ST1,	UD_R_ST2,	UD_R_ST3,
  UD_R_ST4,	UD_R_ST5,	UD_R_ST6,	UD_R_ST7, 

  /* extended multimedia registers */
  UD_R_XMM0,	UD_R_XMM1,	UD_R_XMM2,	UD_R_XMM3,
  UD_R_XMM4,	UD_R_XMM5,	UD_R_XMM6,	UD_R_XMM7,
  UD_R_XMM8,	UD_R_XMM9,	UD_R_XMM10,	UD_R_XMM11,
  UD_R_XMM12,	UD_R_XMM13,	UD_R_XMM14,	UD_R_XMM15,

  UD_R_RIP,

  /* Operand Types */
  UD_OP_REG,	UD_OP_MEM,	UD_OP_PTR,	UD_OP_IMM,	
  UD_OP_JIMM,	UD_OP_CONST
};
```



### ud_itab_list内的各个表
主要的表分为itab__0f和itab__1byte，前者是第一字节为0f的指令，后者是opcode只有一字节的指令

#### 表项
* mnemonic 一个ud_mnemonic_code枚举类型，表示了当前指令的ICLASS，或表示当前指令无法区分，下一步应该根据哪个字段进一步查找
* operand1~3 操作数类型，每个类型都是一个ud_operand结构体，包括type、size和ptr字段
  * type  是一个ud_operand_code类型
  * ptr  是一个union，对应多种不同类型的操作数
  * size  是一个ud_operand_size类型的枚举。
* prefix 若当前指令无法区分，该字段保存下一张表的下标，否则保存了当前ICLASS的一些属性值（通过标志位表示），包括
  * P_none
  * P_c1
  * P_rexb
  * P_depM
  * P_c3
  * P_inv64 当该指令在64位下不可用时设置
  * P_rexw
  * P_c2
  * P_def64 当该指令默认为64位指令时设置
  * P_rexr
  * P_oso
  * P_aso
  * P_rexx
  * P_ImpAddr


#### 查表的过程
这里有个很重要的设置：表中的表项为ud_itab_entry类型，第一个字段对应各个iclass，第二到四个字段对应operand，最后一个字段对应prefix。

但若当前的opcode对应了多个指令，第一个字段会是上述enum最后几个中的一个。指令的最终类型确定方式如下
* 根据enum类型可以得到当前指令需要通过什么字段与其他指令区分，如UD_Igrp_reg表示通过MODRM_REG字段区分
* 对应的区别的表位于该ud_itab_entry的prefix字段。如对于itab__1byte表的0x80项，mnemonic字段为UD_Igrp_reg，说明需要通过MODRM_REG进一步确定指令。此后查找prefix字段，为ITAB__1BYTE__OP_80__REG，即对应itab__1byte__op_80__reg表，此时便是通过MODRM_REG的三位确定是表中的哪个ICLASS
* 以上是进行两次查表的做法，对于更加复杂的指令可能会涉及多次查表的操作，如对于根据MODRM_MOD区分的指令，0~2表示操作数为内存，3表示操作数为寄存器，这个程序中前者被归为一类，后者被归为第二类。两者都需要进一步查表才能获取对应的ICLASS

#### 表的命名
根据表的命名其实可以大概获知该表是哪些表的下一级表，以及什么情况下会到达该表，举例说明

itab__0f的第二个表项mnemonic字段为UD_Igrp_reg，表示指令根据MODRM_REG区分。该表项prefix为ITAB__0F__OP_01__REG，从表名可知其为itab_0f表中opcode为01（即下标01）的字段对应的表，且是根据REG区分指令的

下一级表为itab__0f__op_01__reg，其第四个表项mnemonic为UD_Igrp_mod，prefix为ITAB__0F__OP_01__REG__OP_03__MOD，即表示itab_0f中op为01，REG为03的情况，接下来根据MODRM_MOD确定指令

下一级表为itab__0f__op_01__reg__op_03__mod，其第二个表项mnemonic为UD_Igrp_rm，prefix为ITAB__0F__OP_01__REG__OP_03__MOD__OP_01__RM。表示itab_0f op=01 REG=03 MOD=01的情况，接下来根据MODRM_RM确定指令

下一级表为itab__0f__op_01__reg__op_03__mod__op_01__rm，其第一个表项mnemonic为UD_Igrp_vendor，prefix为ITAB__0F__OP_01__REG__OP_03__MOD__OP_01__RM__OP_00__VENDOR，最后一个字段是根据vendor为Intel还是AMD确定指令

下一级表为itab__0f__op_01__reg__op_03__mod__op_01__rm__op_00__vendor，在这里最终获取指令类型


#### itab__0f系列
以0f为第一个字节的指令，主要是一系列system指令，SSE，条件mov，条件跳转，MMX，位测试与位设置指令，段寄存器push pop指令，三操作数移位指令，位扫描指令，原子操作指令，SSE2指令

#### itab_1byte系列
opcode为一个字节的指令（可能用到MODRM作为附加编码）


## 顶层接口

* ud_init  初始化ud结构体
* ud_set_mode  设置反汇编位数
* ud_set_vendor  intel还是amd
* ud_set_pc  设置程序指针
* ud_insn_asm  获取反汇编结果
* ud_insn_offset  获取偏移
* ud_insn_hex  获取程序hex
* ud_insn_ptr  
* ud_insn_len  获取程序长度

### ud_disassemble
反汇编一条指令，根据translator指定的翻译器翻译，并返回指令长度

## input.c
用于处理输入和buffer的模块

* inp_set_input_file  设置输入文件
* inp_set_input_buffer  设置输入缓冲区
* inp_file_hook  从文件获取一个字节
* inp_buff_hook  从缓冲区获取一个字节
* ud_input_skip  跳过n个输入字符
* ud_input_end  判断输入是否到达末尾
* inp_back  回退一个字符
* inp_peek  预取一个字符（不移动buffer指针）
* inp_move  前移n个字符
* inp_uint8
* inp_uint16
* inp_uint32
* inp_uint64

### inp_next
获取下一个字节数据，若cache还未到结尾则取cache的元素，否则从文件/buffer取元素，并最后赋值给inp_sess缓冲区（当前session的缓冲区）

## decode.c

### 一些宏
#### rex前缀相关
```c
#define REX_W(r)        ( ( 0xF & ( r ) )  >> 3 )
#define REX_R(r)        ( ( 0x7 & ( r ) )  >> 2 )
#define REX_X(r)        ( ( 0x3 & ( r ) )  >> 1 )
#define REX_B(r)        ( ( 0x1 & ( r ) )  >> 0 )

#define P_REXW(n)       ( ( n >> 5 ) & 1 )
#define P_REXR(n)       ( ( n >> 8 ) & 1 )
#define P_REXX(n)       ( ( n >> 11 ) & 1 )
#define P_REXB(n)       ( ( n >> 1 ) & 1 )

#define REX_PFX_MASK(n) ( ( P_REXW(n) << 3 ) | \
                          ( P_REXR(n) << 2 ) | \
                          ( P_REXX(n) << 1 ) | \
                          ( P_REXB(n) << 0 ) )
```
REX_W REX_R REX_X REX_B对应了REX前缀低4位

#### SIB相关
```c
#define SIB_S(b)        ( ( b ) >> 6 )
#define SIB_I(b)        ( ( ( b ) >> 3 ) & 7 )
#define SIB_B(b)        ( ( b ) & 7 )
```

#### MODRM相关
```c
#define MODRM_REG(b)    ( ( ( b ) >> 3 ) & 7 )
#define MODRM_NNN(b)    ( ( ( b ) >> 3 ) & 7 )
#define MODRM_MOD(b)    ( ( ( b ) >> 6 ) & 3 )
#define MODRM_RM(b)     ( ( b ) & 7 )
```

### ud_decode
解码函数

* clear_insn  重置ud结构体的prefix operand等
* get_prefixes  获取prefix

解码过程按顺序执行下列几个函数

#### get_prefixes
* 若当前为64位，且当前字节 & 0xf0 == 0x40（高八位为4），则为prefix前缀
* 剩下的有如下情况
  * 2E  CS
  * 36  SS
  * 3E  DS
  * 26  ES
  * 64  FS
  * 65  GS
  * 67  地址位数转换
  * F0  lock
  * 66  操作数位数转换
  * F2  repne
  * F3  repe    repne和repe与rex不同时作用
* 根据位数转换前缀和当前位数，转换操作数和地址位数，其中64位操作数转换的位数是根据rex_w和操作数位数转换前缀共同决定的

#### search_itab
* 第一步是为指令寻找合适的itable
  * 字节为90的特殊情况
    * 如果是64位且REX_B为1，不处理
      * 否则若有rep前缀，指令为pause
      * 若无rep前缀，指令为nop
  * 字节为0x0f的情况，读取下一个字节，假设为ch
    * 如果有66 prefix，搜索 `itab__pfx_sse66__0f[ch]` 表是否有对应指令
    * 如果有F2 prefix，搜索 `itab__pfx_ssef2__0f[ch]` 表是否有对应指令
    * 如果有F3 prefix，搜索 `itab__pfx_ssef3__0f[ch]` 表是否有对应指令
    * 否则使用itab__0f表
  * 否则使用itab__1byte表
* 第二步是根据itable的对应值进行合适的操作，主要是
  * 若有对应的ud_mnemonic_code，且对应了一个ICLASS（即不是最后的11个enum）
    * 若为UD_Iinvalid，返回错误
    * 其他情况说明找到了对应指令，跳转到第三步
  * 下面是几种其他的case，具体的查表方法在前面有说明
    * UD_Igrp_reg  根据MODRM_REG字段查表
    * UD_Igrp_mod  根据MODRM_MOD字段查表，这种情况下虽然mod字段有2bit，但对应的表项其实只根据是否为3分为两种情况。为3是仅使用了寄存器或立即数操作数的情况
    * UD_Igrp_rm   根据MODRM_RM字段查表
    * UD_Igrp_x87  x87指令前缀字节相同的情况，根据下一个字节-0xC0作为下标查表
    * UD_Igrp_osize  根据operand size长度确定指令，下标为： 0:16b  1:32b  2:64b
    * UD_Igrp_asize  根据address size长度确定指令，下标同上
    * UD_Igrp_mode  根据当前位数确定指令，下标同上
    * UD_Igrp_vendor  根据intel还是amd确定指令，下标： amd:0  intel:1
  * 若当前mnemonic为上述8种情况，则重新执行第二步（进行下一步查表），当然这里表和索引值是根据当前查表结果确定的
* 第三步就是返回最终查到的ud_itab_entry结构

#### do_mode
* 若反汇编模式为64位
  * 检查P_inv64位，确定当前指令是否可用于64位
  * 确定operand长度
    * 若rex_w位设置，则为64
    * 若prefix 66设置，则为16
    * 若P_def64标志设置，则为64
    * 否则为32
  * 确定address长度
    * 若prefix 67设置，则为32
    * 否则为64
* 若反汇编模式为32位
  * 若prefix 66设置，operand长度16，否则32
  * 若prefix 67设置，operand长度16，否则32
* 若反汇编模式为16位
  * 若prefix 66设置，operand长度32，否则16
  * 若prefix 67设置，operand长度32，否则16

#### disasm_operands
根据第一个操作数的类型，执行解码的操作
##### 操作数类型
如下，这里简单起见直接把disasm_operands对应各个不同操作数的操作写在底下，因为这个函数也只干了这一件事
```c
enum ud_operand_code
{
  OP_NONE,

  OP_A,         // 对应 seg:offset的情况，调用decode_a
  OP_E,         // 
  OP_M,
  OP_G,
  OP_I,

  OP_AL,
  OP_CL,
  OP_DL,
  OP_BL,
  OP_AH,
  OP_CH,
  OP_DH,
  OP_BH,

  OP_ALr8b,
  OP_CLr9b,
  OP_DLr10b,
  OP_BLr11b,
  OP_AHr12b,
  OP_CHr13b,
  OP_DHr14b,
  OP_BHr15b,

  OP_AX,
  OP_CX,
  OP_DX,
  OP_BX,
  OP_SI,
  OP_DI,
  OP_SP,
  OP_BP,

  OP_rAX,
  OP_rCX,
  OP_rDX,
  OP_rBX,
  OP_rSP,
  OP_rBP,
  OP_rSI,
  OP_rDI,

  OP_rAXr8,
  OP_rCXr9,
  OP_rDXr10,
  OP_rBXr11,
  OP_rSPr12,
  OP_rBPr13,
  OP_rSIr14,
  OP_rDIr15,

  OP_eAX,
  OP_eCX,
  OP_eDX,
  OP_eBX,
  OP_eSP,
  OP_eBP,
  OP_eSI,
  OP_eDI,

  OP_ES,
  OP_CS,
  OP_SS,
  OP_DS,
  OP_FS,
  OP_GS,

  OP_ST0,
  OP_ST1,
  OP_ST2,
  OP_ST3,
  OP_ST4,
  OP_ST5,
  OP_ST6,
  OP_ST7,

  OP_J,
  OP_S,
  OP_O,
  OP_I1,
  OP_I3,

  OP_V,
  OP_W,
  OP_Q,
  OP_P,

  OP_R,
  OP_C,
  OP_D,
  OP_VR,
  OP_PR
};
```

#### resolve_mnemonic
对call jmp swapgs和3Dnow的特殊处理
* 若为call/jmp
  * 操作数0的size为SZ_WP（16位指针），则设置短跳标志位（ud.br_near）
  * 操作数0的size为SZ_DP（32位指针），则设置为长跳标志位（ud.br_far）
  * 若都不是，视为短跳
* 若为3Dnow指令
  * 在3Dnow的table中取对应的iclass（然而这个3Dnow的table没有一条不是UD_Iinvalid）
* 若为swapgs，且反汇编模式为64，报错

### 一系列decode函数

#### decode_a

解码seg:off形式的指令

