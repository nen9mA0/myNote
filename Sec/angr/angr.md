# angr

## 核心概念

### Top Level Interfaces 顶层接口

#### 载入程序

```python
import angr
proj = angr.Project('/bin/true')
```

#### 基本属性

首先，我们有一些基本的关于项目文件的属性，如：

* .arch  CPU架构，这是一个**archinfo.Arch**对象，常用的属性有
  * **arch.bits**  一个word的位数
  * **arch.bytes**  一个word的字节数
  * **arch.memory_endness**  地址范围
* .filename  文件名（绝对路径）
* .entry  程序入口点

```python
import monkeyhex # this will format numerical results in hexadecimal

>>> proj.arch
<Arch AMD64 (LE)>
>>> proj.entry
0x401670
>>> proj.filename
'/bin/true'
```

#### 加载器

将一个二进制文件加载到虚拟地址中是很繁琐的，我们有一个叫**CLE**的模块来干这件事。CLE的处理结果可以在loader属性中被访问，下一章将详细介绍，这里展示加载一个动态链接库文件并进行一些基本属性查询。

```python
>>> proj.loader
<Loaded true, maps [0x400000:0x5004000]>

>>> proj.loader.shared_objects # may look a little different for you!
{'ld-linux-x86-64.so.2': <ELF Object ld-2.24.so, maps [0x2000000:0x2227167]>,
 'libc.so.6': <ELF Object libc-2.24.so, maps [0x1000000:0x13c699f]>}

>>> proj.loader.min_addr
0x400000
>>> proj.loader.max_addr
0x5004000

>>> proj.loader.main_object  # we've loaded several binaries into this project. Here's the main one!
# 这里显示的是加载器加载的主模块
<ELF Object true, maps [0x400000:0x60721f]>

>>> proj.loader.main_object.execstack  # sample query: does this binary have an executable stack?
# 堆栈是否有可执行属性
False
>>> proj.loader.main_object.pic  # sample query: is this binary position-independent?
# 是否为地址无关代码
True
```

#### 工厂函数

angr里有很多类模块，大多数需要提供一个项目文件以用于实例化。angr提供了一系列工厂函数：**project.factory**，以便于使用不同的构造器来构建实例。

这章同样会介绍一些angr的基本概念

##### Blocks

**project.factory.block()** 可以用于从一个给定的地址提取响应的**基本块**。这是angr的一个重要概念：angr通过一系列的基本块单元分析代码。通过这个函数可以获得一个block对象，以获取一些有用的信息：

```python
>>> block = proj.factory.block(proj.entry) # lift a block of code from the program's entry point
# 取程序入口点的基本块
<Block for 0x401670, 42 bytes>

>>> block.pp()                          # pretty-print a disassembly to stdout
0x401670:       xor     ebp, ebp
0x401672:       mov     r9, rdx
0x401675:       pop     rsi
0x401676:       mov     rdx, rsp
0x401679:       and     rsp, 0xfffffffffffffff0
0x40167d:       push    rax
0x40167e:       push    rsp
0x40167f:       lea     r8, [rip + 0x2e2a]
0x401686:       lea     rcx, [rip + 0x2db3]
0x40168d:       lea     rdi, [rip - 0xd4]
0x401694:       call    qword ptr [rip + 0x205866]

>>> block.instructions                  # how many instructions are there?
0xb
>>> block.instruction_addrs             # what are the addresses of the instructions?
[0x401670, 0x401672, 0x401675, 0x401676, 0x401679, 0x40167d, 0x40167e, 0x40167f, 0x401686, 0x40168d, 0x401694]
```

此外，可以用block对象来获取汇编代码的不同表示

```python
>>> block.capstone                       # capstone disassembly
# capstone的反汇编对象
<CapstoneBlock for 0x401670>
>>> block.vex                            # VEX IRSB (that's a python internal address, not a program address)
# pyvex IR中间表示语言
<pyvex.block.IRSB at 0x7706330>
```

##### States

**Project**对象仅仅表示程序的初始状态，当你使用angr执行程序时，会产生一个特殊的对象**SimState**用于代表被分析的程序状态。

```python
>>> state = proj.factory.entry_state()
<SimState @ 0x401670>
```

SimState对象包含程序的内存、寄存器、文件系统数据等。所有的会随程序执行改变的数据都被SimState所记录。稍后我们将深入介绍如何与SimState进行交互，现在先给个简单的例子

```python
>>> state.regs.rip        # get the current instruction pointer
<BV64 0x401670>
>>> state.regs.rax
<BV64 0x1c>
>>> state.mem[proj.entry].int.resolved  # interpret the memory at the entry point as a C int
# 以int数据类型返回程序入口点的数据
<BV32 0x8949ed31>
```

注意这些数据的类型不是python的int而是bitvector。因为python的int类型行为与C的不一样（比如它不会溢出）。所以我们用位向量，可以理解为用一串二进制数来表示一个int型数据。注意每个bitvector类型都有**length**属性。

以后会说明怎么具体使用它们，现在先给出几个python的int型与bitvector互相转换的例子

```python
>>> bv = state.solver.BVV(0x1234, 32)       # create a 32-bit-wide bitvector with value 0x1234
<BV32 0x1234>                               # BVV stands for bitvector value
>>> state.solver.eval(bv)                # convert to python int
0x1234
```

你可以使用bitvector给寄存器或内存赋值，也可以直接赋一个python的int类型的值，它会被转换为大小合适的bitvector类型

```python
>>> state.regs.rsi = state.solver.BVV(3, 64)
>>> state.regs.rsi
<BV64 0x3>

>>> state.mem[0x1000].long = 4
>>> state.mem[0x1000].long.resolved
<BV64 0x4>
```

**mem**接口似乎初看有点令人疑惑，因为它使用了一些python的语法糖。主要用法有

* 使用array[index]的形式指定一个内存地址
* 使用\.\<type\>指定该地址的数据应该被解释为什么类型（char,short,int,long,size_t,uint8_t,uint16_t等）
* 通过上面的语法可以
  * 给指定内容复制，既可以是python的int也可以是bitvector
  * 使用 **.resolved** 获取指定内容的值（以bitvector形式）
  * 使用 **.concrete** 获取指定内容的值（以python int形式）

最后，在你尝试获取寄存器时可能会遇到如下的值

```python
>>> state.regs.rdi
<BV64 reg_48_11_64{UNINITIALIZED}>
```

这仍是一个64位的bitvector但他没有值。取而代之的是一个名字。

这被称为符号变量，并被用于符号执行。在下两章将详细叙述。

##### Simulation Managers

当程序运行到某一状态时，必须有一个方法让它知道下一次应执行到什么状态。

Simulation Managers是angr用于管理执行、模拟过程的主要接口。当你调用它时应传入一个state对象。下面简要说明如何通过之前创建过的state对象调用**SimulationManager**

```python
>>> simgr = proj.factory.simulation_manager(state)
<SimulationManager with 1 active>
>>> simgr.active
[<SimState @ 0x401670>]
```

SimulationManager可以保存多个状态。默认的状态属性**active**使用我们传入的state初始化。我们可以通过 **simgr.active[0]** 查看我们传入的state。

现在我们执行一下程序

```python
>>> simgr.step()
```

我们刚刚进行了一次符号执行，我们可以查看**active**，注意它被更新了。但程序初始状态的记录并没有改变，之前传入的state对象不会因执行而改变，因此传入**SimulationManager**的**SimState**对象可以作为一个基本状态并从该处多次执行。

##### Analyses

angr有一些内建的包用来对程序进行分析，如下：

```python
>>> proj.analyses.            # Press TAB here in ipython to get an autocomplete-listing of everything:
 proj.analyses.BackwardSlice        proj.analyses.CongruencyCheck      proj.analyses.reload_analyses       
 proj.analyses.BinaryOptimizer      proj.analyses.DDG                  proj.analyses.StaticHooker          
 proj.analyses.BinDiff              proj.analyses.DFG                  proj.analyses.VariableRecovery      
 proj.analyses.BoyScout             proj.analyses.Disassembly          proj.analyses.VariableRecoveryFast  
 proj.analyses.CDG                  proj.analyses.GirlScout            proj.analyses.Veritesting           
 proj.analyses.CFG                  proj.analyses.Identifier           proj.analyses.VFG                   
 proj.analyses.CFGEmulated          proj.analyses.LoopFinder           proj.analyses.VSA_DDG               
 proj.analyses.CFGFast              proj.analyses.Reassembler
```

一些方法在下面的章节将介绍。但如果你希望知道一些模块的具体使用方法，请查看[API文档](http://angr.io/api-doc/angr.html?highlight=cfg#module-angr.analysis)

下面是一个简单的例子用于获取程序的控制流图

```python
# Originally, when we loaded this binary it also loaded all its dependencies into the same virtual address  space
# This is undesirable for most analysis.
>>> proj = angr.Project('/bin/true', auto_load_libs=False)
>>> cfg = proj.analyses.CFGFast()
<CFGFast Analysis Result at 0x2d85130>

# cfg.graph is a networkx DiGraph full of CFGNode instances
# You should go look up the networkx APIs to learn how to use this!
>>> cfg.graph
<networkx.classes.digraph.DiGraph at 0x2da43a0>
>>> len(cfg.graph.nodes())
951

# To get the CFGNode for a given address, use cfg.get_any_node
>>> entry_node = cfg.get_any_node(proj.entry)
>>> len(list(cfg.graph.successors(entry_node)))
2
```

### Loading a Binary 载入一个二进制文件

上一节中你学到了如何载入/bin/true，并且用不载入动态链接库的方式重新载入/bin/true。并且学习了一些loader的用法。下面将深入探讨这个模块。

这里简单提一下angr的二进制加载模块，CLE。CLE的职责是加载一个二进制文件及其库文件，并用一种易于处理的形式传递给angr的其他模块。

#### 加载器

重新加载一次/bin/true

```python
>>> import angr, monkeyhex
>>> proj = angr.Project('/bin/true')
>>> proj.loader
<Loaded true, maps [0x400000:0x5008000]>
```

##### 被加载对象

**cle.loader**对象用于表示所有被加载并映射的二进制文件。每个二进制文件被加载时都传给loader对象的后端（**cle.Backend**），由其来处理不同的文件格式，如**cle.ELF**用于加载ELF文件

在内存中也有一些不映射到任何二进制文件的对象，如用于提供线程本地存储（TLS）支持的对象及未解析的外部符号。

可以使用loader.all_objects获取CLE的完整信息

```python
# All loaded objects
>>> proj.loader.all_objects
[<ELF Object fauxware, maps [0x400000:0x60105f]>,
 <ELF Object libc.so.6, maps [0x1000000:0x13c42bf]>,
 <ELF Object ld-linux-x86-64.so.2, maps [0x2000000:0x22241c7]>,
 <ELFTLSObject Object cle##tls, maps [0x3000000:0x300d010]>,
 <KernelObject Object cle##kernel, maps [0x4000000:0x4008000]>,
 <ExternObject Object cle##externs, maps [0x5000000:0x5008000]>

# This is the "main" object, the one that you directly specified when loading the project
>>> proj.loader.main_object
<ELF Object true, maps [0x400000:0x60105f]>

# This is a dictionary mapping from shared object name to object
>>> proj.loader.shared_objects
{ 'libc.so.6': <ELF Object libc.so.6, maps [0x1000000:0x13c42bf]>
  'ld-linux-x86-64.so.2': <ELF Object ld-linux-x86-64.so.2, maps [0x2000000:0x22241c7]>}

# Here's all the objects that were loaded from ELF files
# If this were a windows program we'd use all_pe_objects!
>>> proj.loader.all_elf_objects
[<ELF Object true, maps [0x400000:0x60105f]>,
 <ELF Object libc.so.6, maps [0x1000000:0x13c42bf]>,
 <ELF Object ld-linux-x86-64.so.2, maps [0x2000000:0x22241c7]>]

# Here's the "externs object", which we use to provide addresses for unresolved imports and angr internals
>>> proj.loader.extern_object
<ExternObject Object cle##externs, maps [0x5000000:0x5008000]>

# This object is used to provide addresses for emulated syscalls
>>> proj.loader.kernel_object
<KernelObject Object cle##kernel, maps [0x4000000:0x4008000]>

# Finally, you can to get a reference to an object given an address in it
>>> proj.loader.find_object_containing(0x400000)
<ELF Object true, maps [0x400000:0x60105f]>
```

可以通过直接与对象交互的方式获取对象属性

```python
>>> obj = proj.loader.main_object

# The entry point of the object
>>> obj.entry
0x400580

>>> obj.min_addr, obj.max_addr
(0x400000, 0x60105f)

# Retrieve this ELF's segments and sections
>>> obj.segments
<Regions: [<ELFSegment offset=0x0, flags=0x5, filesize=0xa74, vaddr=0x400000, memsize=0xa74>,
           <ELFSegment offset=0xe28, flags=0x6, filesize=0x228, vaddr=0x600e28, memsize=0x238>]>
>>> obj.sections
<Regions: [<Unnamed | offset 0x0, vaddr 0x0, size 0x0>,
           <.interp | offset 0x238, vaddr 0x400238, size 0x1c>,
           <.note.ABI-tag | offset 0x254, vaddr 0x400254, size 0x20>,
            ...etc

# You can get an individual segment or section by an address it contains:
>>> obj.find_segment_containing(obj.entry)
<ELFSegment offset=0x0, flags=0x5, filesize=0xa74, vaddr=0x400000, memsize=0xa74>
>>> obj.find_section_containing(obj.entry)
<.text | offset 0x580, vaddr 0x400580, size 0x338>

# Get the address of the PLT stub for a symbol
>>> addr = obj.plt['abort']
>>> addr
0x400540
>>> obj.reverse_plt[addr]
'abort'

# Show the prelinked base of the object and the location it was actually mapped into memory by CLE
>>> obj.linked_base
0x400000
>>> obj.mapped_base
0x400000
```

#### 符号和重定位

可以使用CLE获取符号信息。符号是可执行文件格式的一个基本概念，用于将一个地址映射为一个名称。

最简单的获取符号的方法就是**find_symbol**，可以传入符号获取地址，或传入地址获取符号

```python
>>> malloc = proj.loader.find_symbol('malloc')
>>> malloc
<Symbol "malloc" in libc.so.6 at 0x1054400>
```

符号最有用的属性就是名称、所有者与地址，但地址可能不是一成不变的。有三种方法获取符号地址

* **rebased_addr**  符号所在的全局地址，即为加载后的虚拟地址
* **linked_addr**  文件偏移地址
* **relative_addr**  RVA

```c
>>> strcmp.name
'strcmp'

>>> strcmp.owner
<ELF Object libc-2.23.so, maps [0x1000000:0x13c999f]>

>>> strcmp.rebased_addr
0x1089cd0
>>> strcmp.linked_addr
0x89cd0
>>> strcmp.relative_addr
0x89cd0
```

此外为了提供更多的调试信息，符号支持解析动态链接。比如libc提供strcmp函数，主程序需要strcmp函数。如果我们用CLE获取主程序中strcmp符号的地址，CLE将告诉我们这是个导入符号。导入符号并不和一个特定地址联系，但它们提供了一个用于解析它们的符号引用。可以使用**resolveby**解析。

```python
>>> strcmp.is_export
True
>>> strcmp.is_import
False

# On Loader, the method is find_symbol because it performs a search operation to find the symbol.
# On an individual object, the method is get_symbol because there can only be one symbol with a given name.
>>> main_strcmp = proj.loader.main_object.get_symbol('strcmp')
>>> main_strcmp
<Symbol "strcmp" in fauxware (import)>
>>> main_strcmp.is_export
False
>>> main_strcmp.is_import
True
>>> main_strcmp.resolvedby
<Symbol "strcmp" in libc.so.6 at 0x1089cd0>
```

所有导入或导出的符号都被一个叫重定位的过程处理。重定位即为：如果有一个导入符号与导出符号相匹配，将导出符号地址写到对应的导入表项中。我们可以通过**obj.relocs**获取obj的所有**relocation类列表**，或使用**obj.imports**获取重定位名称与**reloction类**映射的**字典**。没有对应方法获取导出符号。

可以使用 **.symbol**获取一个**relocation类**对应的**Symbol类**。重定位的地址可以使用任何Symbol类中支持的地址标识符类型来获取。同样，你可以通过 **.owner**获取Symbol类拥有者的Object类（即拥有Symbol导入符号的Object）

```python
# Relocations don't have a good pretty-printing, so those addresses are python-internal, unrelated to our program
>>> proj.loader.shared_objects['libc.so.6'].imports
{u'__libc_enable_secure': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x4221fb0>,
 u'__tls_get_addr': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x425d150>,
 u'_dl_argv': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x4254d90>,
 u'_dl_find_dso_for_object': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x425d130>,
 u'_dl_starting_up': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x42548d0>,
 u'_rtld_global': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x4221e70>,
 u'_rtld_global_ro': <cle.backends.relocations.generic.GenericJumpslotReloc at 0x4254210>}
```

若一个导入符号无法找到对应导出符号，如无法找到该链接库，CLE将自动更新loader.extern_object，将该符号声明为一个export类型（即符号声明不在当前Object）。

#### 加载选项

若使用angr.Project加载程序，且希望传入一些选项到cle.loader，可以在**Project**类构造函数中以关键字参数的方式传递，该构造函数会将对应的参数传给cle.loader。具体可见[CLE API docs](angr.io/api-doc/cle.html)。我们将介绍一些常用的选项

##### 基本选项

我们已经讨论了**auto_load_libs**参数：它用于选择是否自动加载程序引用的共享库，默认情况下是加载的

**except_missing_libs**若为True，当无法找到一个程序引用的共享库时将抛出异常

**force_load_libs** 传入一个字符串列表，用于强制加载对应的共享库

**skip_libs**  传入一个字符串列表，字符串列出的共享库将不被加载

**ld_path** 可以添加一个共享库的搜索路径，默认的搜索路径为**被加载的程序目录**，**当前目录**及**系统库目录**

##### 用于指定二进制文件的选项

用于对指定二进制文件使用单独的选项

* **main_opts**  用于main_object，是一个选项名与值映射的字典

* **lib_opts**  一个库名与对应选项字典映射的字典，选项字典是一个选项名与其值映射的字典

常用选项

* backend 指定loader使用的后端，可以是一个类或名称
* base_addr  指定加载的基址
* entry_point  指定入口点
* arch  指定使用的架构

```python
>>> angr.Project('examples/fauxware/fauxware', main_opts={'backend': 'blob', 'arch': 'i386'}, lib_opts={'libc.so.6': {'backend': 'elf'}})
<Project examples/fauxware/fauxware>
```

#### 后端

CLE有加载ELF,PE,CGC,Mach-O,core dump等文件的后端，也可以使用IDA加载二进制文件或将文件加载到一块平坦的地址空间。大多数情况下CLE会自动检测文件类型并使用对应后端，所以一般情况下不需要指定后端。

可以通过上节的方法让CLE使用指定后端。一些情况下，后端无法自动识别架构，需要指定arch参数。angr会根据传入的字符串自动识别相应的架构。

* elf   基于PyELFTools的elf加载器
* pe   基于PEFile的PE加载器
* mach-o  mach-o加载器，不支持动态链接和重定向
* cgc  Cyber Grand Challenge文件加载器
* backedcgc  支持指定内存地址和register backers的cgc加载器
* elfcore  core dump加载器
* ida  使用一个IDA实例对象加载文件，需要指定arch（好像新版的manual把这个删了）
* blob  将文件加载到一个平坦的内存空间，需要指定架构

#### 符号函数

默认情况下，Project类会尝试将对外部库函数的调用替换为被称为**SimProcedures**的符号函数：实际上就是用于模拟库函数对状态影响的python函数。我们已经在SimProcedures中实现了[一系列库函数](<https://github.com/angr/angr/tree/master/angr/procedures>)。这些内建函数的列表可以在**angr.SIM_PROCEDURES**字典中被找到，这是个嵌套的字典，可以用第一个关键字指定一个库文件（libc,posix,win32,stubs等），第二个关键字指定函数名。使用这些内建函数替换库函数调用可以使分析更加准确。

若对于某个库函数找不到对应的符号函数：

* 若**auto_load_libs**为真（默认），系统将执行库函数本身。这可能不是我们所期望的，取决于库函数功能。比如，某些函数分析起来很复杂且会导致程序状态数量激增。
* 若**auto_load_libs**为假，系统不会执行库函数，而是将这些函数调用指向SimProcedure的stub模块中一个称为**ReturnUnconstrained**的函数，这个函数返回一个无约束的符号量。
* 若**use_sim_procedures**为假（默认为真）（这是`angr.Project`的选项，而非`cle.Loader`），则只有外部对象提供的符号会被替换，且替换为上述的ReturnUnconstrained函数
* 你可以通过传递**exclude_sim_procedures_list**和**exclude_sim_procedures_func**参数给angr.Project来指定不被SimProcedures模拟的函数
* **angr.Project._register_object** 源码提供了该功能的具体算法

##### 钩子

angr使用python函数替换原库函数的原理被称为函数钩子，你同样可以自己使用该功能。当模拟运行时，每执行一步angr会检查当前地址有没有钩子，如果有，执行hook指定的代码。API为**proj.hook(addr,hook)**，其中hook是一个**SimProcedure**实例，可以使用is_hooked，.unhook，.hooked_by等方法。

还有一种方法可以用于自定义函数钩子，即将 **proj.hook(addr)** 作为一个装饰器使用。如果你这样做，你还可以给出一个**length**参数用于指定你的函数执行完后应跳转到原地址后几个字节处开始执行。

```python
>>> stub_func = angr.SIM_PROCEDURES['stubs']['ReturnUnconstrained'] # this is a CLASS
>>> proj.hook(0x10000, stub_func())  # hook with an instance of the class

>>> proj.is_hooked(0x10000)            # these functions should be pretty self-explanitory
True
>>> proj.unhook(0x10000)
>>> proj.hooked_by(0x10000)
<ReturnUnconstrained>

>>> @proj.hook(0x20000, length=5)
... def my_hook(state):
...     state.regs.rax = 1

>>> proj.is_hooked(0x20000)
True
```

此外，你可以用**proj.hook_symbol(name,hook)**，将符号名作为第一个参数，用于hook符号所在的地址。这种特性的一个重要应用是扩展内建SimProcedures函数的应用，因为这些库函数都是一个类，你可以将其作为子类，改写其中的一些方法，并将其作为你的自定义钩子函数。

#### 本节介绍的模块与方法

##### Project

```python
angr.Project(filename)        #创建一个angr工程

参数：
auto_load_libs
except_missing_libs
force_load_libs
skip_libs
ld_path
main_opts
lib_opts
backend
base_addr
entry_point
arch
exclude_sim_procedures_list
exclude_sim_procedures_func
```

##### loader

```python
属性：
.all_objects   显示所有模块（主程序、动态链接库、内核、外部引用、TLS等）
.main_object   显示主模块
.shared_object
.all_elf_objects
.extern_object
.kernel_object

方法:
.find_object_containing()  返回给定地址所在的模块
.find_symbol()        返回符号名对应的symbol对象
```

##### Object

```python
属性：
.entry            入口点
.min_addr
.max_addr
.segments
.sections
.plt            输入符号名返回地址
.reverse_plt    输入地址返回符号名
.link_base        链接时指定的虚拟地址
.mapped_base    实际的加载地址

方法：
.find_segment_containing()
.find_section_containing()
```

##### symbol

```python
属性：
.name
.owner_obj        符号链接到的地址
.rebased_addr    虚拟地址
.linked_addr    文件偏移地址
.relative_addr    RVA

方法：
.is_export
.is_import
.resolvedby        解析符号所对应地址
```

##### SimProcedure

```python
属性：
.is_hooked
.unhook
.hooked_by
```

### Solver Engine 求解器引擎

angr最厉害的地方不在于它的模拟器功能，而在于他可以执行我们称为符号变量的东西。与保存一个具体数值的普通变量不同，符号变量保存一个符号，实际上就是一个名称。在符号变量上执行算术操作将生成一个操作树（在编译原理中被称为抽象语法树，AST）。AST可以被翻译为SMT求解器（如z3）的约束。下面我们将学习angr是如何解决一个类似“给定一个输出，求解输入”的问题。

#### 使用Bitvectors

##### 声明

```python
# 这里首先需要创建state，从而获取一个上下文
>>> import angr, monkeyhex
>>> proj = angr.Project('/bin/true')
>>> state = proj.factory.entry_state()


# 64-bit bitvectors with concrete values 1 and 100
>>> one = state.solver.BVV(1, 64)
>>> one
 <BV64 0x1>
>>> one_hundred = state.solver.BVV(100, 64)
>>> one_hundred
 <BV64 0x64>

# create a 27-bit bitvector with concrete value 9
>>> weird_nine = state.solver.BVV(9, 27)
>>> weird_nine
<BV27 0x9>
```

##### 运算

```python
>>> one + one_hundred
<BV64 0x65>

# You can provide normal python integers and they will be coerced to the appropriate type:
>>> one_hundred + 0x100
<BV64 0x164>

# The semantics of normal wrapping arithmetic apply
>>> one_hundred - one*200
<BV64 0xffffffffffffff9c>
```

**注意：参与运算的两个BVV位长必须相等**

要使用不同位长的BVV进行运算需要进行数位扩展

* **zero_extend**  无符号扩展
* **sign_extend**  有符号扩展

```python
>>> weird_nine.zero_extend(64 - 27)
<BV64 0x9>
>>> one + weird_nine.zero_extend(64 - 27)
<BV64 0xa>
>>> two = state.solver.BVV(-2,27)
>>> two
<BV27 0x7fffffe>
>>> unsignextend = two.zero_extend(64-27)    #无符号扩展
>>> unsignextend
<BV64 0x7fffffe>
>>> signextend = two.sign_extend(64-27)        #有符号扩展
>>> signextend
<BV64 0xfffffffffffffffe>
```

##### 声明符号变量

```python
# Create a bitvector symbol named "x" of length 64 bits
>>> x = state.solver.BVS("x", 64)
>>> x
<BV64 x_9_64>
>>> y = state.solver.BVS("y", 64)
>>> y
<BV64 y_10_64>
```

##### 符号变量的计算

```python
>>> x + one
<BV64 x_9_64 + 0x1>

>>> (x + one) / 2
<BV64 (x_9_64 + 0x1) / 0x2>

>>> x - y
<BV64 x_9_64 - y_10_64>
```

符号变量运算不会得到一个具体值，而是一个AST。每一个符号变量（包括bitvector）都是一个AST（常量也是一个深度为1的AST）

##### 符号变量的属性

每个AST都有.op和.args属性

op属性定义运算符，args属性定义操作数。由于本质为AST，所以args只可能是其他AST，或是BVV/BVS

```python
>>> tree = (x + 1) / (y + 2)
>>> tree
<BV64 (x_9_64 + 0x1) / (y_10_64 + 0x2)>
>>> tree.op
'__div__'
>>> tree.args
(<BV64 x_9_64 + 0x1>, <BV64 y_10_64 + 0x2>)
>>> tree.args[0].op
'__add__'
>>> tree.args[0].args
(<BV64 x_9_64>, <BV64 0x1>)
>>> tree.args[0].args[1].op
'BVV'
>>> tree.args[0].args[1].args
(1, 64)
```

#### 符号约束

将比较运算符用于两个AST变量将产生一个新的AST：符号布尔量

```python
>>> x == 1
<Bool x_9_64 == 0x1>
>>> x == one
<Bool x_9_64 == 0x1>
>>> x > 2
<Bool x_9_64 > 0x2>
>>> x + y == one_hundred + 5
<Bool (x_9_64 + y_10_64) == 0x69>
>>> one_hundred > 5        #注意这里如果AST是常量将直接产生比较运算后的结果
<Bool True>
>>> one_hundred > -5    #注意，这里的-5默认会被初始化为<BV64 0xfffffffffffffffb>（即无符号整型），
                        #因此比较结果为False，若想使用有符号数比较应调用one_hundred.SGT(-5)
                        #signed greater than
<Bool False>
```

上述结果可以看出，默认的比较是无符号比较。若需要有符号比较，可以使用 `one_hundred.SGT(-5)` 即signed greater than。其他运算符见

注意上面说明所有的AST比较运算将生成一个AST，因此如果要获得表达式的值（即真或假），需使用.is_true和.is_false（即使这些符号量被赋了确定值）

```python
>>> yes = one == 1
>>> no = one == 2
>>> maybe = x == y
>>> state.solver.is_true(yes)
True
>>> state.solver.is_false(yes)
False
>>> state.solver.is_true(no)
False
>>> state.solver.is_false(no)
True
>>> state.solver.is_true(maybe)    #注意未约束的符号量的输出，两个函数都为False
False
>>> state.solver.is_false(maybe)
False
```

#### 约束求解

与z3里的差不多，直接贴代码

```python
x = state.solver.BVS("x",64)
y = state.solver.BVS("y",64)
state.solver.add(x>y)
state.solver.add(y>2)
state.solver.add(10>x)
print state.solver.eval(x)
```

输出：9（因为这不是一个单解的约束，因此答案可能是4~9中的任意一个）

```python
input = state.solver.BVS('input', 64)
operation = (((input + 4) * 3) >> 1) + input
output = 200
state.solver.add(operation == output)
state.solver.eval(input)
```

输出：0x3333333333333381

添加约束后，可以使用.satisfiable()检查约束是否有解

```python
>>> state.solver.add(input < 2**32)
>>> state.satisfiable()
False
```

此外，可以添加更复杂的约束

```python
>>> state.solver.add(x - y >= 4)
>>> state.solver.add(y > 0)
>>> state.solver.eval(x)
5
>>> state.solver.eval(y)
1
>>> state.solver.eval(x + y)
6
```

可以看到eval函数是一个通用的将bitvector符号值转换成python元类型的函数

##### 注意

**符号不会和状态绑定，因此在某个状态创建的符号在其他状态下依旧可以调用**

#### 浮点数

因为z3支持IEEE754浮点，因此angr也进行了适配，差别只是z3使用width作为参数，而angr使用了枚举来定义类型

```python
# fresh state
>>> state = proj.factory.entry_state()
>>> a = state.solver.FPV(3.2, state.solver.fp.FSORT_DOUBLE)
>>> a
<FP64 FPV(3.2, DOUBLE)>

>>> b = state.solver.FPS('b', state.solver.fp.FSORT_DOUBLE)
>>> b
<FP64 FPS('FP_b_0_64', DOUBLE)>

>>> a + b
<FP64 fpAdd('RNE', FPV(3.2, DOUBLE), FPS('FP_b_0_64', DOUBLE))>

>>> a + 4.4
<FP64 FPV(7.6000000000000005, DOUBLE)>

>>> b + 2 < 0
<Bool fpLT(fpAdd('RNE', FPS('FP_b_0_64', DOUBLE), FPV(2.0, DOUBLE)), FPV(0.0, DOUBLE))>
```

##### 舍入模式

可以使用solver.fp.RM_*指定浮点数舍入模式，若要指定舍入模式，在调用的时候使用solver.fpAdd代替重载的+运算符，并将舍入模式作为第一个参数

可用参数：

* RM_RNE  round to nearest,ties to even，默认
* RM_RNA  round to nearest,ties away from zero
* RM_RTZ   round toward 0
* RM_RTP   round toward +inf
* RM_RTN   round toward -inf

[舍入模式](<https://en.wikipedia.org/wiki/IEEE_754#Rounding_rules>)

##### 数据类型转换

###### 二进制转换

直接将浮点数与其二进制表示互相转换，包含符号位

```python
>>> a.raw_to_bv()
<BV64 0x400999999999999a>
>>> b.raw_to_bv()
<BV64 fpToIEEEBV(FPS('FP_b_0_64', DOUBLE))>

>>> state.solver.BVV(0, 64).raw_to_fp()
<FP64 FPV(0.0, DOUBLE)>
>>> state.solver.BVS('x', 64).raw_to_fp()
<FP64 fpToFP(x_1_64, DOUBLE)>
```

###### 类型转换

```python
>>> a
<FP64 FPV(3.2, DOUBLE)>
>>> a.val_to_bv(12)
<BV12 0x3>
>>> a.val_to_bv(12).val_to_fp(state.solver.fp.FSORT_FLOAT)
<FP32 FPV(3.0, FLOAT)>
```

#### 其他求解选项

* `solver.eval(e)`  给出一个可能的解

* `solver.eval_one(e)`  给出一个解，且当问题不止有一个解时会抛出一个异常

* `solver.eval_upto(e, n)`  给出最多n个解

* `solver.eval_atleast(e, n)`  给出n个解，若解数小于n会抛出异常

* `solver.eval_exact(e, n)`  给出n个解，若解数不等于n时会抛出异常

* `solver.min(e)`  给出最小的一个可能解

* `solver.max(e)`  给出最大的一个可能解

此外上述的方法都可以接收下列参数

* `extra_constraints`  以tuple形式提供，作为额外的约束加入求解，但不会被加入当前状态

* `cast_to`  使得返回的结果转换为指定的类型，这里指定的类型只能是int或bytes
  
  ```python
  >>> state.solver.eval(state.solver.BVV(0x41424344, 32), cast_to=bytes)
  b'ABCD'
  ```

#### 本节介绍的模块与方法

##### Bitvectors

```python
state.solver.BVV    #构造一个Bitvectors常数
state.solver.BVS    #构造一个Bitvectors符号量
    #此处state为SimState类，见第一节顶层接口的States
方法：
.zero_extend        #无符号扩展
.sign_extend        #有符号扩展
属性：
.op                    #运算符
.args                #操作数
                    #具体行为可看下面的程序
```

###### 例子

```python
>>> one = state.solver.BVV(1,64)
>>> one
<BV64 0x1>
>>> one.op
BVV
>>> one.args
(1L, 64)
>>> x = state.solver.BVS("x",64)
>>> x
BVS
>>> x.op
BVS
>>> x.args
('x_0_64', None, None, None, False, False, None)
>>> y = x+one
>>> y
<BV64 x_0_64 + 0x1>
>>> y.op
__add__
>>> y.args
(<BV64 x_0_64>, <BV64 0x1>)
```

##### is_true/is_false

同样是SimSolver的方法

###### 例子

```python
>>> yes = one == 1
>>> yes
<Bool True>
>>> state.solver.is_true(yes)
True
```

##### 关于求解器的方法

```python
solver.add
solver.eval
solver.eval_one   #若多于一解将抛出异常
solver.eval_upto(expression,n)  #求出n个解
solver.eval_atleast(expression,n) #如果少于n解抛出异常
solver.min
solver.max            #可能的解的数量
solver.satisfiable
```

上述方法都可以有以下参数：

* extra_constraints  传递一个约束式的tuple
* cast_to  一般为cast_to=str，用于将结果转换为对应ASCII

```python
>>> state.solver.eval(state.solver.BVV(0x41424344, 32), cast_to=str)
"ABCD"
```

### Program State 程序状态

#### 访问寄存器和内存

```python
>>> import angr, claripy
>>> proj = angr.Project('/bin/true')
>>> state = proj.factory.entry_state()

# copy rsp to rbp
>>> state.regs.rbp = state.regs.rsp

# store rdx to memory at 0x1000
>>> state.mem[0x1000].uint64_t = state.regs.rdx

# dereference rbp
>>> state.regs.rbp = state.mem[state.regs.rbp].uint64_t.resolved

# add rax, qword ptr [rsp + 8]
>>> state.regs.rax += state.mem[state.regs.rsp + 8].uint64_t.resolved
```

#### 基本执行

仿真管理器的具体内容在下一节，这节简单介绍基本方法

基本的单步执行可以用 **state.step()** 完成，该方法返回一个**SimSuccessors**类

这里我们关心的是 **.successor** 属性，该属性会返回该状态的所有successor列表。为什么是一个列表？事实上当符号执行遇到分支语句时（如if(x>4)），将分出两个state，并加入两个不同的约束式，一个是 `<Bool x_32_1 > 4>` ，一个是 `<Bool x_32_1 <= 4>` ，分别代表分支跳转和不跳转的状态 。

下面举一个例子，这是一个模拟的简单恶意软件，源码如下

```c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

char *sneaky = "SOSNEAKY";

int authenticate(char *username, char *password)
{
    char stored_pw[9];
    stored_pw[8] = 0;
    int pwfile;

    // evil back d00r
    if (strcmp(password, sneaky) == 0) return 1;

    pwfile = open(username, O_RDONLY);
    read(pwfile, stored_pw, 8);

    if (strcmp(password, stored_pw) == 0) return 1;
    return 0;

}

int accepted()
{
    printf("Welcome to the admin console, trusted user!\n");
}

int rejected()
{
    printf("Go away!");
    exit(1);
}

int main(int argc, char **argv)
{
    char username[9];
    char password[9];
    int authed;

    username[8] = 0;
    password[8] = 0;

    printf("Username: \n");
    read(0, username, 8);
    read(0, &authed, 1);
    printf("Password: \n");
    read(0, password, 8);
    read(0, &authed, 1);

    authed = authenticate(username, password);
    if (authed) accepted();
    else rejected();
}
```

显然SOSNEAKY是个后门账号。

下面的代码执行到当代码存在分支时，即判断是否为后门账号的地方。

```python
proj = angr.Project('fake_firmware')
state = proj.factory.entry_state(stdin=angr.SimFile)  # ignore that argument for now - we're disabling a more complicated default setup for the sake of education
while True:
    succ = state.step()
    if len(succ.successors) == 2:
        break
    state = succ.successors[0]

state1,state2 =  succ.successors
print state1,state2
```

输出：<SimState @ 0x400867> <SimState @ 0x40086e>

可以看到上面的输出就是经过分支后的两种不同状态。

注意这里的strcmp是一个angr hook并进行符号模拟的函数，因为函数的可能分支数非常多。

默认情况下，对于标准输入，angr会将其当做一个无限字节流的符号变量。

可以使用 **state.posix.stdin.load(0,state.posix.stdin.size)** 获取某个状态对输入变量的**引用**，如

```python
>>> input_data = state1.posix.stdin.load(0, state1.posix.stdin.size)

>>> state1.solver.eval(input_data, cast_to=bytes)
b'\x00\x00\x00\x00\x00\x00\x00\x00\x00SOSNEAKY\x00\x00\x00'

>>> state2.solver.eval(input_data, cast_to=bytes)
b'\x00\x00\x00\x00\x00\x00\x00\x00\x00S\x00\x80N\x00\x00 \x00\x00\x00\x00'
```

可以看到，走第一条分支的密码SOSNEAKY已被求出，代表了走向该路径可能的一种解

#### 预设的State

* `.blank_state()`  大多数的数据没有被初始化，当访问未初始化数据时会返回一个无约束的符号量

* `.entry_state()`  构造一个到达程序入口点，准备执行的状态

* `.full_init_state()`  构造一个到达程序入口点，准备执行的状态，且执行了所有的初始化器（如共享库构造器）

* `.call_state()`  构造一个准备执行某个指定函数的状态

也可以自己通过这些函数来指定state

* 所有的构造器都可以接受一个addr参数来指定从哪个地址开始

* `entry_state`和`full_init_state`
  
  * 可以使用args和env参数来指定程序启动的参数，该参数可以是一个字符串，或是一个bitvector（表示其作为符号）
  
  * 若希望argc是符号化的，可以传入一个符号量作为 `argc` 的参数，注意如果这样做了，应该添加一个约束式，令argc小于等于args中参数的个数

* `call_state(addr, arg1, arg2, ...)`
  
  * addr 指定函数地址，argN指定函数的参数。若希望传入的参数是一个实际指向一块分配了内存的指针，则应该使用PointerWrapper传入
    
    ```python
    angr.PointerWrapper("point to me!")
    ```
  
  * 关于调用约定，可以通过传入[SimCC](https://api.angr.io/angr.html#module-angr.calling_conventions)对象作为`cc`的参数来实现

此外对于上述的构造器，也有其他的选项，见 [project.factory](#https://api.angr.io/angr#angr.factory.AngrObjectFactory)

#### 内存交互的底层接口

除了使用**state.mem**外，可以用更底层的接口**state.memory**，并使用**store**和**load**方法交互

```python
>>> s = proj.factory.blank_state()
>>> s.memory.store(0x4000, s.solver.BVV(0x0123456789abcdef0123456789abcdef, 128))
>>> s.memory.load(0x4004, 6) # load-size is in bytes
<BV48 0x89abcdef0123>
```

默认以大端序传入，可以使用**endness**属性指定，属性值在archinfo的Endness枚举类型中定义

可以使用**arch.memory_endness**或**state.arch.memory_endness**获取当前字节序

```python
>>> import archinfo
>>> s.memory.load(0x4000, 4, endness=archinfo.Endness.LE)
<BV32 0x67452301>
```

对于寄存器对象，也有一个底层的接口 `state.registers` 可以存取，并且与 `state.memory` 提供的访问接口类似，但因为涉及IR，这里不深入

#### State属性

**state.options**拥有很多属性，其以angr.options开头，具体可在附录中找到

可以使用state.options.add或state.options.remove指定

```python
# Example: enable lazy solves, an option that causes state satisfiability to be checked as infrequently as possible.
# This change to the settings will be propagated to all successor states created from this state after this line.
# 这种模式会使之后所有创建的state都继承该属性
s.options.add(angr.options.LAZY_SOLVES)

# Create a new state with lazy solves enabled
s = proj.factory.entry_state(add_options={angr.options.LAZY_SOLVES})

# Create a new state without simplification options enabled
s = proj.factory.entry_state(remove_options=angr.options.simplification)
```

#### State插件

如上所示，SimState类有很多的方法，如memory,registers,mem,regs,solver等，这些都是作为插件提供的方法。

比如，memory插件模拟了一个平坦化的内存空间，同时也可以选择“抽象化内存”插件，该插件模拟一段地址无关的自由内存（具体用来干什么的我之后看看）。两个插件都使用state.memory对访问提供接口。

此外，state.memory和state.registers是同一个插件的两个不同接口，因为寄存器也是在内存中被模拟的。

##### globals插件

**state.globals**提供了一个访问state的\_\_dict\_\_属性的接口

##### history插件

state.history存放了程序运行路径中的一些数据。且该属性是个链表结构，可以通过**state.history.parent**，回溯上一次执行的history结构。

###### 迭代器

```python
state.history.bbl_addrs
state.history.recent_bbl_addrs            #上一次执行所经过的所有基本块地址
state.history.parent.recent_bbl_addrs    #上上次执行的所有基本块地址

for addr in state.history.bbl_addrs:    #打印所有trace基本块的地址
    print hex(addr)
```

###### history保存的数据

```python
history.descriptions    #每轮执行到该状态时的描述（存疑
history.bbl_addrs        #基本块地址列表
history.jumpkinds        #每次控制流转移执行的处理方式列表，作为VEX枚举字符串给出
history.guards            #记录每次转移条件的列表
history.events            #记录了程序的一些事件，如弹出msgbox，或符号跳转条件等
history.actions            #一般为空，但若指定了angr.options.refs，将返回一个记录
                        #内存、寄存器等信息的log
```

##### callstack插件

类似history，是个链表，但不提供迭代器方法，可以使用**state.callstack**作为迭代器

```python
callstack.func_addr            #当前执行函数的地址
callstack.call_site_addr    #调用该函数的基本块地址
callstack.stack_ptr            #esp-ebp
callstack.ret_addr            #返回地址
```

#### 更多关于IO的信息：文件、文件系统、socket

[看这里](<https://github.com/angr/angr-doc/blob/71e6143503da2cf585c2203ab0cab66020413f92/docs/file_system.md>)

#### 复制与合并

一个状态可以被快速复制，如下

```python
s = proj.factory.blank_state()
s1 = s.copy()            #对初始状态s复制了两份副本
s2 = s.copy()

s1.mem[0x1000].uint32_t = 0x41414141
s2.mem[0x1000].uint32_t = 0x42424242
```

也可以被合并

```python
# merge will return a tuple. the first element is the merged state
# the second element is a symbolic variable describing a state flag
# the third element is a boolean describing whether any merging was done
#返回一个tuple，第一个元素为merge后的state，第二个为描述state的符号变量，第三个为bool量，表示是否merge成功
(s_merged, m, anything_merged) = s1.merge(s2)

# this is now an expression that can resolve to "AAAA" *or* "BBBB"
aaaa_or_bbbb = s_merged.mem[0x1000].uint32_t
# 这里合并之后，0x1000处的内存可能有两种值，AAAA或BBBB
```

#### 本节介绍的模块与方法

##### SimState

```python
state.mem[].type    #取内存地址，type指定了数据类型，如uint64_t
state.mem[].type.resolved    #取内存值
state.regs.xxx        #访问寄存器

state.step()

state.posix.stdin.load(0,state.posix.stdin.size)    #获取对标准输入的引用

state.mem
state.memory.store
state.memory.load

state.options        #指定state的属性，具体属性列表见附录

state.globals

state.history
```

##### 初始的执行状态

```python
.entry_state()        #构造一个准备执行main函数的状态
.blank_state()        #构造一个未初始化的状态，所有对未初始化地址的访问被视作一个符号变量
.full_init_state()    #构造一个执行程序初始化工作前的状态，即链接器等初始化工作还没执行时的状态
.call_state()        #构造一个准备执行给定函数的状态
    .call_state(addr,arg1,arg2...)
                    #如果需要使用指针传入一块内存，得以如下方法给出arg参数
            angr.PointerWrapper("point to me!")
                    #可以通过cc参数传入SimCC类指定调用约定
```

参数：

```python
addr: 所有函数都可指定
args,env: entry_state和full_init_state可指定
argc: 如果希望argc是个符号变量可用该参数传入
```

##### SimSuccessors

SimState执行step返回一个SimSuccessors对象，若当前语句没有分支则**successors**属性只有一个值，否则返回一个指向两个分支状态的列表，且列表项为**SimState**对象

```python
state = proj.factory.entry_state(stdin=angr.SimFile)
while True:
    succ = state.step()
    if len(succ.successors) == 2:
        break
    state = succ.successors[0]
state1,state2 =  succ.successors
```

### Simulation Managers 模拟管理器

模拟管理器用于控制符号执行时同时产生的一系列状态，提供了一些搜索策略来探索程序的状态空间。其中状态被组织为 stashes ，可以对该对象应用单步执行、筛选、合并、移动等操作

最常见的stash类型是 `active` ，包含了大多数操作，初始化一个模拟管理器后的state默认存放的stash类型即为active

#### 步进

模拟管理器最基本的操作就是通过 `.step()` 遍历一个bbl中给定stash的所有状态

```python
>>> import angr
>>> proj = angr.Project('examples/fauxware/fauxware', auto_load_libs=False)
>>> state = proj.factory.entry_state()
>>> simgr = proj.factory.simgr(state)
>>> simgr.active
[<SimState @ 0x400580>]

>>> simgr.step()
>>> simgr.active
[<SimState @ 0x400540>]
```

stash模型真正的能力是当遇到了符号化的条件分支，两种可能的后续状态都会出现在stash中，而我们可以同时对它们采取步进操作

此外，可以直接使用 `.run()` 使得程序运行到没有能够继续步进执行的地方

```python
# Step until the first symbolic branch
>>> while len(simgr.active) == 1:
...    simgr.step()

>>> simgr
<SimulationManager with 2 active>
>>> simgr.active
[<SimState @ 0x400692>, <SimState @ 0x400699>]

# Step until everything terminates
>>> simgr.run()
>>> simgr
<SimulationManager with 3 deadended>
```

这里到达了三个deadend状态。当一个状态无法产生任何后续状态时，比如运行到exit系统调用，则模拟管理器会将其从active stash移动到deadend stash

#### Stash管理

一个模拟管理器中包含了多个stash，可以对这几个stash进行操作

* 在stash间移动state
  
  ```python
  >>> simgr.move(from_stash='deadended', to_stash='authenticated', filter_func=lambda s: b'Welcome' in s.posix.dumps(1))
  >>> simgr
  <SimulationManager with 2 authenticated, 1 deadended>
  ```
  
  这里创建了一个新的stash，名为authenticated，并将deadended stash中所有在stdout中含有“Welcome”的state（由filter_func指定）移动到该stash

* 遍历stash
  
  每个stash都是个列表，所以可以直接遍历。也可以用一些方法来遍历，主要分为两类
  
  * `one_<name>` 返回stash中对应的第一个state
  
  * `mp_<name>` 返回stash的[mulpyplexed](https://github.com/zardus/mulpyplexer)对象
  
  ```python
  >>> for s in simgr.deadended + simgr.authenticated:
  ...     print(hex(s.addr))
  0x1000030
  0x1000078
  0x1000078
  
  >>> simgr.one_deadended
  <SimState @ 0x1000030>
  >>> simgr.mp_authenticated
  MP([<SimState @ 0x1000078>, <SimState @ 0x1000078>])
  >>> simgr.mp_authenticated.posix.dumps(0)
  MP(['\x00\x00\x00\x00\x00\x00\x00\x00\x00SOSNEAKY\x00',
      '\x00\x00\x00\x00\x00\x00\x00\x00\x00S\x80\x80\x80\x80@\x80@\x00'])
  ```

* 其他方法，如step run等都可以接受一个参数 `stash` ，指定执行操作的对象

#### Stash类型

| Stash类型       | 描述                                                                                                                        |
| ------------- | ------------------------------------------------------------------------------------------------------------------------- |
| active        | 默认包含了哪些将要执行步进操作的状态                                                                                                        |
| deadended     | 当一个state无法执行下去时会保存在该stash，包括遇到无效指令、无效程序指针、后续状态不可达等                                                                        |
| pruned        | 若指定了 LAZY_SOLVES 参数，则程序除非在必要时不会检查是否满足约束。当发现当前state为unsat时，会回溯history以确定具体转变为unsat的地方，并将之后的所有状态放到该stash中（显然这些状态也必然是unsat的） |
| unconstrained | 若指定了 save_unconstrained 参数，则将那些被判定为unconstrained的状态保存在该stash（如程序指针被用户数据或其他符号值控制）                                          |
| unsat         | 若指定了 save_unsat 参数，则将那些unsat的状态放在该stash                                                                                   |

还有不是stash，但是也是state列表的对象errored 。若执行时遇到错误，则会被包装为 ErrorRecord 对象，并加入errored列表。可以使用`.error`属性查看其错误类型，`.state`查看对应的state。此外可以使用`.debug()`方法来调试

#### 简单的路径探索

符号执行中最基础的操作就是找到到达某个地址的state，而忽略其他的state。可以使用`.explore()`方法。

##### find参数

用于指定路径搜索的结束条件，可以是

* 一个地址，当到达地址对应的指令则停止执行

* 一个地址列表，与上面类似

* 一个回调函数，传入的参数是当前的state，而函数返回是否停止执行

当一个state满足find指定的条件，则会被放入名为`found`的stash

##### avoid参数

格式与上述类似，当state满足条件时会被放入名为`avoided`的stash

##### num_find参数

指定在explore返回前应该找到几个满足条件的state，默认为1

#### 路径探索算法

当angr搜索程序的状态空间时，默认的方法是类似BFS，但提供了很多方法可以调用，如DFS。甚至可以自定义搜索算法

使用 `simgr.use_technique(tech)` 指定算法

下面是几种内建算法

* DFS  深度优先搜索，同一时刻只有一个状态被激活并运行，剩下的状态被放在名为`deferred`的stash，直到激活的状态变为deadend才会遍历下一个

* Explorer  就是`.explore()`调用的方法，允许指定find和avoid参数

* LengthLimiter  限定某个状态经过的程序路径长度

* LoopSeer  若某个循环运行了太多次，会自动将它产生的状态放入名为`spinning`的stash，直到其他状态遍历完之后再回来遍历这些由循环导致的路径爆炸产生的状态

* ManualMergepoint  标记一个地址为merge point，当一个状态到达这个地址时会被保存，而若其他状态在timeout时间内也到达该地址，则这些状态会被merge

* MemoryWatcher  监控内存占用，若运行太慢则会中断explore

* Oppologist  "operation apologist"，这个功能主要针对angr不支持的指令。当遇到不支持指令时，angr会将所有输入实例化，并调用unicorn来模拟执行这些输入作用于该指令的结果。从而使得符号执行不会被类似指令影响

* Spiller  若产生了太多状态，这项技术可以将一些状态暂存于磁盘

* Threading  对步进过程进行线程化，这项技术大多数时候不会加速太多，因为python解释器本身有全局锁。但若速度的瓶颈在于一些native代码（如unicorn z3 libvex等），则可以用该技术加速

* Tracer  该技术可以让符号执行沿着某个trace文件的路径运行

* Veritesting  是[这篇论文](https://users.ece.cmu.edu/~dbrumley/pdf/Avgerinos%20et%20al._2014_Enhancing%20Symbolic%20Execution%20with%20Veritesting.pdf)所描述的技术的实现，用于自动识别一些可用的merge point，从而加速符号执行

#### 本节介绍的模块与方法

TODO

### Execution Engines  执行引擎

当我们进行步进操作时，angr使用一系列的引擎来模拟执行，下面为默认的执行引擎列表，angr的执行处理顺序如下

* 当执行使程序进入无法继续执行的状态，调用failure engine

* 当执行调用了syscall，则使用syscall engine处理

* 若当前地址为hook地址，调用hook engine

* 若当前没有符号化的数据，且`UNICORN`选项开启，则调用unicorn engine

* 其他情况下，调用VEX engine

##### SimSuccessors

是`.step()`返回的类，该类目的是对successor的属性进行简单分类，这些属性有：

| 属性         | 条件  | 指令指针 | 描述  |
| ---------- | --- | ---- | --- |
| successors |     |      |     |
|            |     |      |     |
|            |     |      |     |
|            |     |      |     |



## 附录

### State属性目录

#### State模式

通过给构造器传入mode=xxx

* symbolic  默认
* symbolic_approximating  允许使用近似模式解决约束
* static  对于静态分析很有用，内存模型变为一个抽象的区块映射系统，有许多特性
* fastpath  对于轻量化静态分析很有用，跳过所有耗时的处理，并分析程序的大体行为
* tracing  使用给定输入运行程序，使用unicorn和弹性选择，并尝试正确模拟访问冲突（有点不懂意思）

#### 属性集合

使用angr.options.xxx指定
