# 构建

xed会根据配置自动生成一系列文件和表

## encoder

### py文件

```
pysrc/read-encfile.py
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

# 源码

xed使用了一堆规则文件（在datafile内）生成对应的翻译表

## 数据结构

##### xed-types.h

定义了`xed_int8_t  xed_int16_t`等基本数据类型长度。和`xed_union16_t` `xed_union32_t` `xed_union32_t`等

##### xed_decoded_inst_t

```c
typedef struct xed_decoded_inst_s  {
    /// The _operands are storage for information discovered during
    /// decoding. They are also used by encode.  The accessors for these
    /// operands all have the form xed3_operand_{get,set}_*(). They should
    /// be considered internal and subject to change over time. It is
    /// preferred that you use xed_decoded_inst_*() or the
    /// xed_operand_values_*() functions when available.
    xed_operand_storage_t _operands;

#if defined(XED_ENCODER)
    /// Used for encode operand ordering. Not set by decode.
    xed_uint8_t _operand_order[XED_ENCODE_ORDER_MAX_OPERANDS];	// 存储各个操作数的次序及其对应的类型
    /// Length of the _operand_order[] array.
    xed_uint8_t _n_operand_order; 
#endif
    xed_uint8_t _decoded_length;

    /// when we decode an instruction, we set the _inst and get the
    /// properites of that instruction here. This also points to the
    /// operands template array.
    const xed_inst_t* _inst;

    // decoder does not change it, encoder does    
    union {
        xed_uint8_t* _enc;
        const xed_uint8_t* _dec;
    } _byte_array; 

    // The ev field is stack allocated by xed_encode(). It is per-encode
    // transitory data.
    union {
        /* user_data is available as a user data storage field after
         * decoding. It does not live across re-encodes or re-decodes. */
        xed_uint64_t user_data; 
#if defined(XED_ENCODER)
        struct xed_encoder_vars_s* ev;
#endif
    } u;
    
} xed_decoded_inst_t;
```

##### xed_encoder_request_t

```c
typedef xed_decoded_inst_t xed_encoder_request_t; 
```

##### xed_encoder_vars_s

```c
typedef struct xed_encoder_vars_s {
    /// _iforms is a dynamically generated structure containing the values of
    /// various encoding decisions
    xed_encoder_iforms_t _iforms;
       
    // the index of the iform in the xed_encode_iform_db table
    xed_uint16_t _iform_index;
    
    /// Encode output array size, specified by caller of xed_encode()
    xed_uint32_t _ilen;

    /// Used portion of the encode output array
    xed_uint32_t _olen;

    xed_uint32_t _bit_offset;
} xed_encoder_vars_t;
```

##### xed_encoder_iforms_s

```c
typedef struct xed_encoder_iforms_s {
    xed_uint32_t x_SIBBASE_ENCODE;
    xed_uint32_t x_SIBBASE_ENCODE_SIB1;
    xed_uint32_t x_SIBINDEX_ENCODE;
    xed_uint32_t x_MODRM_MOD_ENCODE;
    xed_uint32_t x_MODRM_RM_ENCODE;
    xed_uint32_t x_MODRM_RM_ENCODE_EA16_SIB0;
    xed_uint32_t x_MODRM_RM_ENCODE_EA64_SIB0;
    xed_uint32_t x_MODRM_RM_ENCODE_EA32_SIB0;
    xed_uint32_t x_SIB_NT;
    xed_uint32_t x_DISP_NT;
    xed_uint32_t x_REMOVE_SEGMENT;
    xed_uint32_t x_REX_PREFIX_ENC;
    xed_uint32_t x_PREFIX_ENC;
    xed_uint32_t x_VEXED_REX;
    xed_uint32_t x_XOP_TYPE_ENC;
    xed_uint32_t x_XOP_MAP_ENC;
    xed_uint32_t x_XOP_REXXB_ENC;
    xed_uint32_t x_VEX_TYPE_ENC;
    xed_uint32_t x_VEX_REXR_ENC;
    xed_uint32_t x_VEX_REXXB_ENC;
    xed_uint32_t x_VEX_MAP_ENC;
    xed_uint32_t x_VEX_REG_ENC;
    xed_uint32_t x_VEX_ESCVL_ENC;
    xed_uint32_t x_SE_IMM8;
    xed_uint32_t x_VSIB_ENC_BASE;
    xed_uint32_t x_VSIB_ENC;
    xed_uint32_t x_EVEX_62_REXR_ENC;
    xed_uint32_t x_EVEX_REXX_ENC;
    xed_uint32_t x_EVEX_REXB_ENC;
    xed_uint32_t x_EVEX_REXRR_ENC;
    xed_uint32_t x_EVEX_MAP_ENC;
    xed_uint32_t x_EVEX_REXW_VVVV_ENC;
    xed_uint32_t x_EVEX_UPP_ENC;
    xed_uint32_t x_AVX512_EVEX_BYTE3_ENC;
    xed_uint32_t x_UIMMv;
    xed_uint32_t x_SIMMz;
    xed_uint32_t x_SIMM8;
    xed_uint32_t x_UIMM8;
    xed_uint32_t x_UIMM8_1;
    xed_uint32_t x_UIMM16;
    xed_uint32_t x_UIMM32;
    xed_uint32_t x_BRDISP8;
    xed_uint32_t x_BRDISP32;
    xed_uint32_t x_BRDISPz;
    xed_uint32_t x_MEMDISPv;
    xed_uint32_t x_MEMDISP32;
    xed_uint32_t x_MEMDISP16;
    xed_uint32_t x_MEMDISP8;
    xed_uint32_t x_MEMDISP;
} xed_encoder_iforms_t;
```

##### xed_operand_storage_s

```c
typedef struct xed_operand_storage_s {
    xed_uint8_t agen;				// 表示该指令操作数有AGEN参数
    xed_uint8_t amd3dnow;
    xed_uint8_t asz;
    xed_uint8_t bcrc;
    xed_uint8_t cet;
    xed_uint8_t cldemote;
    xed_uint8_t df32;
    xed_uint8_t df64;
    xed_uint8_t dummy;
    xed_uint8_t encoder_preferred;
    xed_uint8_t has_sib;
    xed_uint8_t ild_f2;
    xed_uint8_t ild_f3;
    xed_uint8_t imm0;				// 设为1表示该指令有一个立即数操作数
    xed_uint8_t imm0signed;			// imm0的符号
    xed_uint8_t imm1;				// 设为1表示该指令有两个立即数操作数
    xed_uint8_t lock;
    xed_uint8_t lzcnt;
    xed_uint8_t mem0;				// 设为1表示该指令有一个内存操作数
    xed_uint8_t mem1;				// 设为1表示该指令有两个内存操作数
    xed_uint8_t modep5;
    xed_uint8_t modep55c;
    xed_uint8_t mode_first_prefix;
    xed_uint8_t mpxmode;
    xed_uint8_t must_use_evex;
    xed_uint8_t needrex;
    xed_uint8_t norex;
    xed_uint8_t no_scale_disp8;
    xed_uint8_t osz;
    xed_uint8_t out_of_bytes;
    xed_uint8_t p4;
    xed_uint8_t prefix66;
    xed_uint8_t ptr;				// 存在PTR displacement时设为1
    xed_uint8_t realmode;
    xed_uint8_t relbr;				// 存在branch displacement时设为1
    xed_uint8_t rex;
    xed_uint8_t rexb;
    xed_uint8_t rexr;
    xed_uint8_t rexrr;
    xed_uint8_t rexw;
    xed_uint8_t rexx;
    xed_uint8_t sae;
    xed_uint8_t sib;
    xed_uint8_t tzcnt;
    xed_uint8_t ubit;
    xed_uint8_t using_default_segment0;
    xed_uint8_t using_default_segment1;
    xed_uint8_t vexdest3;
    xed_uint8_t vexdest4;
    xed_uint8_t vex_c4;
    xed_uint8_t wbnoinvd;
    xed_uint8_t zeroing;
    xed_uint8_t default_seg;
    xed_uint8_t easz;
    xed_uint8_t eosz;
    xed_uint8_t first_f2f3;
    xed_uint8_t has_modrm;
    xed_uint8_t last_f2f3;
    xed_uint8_t llrc;
    xed_uint8_t mod;
    xed_uint8_t mode;
    xed_uint8_t rep;
    xed_uint8_t sibscale;
    xed_uint8_t smode;
    xed_uint8_t vex_prefix;
    xed_uint8_t vl;					// vl值，该值仅在使用到了xmm/ymm/zmm寄存器时被使用，目的是获取对应的操作数位数
    xed_uint8_t hint;
    xed_uint8_t mask;
    xed_uint8_t reg;
    xed_uint8_t rm;
    xed_uint8_t roundc;
    xed_uint8_t seg_ovd;
    xed_uint8_t sibbase;
    xed_uint8_t sibindex;
    xed_uint8_t srm;
    xed_uint8_t vexdest210;
    xed_uint8_t vexvalid;
    xed_uint8_t error;
    xed_uint8_t esrc;
    xed_uint8_t map;
    xed_uint8_t nelem;
    xed_uint8_t scale;				// 基址+index寻址中保存scale
    xed_uint8_t bcast;
    xed_uint8_t chip;
    xed_uint8_t need_memdisp;
    xed_uint8_t brdisp_width;		// 设置branch displacement字长
    xed_uint8_t disp_width;			// 设置displacement字长
    xed_uint8_t ild_seg;
    xed_uint8_t imm1_bytes;
    xed_uint8_t imm_width;			// 立即数位数
    xed_uint8_t max_bytes;
    xed_uint8_t modrm_byte;
    xed_uint8_t nominal_opcode;
    xed_uint8_t nprefixes;
    xed_uint8_t nrexes;
    xed_uint8_t nseg_prefixes;
    xed_uint8_t pos_disp;
    xed_uint8_t pos_imm;
    xed_uint8_t pos_imm1;
    xed_uint8_t pos_modrm;
    xed_uint8_t pos_nominal_opcode;
    xed_uint8_t pos_sib;
    xed_uint8_t uimm1;				// 第二个立即数操作数的数值。若一条指令含有两个立即数，第二个只有8位
    xed_uint16_t base0;				// 若有基址变址寻址，这里保存基址寄存器
    xed_uint16_t base1;
    xed_uint16_t element_size;
    xed_uint16_t index;				// 基址+index寻址中保存index
    xed_uint16_t outreg;
    xed_uint16_t reg0;				// 处理纯寄存器参数，所有寄存器参数存放在reg0~8
    xed_uint16_t reg1;
    xed_uint16_t reg2;
    xed_uint16_t reg3;
    xed_uint16_t reg4;
    xed_uint16_t reg5;
    xed_uint16_t reg6;
    xed_uint16_t reg7;
    xed_uint16_t reg8;
    xed_uint16_t seg0;				// 基址+index寻址中保存segment寄存器
    xed_uint16_t seg1;
    xed_uint16_t iclass;			// 指令助记符的iclass
    xed_uint16_t mem_width;			// 设置内存操作数字长
    xed_uint64_t disp;				// 设置displacement，分为BRDISP和PTR
    xed_uint64_t uimm0;				// 设置初始获取的立即数值，无论是有符号还是无符号都存放在这
} xed_operand_storage_t;
```

##### xed_state_s

```c
typedef struct xed_state_s {
  /// real architected machine modes
  xed_machine_mode_enum_t mmode; 
  /// for 16b/32b modes
  xed_address_width_enum_t stack_addr_width; 
} xed_state_t;
```



## encoder

### xed-encode

#### nop处理

##### xed_encode_nop

用于编码各个字节长度的nop，使用了一个固定的表

```c
#define XED_MAX_FIXED_NOPS 9
static const xed_uint8_t xed_nop_array[XED_MAX_FIXED_NOPS][XED_MAX_FIXED_NOPS] = {
    /*1B*/  { 0x90 },
    /*2B*/  { 0x66, 0x90},
    /*3B*/  { 0x0F, 0x1F, 0x00},
    /*4B*/  { 0x0F, 0x1F, 0x40, 0x00},
    /*5B*/  { 0x0F, 0x1F, 0x44, 0x00, 0x00},
    /*6B*/  { 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00},
    /*7B*/  { 0x0F, 0x1F, 0x80, 0x00, 0x00,0x00, 0x00},
    /*8B*/  { 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00,0x00, 0x00},
    /*9B*/  { 0x66, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00,0x00, 0x00},
};
```



## decoder



# 重要的一些片段

### 表之间的重要关系

* `xed_reg_enum_t` 各个值与 `name_array_xed_reg_enum_t` 的关系如下

  ```
  name_array_xed_reg_enum_t[1] 的值为 {"BNDCFGU", XED_REG_BNDCFGU} ，其中XED_REG_BNDCFGU在xed_reg_enum_t 中对应的值为1，也就是说name_array_xed_reg_enum_t中每个项的value值其实就是该数组元素的序号
  ```

  