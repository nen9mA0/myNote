# 构建

## mfile.py

xed_mbuild.execute的一个外层接口

## xed_build_common

存放一些xed_mbuild文件调用的用于指定一些配置参数的函数

## xed_mbuild

### 类

#### generator_inputs_t

<span id="generator_inputs_t"/>

根据files.cfg配置文件提供的各个配置项（对应field中的选项），确定生成指令集所需的各个文件名

##### 重要属性

###### fields

<span id="fields"/>

有下面一系列配置，每个配置都对应decode_command或encode_command的一个参数

| fields配置 | 对应命令行参数 | 对应程序文件argparser解析出的属性 | 备注 |
| ---- | ---- | ---- | |
|  dec-spine | --spine | spine | decoder |
|  dec-instructions   | --isa | isa_input_file | decoder |
|  enc-instructions | --isa | isa_input_file | encoder |
|  dec-patterns | --patterns | structured_input_fn | decoder |
|  enc-patterns | --enc-patterns | enc_patterns | encoder |
|  enc-dec-patterns | --dec-enc-patterns | enc_dec_patterns | encoder |
|  fields | --input-fields | input_fields | both |
|  state | --input-state | input_state | both |
|  registers | --input-regs | input_regs | both |
|  widths | --input-widths | input_widths | decoder |
|  extra-widths | --input-extra-widths | input_extra_widths | decoder |
|  pointer-names | --input-pointer-names | input_pointer_names | decoder |
|  element-types | --input-element-types | input_element_types | decoder |
|  element-type-base | --input-element-type-base | input_element_type_base | decoder |
|  chip-models | --chip-models | chip_models_input_fn | decoder |
|  conversion-table | --ctables | ctables_input_fn | decoder |
|  cpuid | --cpuid | cpuid_input_fn | decoder |
|  map-descriptions | --map-descriptions | map_descriptions_input_fn | decoder |

##### 重要方法

###### decode_command

根据[_parse_extf_files_new](#_parse_extf_files_new)读取的内容生成decoder生成的调用参数（调用generator.py用）

###### encode_command

根据[_parse_extf_files_new](#_parse_extf_files_new)读取的内容生成encoder生成的调用参数（调用read-encfile.py用）

### 重要内部函数

#### _configure_libxed_extensions

<span id="_configure_libxed_extensions"></span>

根据当前关于**指令集**的配置参数添加一系列预定义define参数和files.cfg配置文件，内容可以见下面的配置文件一栏

配置全部保存在`env['extf']`

#### _parse_extf_files_new

<span id="_parse_extf_files_new"></span>

**用于解析files.cfg文件**，并构造其对应的[generator_inputs_t](#generator_inputs_t)

格式：

* ptype : fname
* cmd : ptype : fname[ : priority]

以:为符号进行split，用wrds代表切分后结果

##### ptype

其实ptype就是[fields](#fields)

##### cmd

* clear  清除对应ptype的文件列表
* define  在env中添加一个定义
* remove-source  移除ptype文件列表中的fname文件
* add-source  添加ptype文件列表中的fname文件
* replace-source  替换ptype文件列表中的fname文件
* add  在ptype文件列表中添加fname
* 默认

#### run_decode_generator

<span id="run_decode_generator"/>

调用generator_inputs_t的[decode_command](#decode_command)根据env创建一个调用generator.py的命令，并用mbuild.run_command执行

### 重要接口函数

#### execute

##### env_t初始化

调用mkenv创建env_t，赋初值并赋给`xed_defaults`

##### 指定并解析参数

调用xed_args，指定xed_mbuild可以接受的参数并解析

##### 调用work

调用work

#### work

##### 指定默认编译参数

macro_args

##### 检查配置

verify_args  主要处理一些指令集的依赖关系，比如如果没有指定avx，就将`env['avx512']`置为False

##### 初始化

调用xbc的init，实际上只是添加一个库搜索路径而已

调用xed_mbuild的[init](#init)

清理（若target中指定了clean）并创建目标目录，创建work_queue

build_libxed

#### init

<span id="init"></span>

检查并获取python环境

将`include/private` `include/public/xed`和`include/public`加入包含路径

检查env['target']是否在默认target内

#### build_libxed

创建generator_inputs_t结构，指定了一些输出文件

* 调用[_configure_libxed_extensions](#_configure_libxed_extensions)解解析指令集配置
* 调用[_parse_extf_files_new](#_parse_extf_files_new)解析上面函数得到的`env['extf']`变量中的各种files.cfg文件的内容
* 调用`emit_defines_header`，根据`env['DEFINES']`创建并定义xed-build-defines.h文件
* 生成prep-inputs文件，见重要文件一栏
  * 将[run_generator_preparation](#run_generator_preparation)加入准备构建的dag中
* 构建decoder
  * 调用`add_decoder_command`
    * 该函数指定了一系列需要用到的py文件，具体列表在[decoder](#decoder)
    * 调用[need_to_rebuild](#need_to_rebuild)判断是否需要rebuild
    * 将[run_decode_generator](#run_decode_generator)加入准备构建的dag中

# 通用类与函数

## operand_storage

### 类

#### operand_field_t

<span id="operands_field_t"/>

##### 属性

* name  表示operand类型名
* aggtype  目前都是SCALAR
* ctype  该operand对应的C语言类型，有如下情况
  * xed_reg_enum_t  default_initializer为XED_REG_INVALID
  * xed_iclass_enum_t  default_initializer为XED_ICLASS_INVALID
  * 其他情况  default_initializer为0
* width  位宽
* default_visibility  有这三种情况[EXPLICIT|IMPLICIT|SUPPRESSED]
* internal_or_public  若为internal表示该field不在instructions operands array template中
* dio  该field用于decoder input/output/skip
* eio  该field用于encoder input/output
* encoder_input  若为EI（encoder input）则为true
* decoder_skip  若为DS（decoder skip）则为true

#### operands_storage_t

<span id="operands_storage_t"/>

主要根据输入的field文件格式创建[operand_field_t](#operand_field_t)字典

```python
(name, aggtype, ctype, width, default_visibility,
             xprint, internal_or_public, dio, eio) = wrds
```

文件格式示例为

```
SEG1           SCALAR     xed_reg_enum_t 16     EXPLICIT    PRINT   INTERNAL   DO EI   
BASE1          SCALAR     xed_reg_enum_t 16     EXPLICIT    PRINT   PUBLIC   DO EI   
```

## slash_expand

### 函数

#### expand_all_slashes

将正则`(?P<letter>[a-z])[/](?P<number>[0-9]+)`匹配的内容替换为`<letter>*<number>`的形式，如

```
d/8
```

替换为`dddddddd`

## generator

顶层文件

### 获取的文件结构

#### map_info

<span id="map_info"/>

对应files.cfg中的**map-descriptions**指定的文件，根据该文件内容创建[map_info_t](#map_info_t)

```python
    mi = map_info_t()
    mi.map_name = t[0]
    mi.space = t[1]
    mi.legacy_escape = t[2]
    mi.legacy_opcode = t[3]
    mi.map_id = t[4]
    mi.modrm = t[5]
    mi.imm8 = t[6]
    mi.imm32 = t[7]
    mi.opcpos = t[8]
    mi.search_pattern = t[9]
```

文件格式示例为

```
#  name     space   esc  mapopc mapno modrm imm8 int32  opcpos pattern
legacy-map0  legacy  N/A  N/A    0     var   var  no    0       ''
legacy-map1  legacy  0x0F N/A    1     var   var  no    1       '0x0F'
legacy-map2  legacy  0x0F 0x38   2     yes   no   no    2       '0x0F 0x38'
legacy-map3  legacy  0x0F 0x3A   3     yes   yes  no    2       '0x0F 0x3A'
```

##### 规则



#### operand_storage_fields

<span id="operand_storage_fields"/>

[gen_operand_storage_fields](#gen_operand_storage_fields)

表示operand类型

对应files.cfg中的**fields**指定的文件，创建[operands_storage_t](#operands_storage_t)，后者解析文件并创建[operand_field_t](#operand_field_t)

#### regs

<span id="regs"/>

[gen_regs](#gen_regs)

对应files.cfg中的**registers**指定的文件，根据该文件内容创建[reg_info_t](#reg_info_t)

```python
name = wrds[0]
rtype = wrds[1]
width = wrds[2]
max_enclosing_reg = wrds[3]
max_enclosing_reg_32 = None
if '/' in max_enclosing_reg:
    (max_enclosing_reg, max_enclosing_reg_32) = max_enclosing_reg.split('/')
ordinal = 0
if n >= 5:
    ordinal = int(wrds[4])
    hreg = None
    if n >= 6:
        hreg = wrds[5]
display_str = None
if n >= 7:
    display_str = wrds[6]

ri = reg_info_t(name,
                rtype,
                width,
                max_enclosing_reg,
                ordinal, 
                hreg,
                max_enclosing_reg_32,
                display_str)
```

文件格式示例

```
RIP  ip    64  RIP
EIP  ip    32  RIP/EIP 
IP   ip    16  RIP/EIP 
```

##### 规则

* name  寄存器名
* rtype  寄存器类型
* width  寄存器位宽
* max_enclosing_reg  该寄存器对应的父寄存器，比如32位下IP寄存器的父寄存器为EIP
* ordinal  寄存器序数（是否跟寄存器的二进制编码有关？）
* hreg  表示该寄存器是8八位寄存器，如AH BH

#### width

<span id="width" />

[gen_widths](#gen_widths)

对应files.cfg中的**widths**指定的文件，根据该文件内容创建[width_info_t](#width_info)

```python
if ntokens == 3:
    (name, dtype,  all_width) = wrds
    width8 =  all_width
    width16 = all_width
    width32 = all_width
    width64 = all_width
elif ntokens == 5:
    width8='0'
    (name,  dtype, width16, width32, width64) = wrds
bit_widths = []
for val in [width8, width16, width32, width64]:
   number_string = is_bits(val)
   if number_string:
      bit_widths.append(number_string)
   else:
      bit_widths.append(str(int(val)*8))
widths_list.append(width_info_t(name, dtype, bit_widths))
```

文件格式示例

```
asz         int       2 4 8  # varies with the effective address width
ssz         int       2 4 8  # varies with the stack address width
pseudo      struct    0      # these are for unusual registers
pseudox87   struct    0      # these are for unusual registers
```

##### 规则

有点类似于typedef

* name  数据类型名
* dtype  数据类型对应的实际类型
* width16  在16 32 64位下的位宽
* width32
* width64

#### extra_widths

<span id = "extra_widths"/>

[gen_extra_widths](#gen_extra_widths)

对应files.cfg中的**extra-widths**指定的文件（似乎不太重要）

#### element_type_base

<span id = "element_type_base"/>

[gen_element_types_base](#gen_element_types_base)

对应files.cfg中的**element-type-base**指定的文件

#### element_types

[gen_element_types](#gen_element_types)

对应files.cfg中的**element-type**指定的文件

```python
(xtype, dtype, bits_per_element) = line.split()
```

文件格式示例

```
i1            INT    1
i8            INT    8
```

#### pointer_names

<span id="pointer_names"/>

[get_pointer_names](#get_pointer_names)

ptr类型（byte ptr  word ptr等）

对应files.cfg中的**pointer-names**指定的文件

```python
(bbytes, name, suffix) = wrds
```

文件格式示例

```
1  byte     b
2  word     w
4  dword    l
8  qword    q
16 xmmword  x
```



### 类

#### generator_common_t



#### all_generator_info_t



### 重要函数

#### main

* 调用`map_info_rdr.read_file`获取map_info，构造[map_info_t](#map_info)类型
* 调用[gen_operand_storage_fields](#gen_operand_storage_fields)获取寄存器名及类型，构造[operand_storage_fields](#operand_storage_fields)类型
* 调用[gen_regs](#gen_regs)获取[regs](#regs)寄存器属性，构造[reg_info_t](#reg_info)类型
* 调用[gen_widths](#gen_widths)获取[width](#width)属性
* 调用[gen_extra_widths](#gen_extra_widths)获取[extra_widths](#extra_widths)
* 调用[gen_element_types_base](#gen_element_types_base)获取[element_type_base](#element_type_base)
* 调用[gen_element_types](#gen_element_types)获取[element_types](#element_types)
* 调用[gen_pointer_names](#gen_pointer_names)获取[pointer_names](#pointer_names)
* 调用[gen_everything_else](#gen_everything_else)获取

# encoder

## read_encfile

### 概述

#### fields文件

生成一个[operands_storage_t](#operands_storage_t)对应全局变量storage_field。其中`operand_fields`属性维护了一个[operand_t](#operand_t)列表，每个operand_t结构都存储了一个patterns或ins中的域的属性

#### state文件

保存了一些类似宏与实际标志位对应的结构，用于宏替换pattern中的一些标号，如在state中有规则

```
not64                   MODE!=2
cs_prefix               SEG_OVD=1
```

而在enc-pattern中有一条规则为`not64 cs_prefix -> 0x2e no_return`，则在pattern解析时替换为`MODE!=2 SEG_OVD=1 -> 0x2e`

#### pattern文件

分为enc-pattern dec-pattern和enc-dec-pattern，其中dec-pattern只比enc-dec-pattern多了一些内容，多出来的部分很多是enc-pattern的反向记录，如该节最后的示例

pattern文件存放了ins文件中使用到的一些函数的定义，或者在程序中称为非终结符nonterminal

所有的非终结符存放在`self.nts`和`self.ntlufs`中，此外pattern中还解析`self.seqs`，定义了一个操作序列

##### 非终结符列表

self.nts和self.ntlufs都存放了非终结符[nonterminal_t](#nonterminal_t)，区别只是ntlufs有定义返回值

每个nonterminal_t定义了一个规则列表[rule_t](#rule_t)，每个rult_t维护了一个[action_t](#action_t)列表和一个[conditions_t](#conditions_t)列表，用于定义一个条件式与对应执行的操作。这两者都对应了ins文件中调用的一些函数

##### 操作序列

定义了一个操作调用的非终结符。其中带返回值的ntlufs直接按照原名写在规则中，不带返回值的nts在规则中加入`_BIND`后缀，此外还有`_EMIT`等后缀。如下

```
SEQUENCE ISA_BINDINGS
   FIXUP_EOSZ_ENC_BIND()
   FIXUP_EASZ_ENC_BIND()
   ASZ_NONTERM_BIND()
   INSTRUCTIONS_BIND()
   OSZ_NONTERM_ENC_BIND()   # OSZ must be after the instructions so that DF64 is bound (and before any prefixes obviously)
   PREFIX_ENC_BIND()
   REX_PREFIX_ENC_BIND()

# These emit the bits and bytes that make up the encoding
SEQUENCE ISA_EMIT
   PREFIX_ENC_EMIT()
   REX_PREFIX_ENC_EMIT()
   INSTRUCTIONS_EMIT()  # THIS TAKES CARE OF MODRM/SIB/DISP/IMM
```

##### enc-pattern与dec-pattern对应关系示例

```
xed_reg_enum_t GPR8_R()::

OUTREG=XED_REG_AL -> REG=0x0
OUTREG=XED_REG_CL -> REG=0x1
OUTREG=XED_REG_DL -> REG=0x2
OUTREG=XED_REG_BL -> REG=0x3

OUTREG=XED_REG_AH -> REG=0x4  NOREX=1
OUTREG=XED_REG_CH -> REG=0x5  NOREX=1
OUTREG=XED_REG_DH -> REG=0x6  NOREX=1
OUTREG=XED_REG_BH -> REG=0x7  NOREX=1

OUTREG=XED_REG_SPL ->  REG=0x4  NEEDREX=1
OUTREG=XED_REG_BPL ->  REG=0x5  NEEDREX=1
OUTREG=XED_REG_SIL ->  REG=0x6  NEEDREX=1
OUTREG=XED_REG_DIL ->  REG=0x7  NEEDREX=1

OUTREG=XED_REG_R8B -> REXR=1 REG=0x0
OUTREG=XED_REG_R9B -> REXR=1 REG=0x1
OUTREG=XED_REG_R10B -> REXR=1 REG=0x2
OUTREG=XED_REG_R11B -> REXR=1 REG=0x3
OUTREG=XED_REG_R12B -> REXR=1 REG=0x4
OUTREG=XED_REG_R13B -> REXR=1 REG=0x5
OUTREG=XED_REG_R14B -> REXR=1 REG=0x6
OUTREG=XED_REG_R15B -> REXR=1 REG=0x7
```

对应dec中的

```
xed_reg_enum_t GPR8_R()::

REXR=0 REG=0x0  | OUTREG=XED_REG_AL
REXR=0 REG=0x1  | OUTREG=XED_REG_CL
REXR=0 REG=0x2  | OUTREG=XED_REG_DL
REXR=0 REG=0x3  | OUTREG=XED_REG_BL

REXR=0 REG=0x4  REX=0   | OUTREG=XED_REG_AH
REXR=0 REG=0x5  REX=0   | OUTREG=XED_REG_CH
REXR=0 REG=0x6  REX=0   | OUTREG=XED_REG_DH
REXR=0 REG=0x7  REX=0   | OUTREG=XED_REG_BH

REXR=0 REG=0x4  REX=1   | OUTREG=XED_REG_SPL
REXR=0 REG=0x5  REX=1   | OUTREG=XED_REG_BPL
REXR=0 REG=0x6  REX=1   | OUTREG=XED_REG_SIL
REXR=0 REG=0x7  REX=1   | OUTREG=XED_REG_DIL

REXR=1 REG=0x0  | OUTREG=XED_REG_R8B
REXR=1 REG=0x1  | OUTREG=XED_REG_R9B
REXR=1 REG=0x2  | OUTREG=XED_REG_R10B
REXR=1 REG=0x3  | OUTREG=XED_REG_R11B
REXR=1 REG=0x4  | OUTREG=XED_REG_R12B
REXR=1 REG=0x5  | OUTREG=XED_REG_R13B
REXR=1 REG=0x6  | OUTREG=XED_REG_R14B
REXR=1 REG=0x7  | OUTREG=XED_REG_R15B
```



### 类

#### encoder_input_files_t

<span id="encoder_input_files_t"/>

根据传入的argparser设置对应的属性，主要是encoder的输入文件

属性与argparser属性的对应关系（可以结合[field配置表](#fields)看）

| argparser        | encoder_input_files_t | 对应files.cfg属性 | 对应的all files |
| ---------------- | --------------------- | ----------------- | ------  |
| input_fields     | storage_fields_file   | fields            | all-fields.txt |
| input_regs       | regs_input_file       | registers         | all-registers.txt |
| enc_dec_patterns | decoder_input_files   | enc-dec-patterns  | all-enc-dec-patterns.txt |
| enc_patterns     | encoder_input_files   | enc-patterns      | all-enc-patterns.txt |
| input_state      | state_bits_file       | state             | all-state.txt |
| isa_input_file   | instructions_file     | enc-instructions  | all-enc-instructions.txt |

#### encoder_configuration_t

<span id="encoder_configuration_t"/>

read-encfile的主要类

根据读入的encoder_input_files_t创建各个输入文件对应的数据结构，初始化时主要根据storage_fields_file（这里用的是all-fields.txt）创建[operands_storage_t](#operands_storage_t)

##### 重要属性

###### state_bits

<span id="state_bits"/>

保存parse_state_bits结果，即对state_bits文件的parse结果



##### 重要方法

###### parse_state_bits

<span id="parse_state_bits"/>

将state_bits输入转换为键值对，并将键转为一个编译后的正则，如

```
mode64                 MODE=2  
```

这里key为`mode64`，value为`MODE=2`，最后返回的形式为`[(re.compile(r'\bmode64\b'), "MODE=2"), ...]`，保存到[self.state_bits](#state_bits)

###### expand_state_bits

<span id="expand_state_bits"/>

根据[parse_state_bits](#parse_state_bits)结果，替换pattern内与state_bits的正则匹配的内容为value值

```
mode64   NOREX=0  NEEDREX=1 REXW[w] REXB[b] REXX[x] REXR[r] -> 0b0100 wrxb
```

根据parse_state_bits中提供的例子，替换后结果为

```
MODE=2   NOREX=0  NEEDREX=1 REXW[w] REXB[b] REXX[x] REXR[r] -> 0b0100 wrxb
```

###### finalize_decode_conversion

<span id="finalize_decode_conversion"/>

根据传入的ICLASS，OPERAND，PATTERN和UNAME创建[iform_t](#iform_t)结构

主要通过调用[parse_one_decode_rule](#parse_one_decode_rule)解析

###### parse_one_decode_rule

<span id="parse_one_decode_rule"/>

* 首先将pattern分割，循环并进行下面判断

  * 处理`!= =`的式子，获取lhs（等式左边）

    * 若lhs是`VL`，特殊处理（具体见代码，跟VEX指令有关）
    * 若在storage_field中（storage_field是由all-field.txt读取的，在[encoder_configuration_t](#encoder_configuration_t)初始化创建时构造的），则将lhs加入modal_patterns列表
    * 特殊处理`BCRC=1`

    用来处理PATTERN中如`MOD!=3`的式子，最后将`"MOD"`加入modal_patterns列表

  * 调用[make_decode_patterns](#make_decode_patterns)处理其他类型的式子

###### parse_encode_lines

<span id="parse_encode_lines"/>

读取all-xxx-patterns的内容，并生成对应的数据结构，主要有下列几种格式和结构

* sequence

  ```
  SEQUENCE  <name>
  ```

  序列结构，根据name生成[sequencer_t](#sequencer_t)

* ntluf

  ```
  <rettype> <ntname>()::
  ```

  非终结符结构，带返回值，生成[nonterminal_t](#nonterminal_t)结构

* nt

  ```
  <ntname>()::
  ```

  非终结符结构，不带返回值，生成[nonterminal_t](#nonterminal_t)结构

* 表达式

  ```
  <cond> -> <action>
  ```

  表达式左端为条件，所有左端的表达式生成[condition_t](#condition_t)结构并加入[conditions_t](#conditions_t)列表

  右端为动作，所有右端表达式生成[action_t](#action_t)

  所有条件列表和动作列表组成一个[rule_t](#rule_t)（实际过程有点不一样，action_t列表是字符串传入rule_t后，在rule_t构造函数内生成的）

sequence和nonterminal为两种不同的符号，这种符号定义了一连串的规则和对应操作。而表达式作为具体的运算规则。因此在函数中有nt和seq两个临时变量，若当前解析了一个新的sequence，则赋值给seq，nt置为None，反之相同。每条表达式解析完后会根据当前nt或seq的值是否为None决定加入哪个。

最后返回nt ntluf seqs三个字典

###### parse_decode_lines

<span id="parse_decode_lines"/>

解析`all-enc-dec-patterns.txt`，分别有下列模式串

* ntluf  有返回值的nt，创建[nonterminal_t](#nonterminal_t)

  ```
  <rettype> <ntname>()::
  ```

* nt  创建[nonterminal_t](#nonterminal_t)

  ```
  <ntname>()::
  ```

* rule  调用[parse_decode_rule](#parse_decode_rule)，创建对应[rule_t](#rule_t)

  ```
  <action> | <cond>
  ```

###### parse_decode_rule

<span id="parse_decode_rule"/>

由上文，根据`<action> | <cond>`的格式解析action和condition，主要处理下面几种pattern

这里的action和cond表示传入的字串，最后作为action构建rule的为new_actions和new_conds

* action
  * `<name>=xxx`  若`<name>`存在与storage_fields且其属性为EI(encoder_input)，则在cond中加入当前规则式
  * `<name>[<bits>]`  将`<name>=<bits>`加入cond，并将当前规则式加入new_actions
* cond
  * 

###### make_decode_patterns

<span id="make_decode_patterns"/>

处理pattern除了`!= =`式子外的其他规则式，注意传入的式子是已经被分割后的

* 若为`<ntname>()`形式，则说明是个nonterminal，创建`blot_t("nt")`
* 若为`<name>[<bits>]`形式（如`REG[0b000]`），则根据bits内容创建`blot_t("bits")`或`blot_t("letter")`
  * 若bits为`0b`开头，则视作二进制串，作为blot_t的value值，`length`值为`len(<bits>)`，`field_name`值为`<name>`
  * 若bits为`0x`或`0X`开头，则视作十六进制串，各字段同上
  * 其他情况下，bits以`_`作为分隔符，可以建立多个blot_t。一种是01字串的，只能以二进制形式建立`blot_t("bits")`；另一种是以字母建立`blot_t("letter")`
* 若为0x 0X开头，建立`blot_t("bits")`
* 若为0b开头，建立`blot_t("bits")`
* 若为下划线分隔的数字和字母串（如`011_rrr`），则以`_`为分隔符创建`blot_t("bits")`或`blot_t("letter")`列表
* 若为字母串，则创建`blot_t("letter")`
* 若为等式，则创建`blot_t("od")`
* 若为不等式，也创建`blot_t("od")`

###### read_decoder_instruction_file

<span id="read_decoder_instruction_file"/>

* 非终结符nt，并建立[nonterminal_t](#nonterminal_t)，**一个nt对应了多组ICLASS**，且可能来自不同文件（有点类似于命名空间的概念）

  ```
  <ntname>()::
  ```

* ICLASS，一个ICLASS由一组`{}`标识，**每个ICLASS对应了多组PATTERN**。该函数主要读取ICLASS的下列属性

  * UNAME  可能不存在
  * PATTERN
  * OPERAND

* PATTERN OPERAND，一个ICLASS可能存在多组，其中OPERAND可能没有参数，则被置为`''`。最后调用[finalize_decode_conversion](#finalize_decode_conversion)读取

###### read_encoder_files

<span id="read_encoder_files"/>

该函数循环处理多个pattern文件

首先调用[expand_state_bits](#expand_state_bits)对pattern内的state_bits相关属性进行替换

然后调用[parse_encode_lines](#parse_encode_lines)解析all-enc-patterns

循环，直到文件列表的文件处理完，返回所有非终结符和序列（类型见[parse_encode_lines](#parse_encode_lines)）

###### read_decoder_files

<span id="read_decoder_files"/>

调用[read_decoder_instruction_file](#read_decoder_instruction_file)处理all-enc-patterns.txt文件



###### run

类的主方法，根据输入文件创建各种数据结构

* 调用[parse_state_bits](#parse_state_bits)方法读取all-state.txt
* 调用[read_encoder_files](#read_encoder_files)，主要处理all-enc-patterns.txt文件
* 调用[read_decoder_files](#read_decoder_files)，主要处理all-enc-instructions.txt和all-enc-dec-patterns.txt文件
* 

#### rvalue_t

<span id="rvalue_t"/>

对右值进行解析，若右值为`ntname()`的形式，则标记为non-terminal，否则将其作为二进制串或十进制串解析为value属性

#### conditions_t

<span id="conditions_t"/>

condition_t列表

#### condition_t

<span id="condition_t"/>

主要解析下面几种条件表达式

* lhs = rhs，其中rhs被初始化为[rvalue_t](#rvalue_t)
* lhs != rhs，处理方式与上面的类似
* 若既没匹配到=也没匹配到!=，以输入的字串作为lhs，'*'作为rhs构造rvalue_t（这种情况一般lhs是otherwise）

上面的lhs可以是如`name[bits]`的形式来指明位数

#### rule_t

<span id="rule_t"/>

保存一条规则，具体就是一组[condition_t](#condition_t)及其对应的[action_t](#action_t)

*其中若conditon_t名字为`ENCODER_PREFERRED`，则将该规则删掉，将`enc_preferred`置为true*

#### sequencer_t

​	<span id="sequencer_t"/>

记录保存一个从pattern中读取的sequence，内容是rule_t集合（如[parse_encode_lines](#parse_encode_lines)中读取的seq）

#### nonterminal_t

<span id="nonterminal_t"/>

记录保存一个从pattern中读取的nt或ntluf，内容是rule_t集合（如[parse_encode_lines](#parse_encode_lines)中读取的nt/ntluf）

#### iform_t

<span id="iform_t"/>



#### blot_t

解析enc-instructions的pattern生成的对应结构。一个PATTERN字符串可能根据空格分隔，生成多个blot_t

主要在[parse_one_decode_rule](#parse_one_decode_rule)中被创建，用于解析每个pattern的动作，此后与解析的其他字段创建[iform_t](#iform_t)，在此处blot_t与condition一起创建[rule_t](#rule_t)

可能有下列类型

* bits
* letters
* nt  nonterminal
* od  operand decider

##### 主要属性

* type    `bits letters nt od`
* nt    类型为nonterminal时，保存nt名字
* value    类型为bits时，保存bits的值；类型为od时，保存表达式的值（只有数字值）
* length    类型为bits时，若由hex创建则为8，由二进制串创建为二进制串长度；类型为letter时为letter长度
* letters    类型为letter时，保存letters
* field_name    类型为bits或letter时，对于类似`name[bits]`的字串保存其的name；类型为od时保存lhs
* field_offset    若建立blot_t时是由下划线分隔的字串创建，此处记录了当前创建的blot_t对应表达式所在的offset
* od_equals    类型为od时，保存运算符!=或=

#### operand_t

解析enc-instructions的operand生成的对应结构。



### 函数

#### 主程序

```python
if __name__ == '__main__':
    arg_parser = setup_arg_parser()
    (options, args ) = arg_parser.parse_args()	# 处理args
    enc_inputs = encoder_input_files_t(options)	# 设置输入文件
    enc = encoder_configuration_t(enc_inputs, options.amd_enabled)	# 初始化时解析了operands_storage文件，并且为下面的文件读取和处理创建一系列数据结构
    enc.run()
    enc.look_for_encoder_inputs()      # exploratory stuff
    enc.emit_encode_defines()  # final stuff after all tables are sized
    enc.dump_output_file_names()
    sys.exit(0)
```



## actions

### 类

#### action_t

<span id="action_t"/>



# 配置文件

架构与指令集对应关系见xed_mbuild.py  1315~1338

## default_isa

xed_mbuild.py中的1270行_configure_libxed_extensions

* datafiles/files.cfg
* datafiles/files-xregs.cfg
* datafiles/files-xmm.cfg  若没有指定avx

## via_enabled

* datafiles/files-via-padlock.cfg

## amd_enabled

支持amd cpu

* datafiles/files-amd.cfg
* datafiles/amdxop/files.cfg  若指定了avx

## knc

若指定了knm或knl或skx

* datafiles/knc/files-with-avx512f.cfg

若未指定

* datafiles/knc/files-no-avx512f.cfg

## mpx

* datafiles/mpx/files.cfg

## cet

* datafiles/cet/files.cfg

## 其他默认加入的配置

见xed_mbuild.py  1316~1338

如`_add_normal_ext(env,'rdrand')`对应的配置文件即为`datafiles/rdrand/files.cfg`

## avx对应的一些具体配置

见xed_mbuild.py  1340~1420

如

```python
if env['ivb']:
            _add_normal_ext(env,'fsgsbase')
            _add_normal_ext(env,'rdrand')
            _add_normal_ext(env,'ivbavx')
```

说明若指定了'avx'且指定了ivb模式，加入下列配置文件

* datafiles/fsgsbase/files.cfg
* datafiles/rdrand/files.cfg
* datafiles/ivbavx/files.cfg

# 重要的文件

## prep-input

build文件夹中，里面包含了所有输入文件名

## encoder

### py文件

```
pysrc/read-encfile.py		# 主程序
pysrc/genutil.py
pysrc/encutil.py
pysrc/verbosity.py
pysrc/patterns.py
pysrc/actions.py
pysrc/operand_storage.py
pysrc/opnds.py
pysrc/ild_info.py
pysrc/codegen.py
pysrc/ild_nt.py
pysrc/actions.py
pysrc/ild_codegen.py
pysrc/tup2int.py
pysrc/constraint_vec_gen.py
pysrc/xedhash.py
pysrc/ild_phash.py
pysrc/actions_codegen.py
pysrc/hashlin.py
pysrc/hashfks.py
pysrc/hashmul.py
pysrc/func_gen.py
pysrc/refine_regs.py
pysrc/slash_expand.py
pysrc/nt_func_gen.py
pysrc/scatter.py
pysrc/ins_emit.py
```

### cfg文件

xed_mbuild.py: _configure_libxed_extensions

#### 基本配置

* **default_isa**  files.cfg  file-xregs.cfg
  * **avx**  files-xmm.cfg

扩展指令集在datafile下的各个文件夹内

## encoder2

```
pysrc/genutil.py
pysrc/codegen.py
pysrc/read_xed_db.py
pysrc/opnds.py
pysrc/opnd_types.py
pysrc/cpuid_rdr.py
pysrc/slash_expand.py
pysrc/patterns.py
pysrc/gen_setup.py              
pysrc/enc2gen.py
pysrc/enc2test.py
pysrc/enc2argcheck.py
```

## decoder

<span id="decoder"></span>

```
pysrc/generator.py
pysrc/actions.py
pysrc/genutil.py
pysrc/ild_easz.py
pysrc/ild_codegen.py
pysrc/tup2int.py
pysrc/encutil.py
pysrc/verbosity.py
pysrc/ild_eosz.py
pysrc/xedhash.py
pysrc/ild_phash.py
pysrc/actions_codegen.py
pysrc/patterns.py
pysrc/operand_storage.py
pysrc/opnds.py
pysrc/hashlin.py
pysrc/hashfks.py
pysrc/ild_info.py
pysrc/ild_cdict.py
pysrc/xed3_nt.py
pysrc/codegen.py
pysrc/ild_nt.py
pysrc/hashmul.py
pysrc/enumer.py
pysrc/enum_txt_writer.py
pysrc/xed3_nt.py
pysrc/ild_disp.py
pysrc/ild_imm.py
pysrc/ild_modrm.py
pysrc/ild_storage.py
pysrc/slash_expand.py
pysrc/chipmodel.py
pysrc/flag_gen.py
pysrc/opnd_types.py
pysrc/hlist.py
pysrc/ctables.py
pysrc/ild.py
pysrc/refine_regs.py
pysrc/metaenum.py
pysrc/classifier.py
```

## datafiles

datafiles是一系列不同指令集的配置的集合，xed在生成的时候会先根据配置，将需要用到的指令集配置整合，并生成`all-[name].txt`的文件。主要有

```
all-chip-models.txt
all-conversion-table.txt
all-cpuid.txt
all-dec-instructions.txt
all-dec-patterns.txt
all-dec-spine.txt
all-element-type-base.txt
all-element-types.txt
all-enc-dec-patterns.txt
all-enc-patterns.txt
all-extra-widths.txt
all-fields.txt
all-map-descriptions.txt
all-pointer-names.txt
all-registers.txt
all-state.txt
all-widths.txt
```

### enc/dec instructions

最主要的文件，记录了各个指令的分类，指令接收的参数，输出，改变的flags等

enc-instructions和dec-instructions的差别仅在于dec中有一个nop指令的表