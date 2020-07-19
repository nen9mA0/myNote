### 通用

#### 类

##### Twine

用于高效地表示两个string连接的情况

###### 成员

```cpp
union Child
{
    const Twine *twine;
    const char *cString;
    const std::string *stdString;
    const StringRef *stringRef;
    const SmallVectorImpl<char> *smallString;
    char character;
    unsigned int decUI;
    int decI;
    const unsigned long *decUL;
    const long *decL;
    const unsigned long long *decULL;
    const long long *decLL;
    const uint64_t *uHex;
};

private:
/// LHS - The prefix in the concatenation, which may be uninitialized for
/// Null or Empty kinds.
Child LHS;
/// RHS - The suffix in the concatenation, which may be uninitialized for
/// Null or Empty kinds.
Child RHS;
/// LHSKind - The NodeKind of the left hand side, \see getLHSKind().
NodeKind LHSKind;
/// RHSKind - The NodeKind of the right hand side, \see getRHSKind().
NodeKind RHSKind;
```

NodeKind是个enum，用来说明节点的数据类型

##### Triple

用于表示当前的架构、厂商等信息

###### 成员

```cpp
private:
  std::string Data;

  /// The parsed arch type.
  ArchType Arch;

  /// The parsed subarchitecture type.
  SubArchType SubArch;

  /// The parsed vendor type.
  VendorType Vendor;

  /// The parsed OS type.
  OSType OS;

  /// The parsed Environment type.
  EnvironmentType Environment;

  /// The object format type.
  ObjectFormatType ObjectFormat;
```

其中几个Type都是enum，分别对应架构、架构的子架构（如ARMv7和ARMv8），厂商，操作系统，编译环境和文件格式（ELF等）

###### 构造



### ks.cpp

#### 类

##### ks_arch

* ks_arch  用于表示当前架构的enum
* ks_opt_value  用于表示当前使用的汇编语法

```cpp
struct ks_struct {
    ks_arch arch;
    int mode;
    unsigned int errnum;
    ks_opt_value syntax;

    ks_args_ks_t init_arch = nullptr;
    const Target *TheTarget = nullptr;
    std::string TripleName;
    SourceMgr SrcMgr;
    MCAsmBackend *MAB = nullptr;
    MCTargetOptions MCOptions;
    MCRegisterInfo *MRI = nullptr;
    MCAsmInfo *MAI = nullptr;
    MCInstrInfo *MCII = nullptr;
    std::string FeaturesStr;
    MCSubtargetInfo *STI = nullptr;
    MCObjectFileInfo MOFI;
    ks_sym_resolver sym_resolver = nullptr;

    ks_struct(ks_arch arch, int mode, unsigned int errnum, ks_opt_value syntax)
        : arch(arch), mode(mode), errnum(errnum), syntax(syntax) { }
};
```



#### 函数

##### ks_open

```c
ks_err ks_open(ks_arch arch, int mode, ks_engine **result);
```

主要是判断架构类型，并且调用InitKs进行初始化

##### InitKs

```c
static ks_err InitKs(int arch, ks_engine *ks, std::string TripleName);
```

* 初始化，主要是 `llvm_ks::InitializeAllTargetInfos` `llvm_ks::InitializeAllTargetMCs` `llvm_ks::InitializeAllAsmParsers` 这三者用了一种比较有趣的实现（但感觉可读性比较低）来初始化所有的TargetInfo MC和Parser，见有趣的实现。

  对于X86，这里调用了LLVMInitializeX86TargetInfo

* 

### 有趣的实现

#### 函数

##### llvm_ks::InitializeAllTargetInfos

这个函数用于初始化所有的TargetInfo，调用为

```cpp
llvm_ks::InitializeAllTargetInfos();
```

定义为

```cpp
inline void InitializeAllTargetInfos() {
#define LLVM_TARGET(TargetName) LLVMInitialize##TargetName##TargetInfo();
#include "llvm/Config/Targets.def"
  }
```

到这里有点迷惑，但看看def的内容

```cpp
#ifndef LLVM_TARGET
#  error Please define the macro LLVM_TARGET(TargetName)
#endif

LLVM_TARGET(AArch64)
LLVM_TARGET(AMDGPU)
LLVM_TARGET(ARM)
LLVM_TARGET(BPF)
LLVM_TARGET(Hexagon)
LLVM_TARGET(Lanai)
LLVM_TARGET(Mips)
LLVM_TARGET(MSP430)
LLVM_TARGET(NVPTX)
LLVM_TARGET(PowerPC)
LLVM_TARGET(Sparc)
LLVM_TARGET(SystemZ)
LLVM_TARGET(X86)
LLVM_TARGET(XCore)


#undef LLVM_TARGET
```

在预处理后，include的内容会在include语句的位置被展开，此后因为定义了LLVM_TARGET宏，因此接下来def中的几个宏都会被展开，如`LLVM_TARGET(X86)`变为`LLVMInitializeX86TargetInfo()`。此后该函数被inline到源文件并执行

此后，在执行其他两个初始化函数的时候，重定义了LLVM_TARGET，如

```c
  inline void InitializeAllTargetMCs() {
#define LLVM_TARGET(TargetName) LLVMInitialize##TargetName##TargetMC();
#include "llvm/Config/Targets.def"
  }
```

这样，上述的过程又以新的形式被展开

