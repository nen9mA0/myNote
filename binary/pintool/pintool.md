这里记录一些文档（pin文件夹下的doc/index.html）的内容，论文移步paper文件夹

#### Pin中的trace和basic block

##### trace

Pin中的trace一般是以当前指令序列的一个无条件跳转为结尾的，包含call和ret。pin保证一个trace只有一个入口点，但可能有多个出口。如果一个其他位置的指令跳转到某个trace的中间，将会建立新的trace。

pin保证每个trace结束后pin都可以拿到控制权

##### basic block

trace再被细分为basic block。一般来说我们插桩的代码建议以basic block为单位插桩以提高性能

pin的basic block与传统basic block不同。pin的basic block是有runtime特性的，比如文档里举的例子

```c
switch(i)
{
    case 4: total++;
    case 3: total++;
    case 2: total++;
    case 1: total++;
    case 0:
    default: break;
}
```

编译后

```asm
.L7:
        addl    $1, -4(%ebp)
.L6:
        addl    $1, -4(%ebp)
.L5:
        addl    $1, -4(%ebp)
.L4:
        addl    $1, -4(%ebp)
```

按传统basic block的定义，每条addl都是一个basic block。但对于pin来说，若第一次跳转到L7，则下面的4条指令视为一个basic block。只有在某次跳转到其他标号时才会改变，如第n次运行时跳转到L5，则会将原来的basic block切分为两个basic block



此外如rep cpuid popf等指令可能有一些特殊处理。如对于rep前缀的指令带有循环的性质，原来rep处于其他的basic block里，但在循环次数多于1时，rep指令会单独生成一个basic block

#### 不同粒度的分析

* trace  TRACE_AddInstrumentFunction 

* instruction  INS_AddInstrumentFunction

* image  IMG_AddInstrumentFunction

  image分析依赖于符号信息来确定分析的边界，因此在`PIN_Init`前应先调用`PIN_InitSymbols`

* routine  RTN_AddInstrumentFunction

* thread  PIN_AddThreadStartFunction PIN_AddThreadFiniFunction 

#### SMC

对于self-modifying code，需要让平台支持`Jit Profiling API`

#### 多线程

因为如果直接使用平台提供的API来处理多线程情况会违背Pin API与应用程序隔离的准则，导致一些意想不到的问题，所以pin自己提供了一套多线程的库

在linux上提供的C/C++ runtime有些函数不是线程安全的，比如errno，对于这种情况得上锁

#### 锁

pin默认采用下面的申请锁的策略

* 在acquire其他线程的锁前先acquire当前线程的内部锁

  ```
  Application locks -> Pin internal locks -> Tool locks
  ```

对应页面有一个pin中锁编程的注意事项

* 在pin的callback中如果acquire了，在返回前必须release
* 如果在分析的routine上acquire了，返回前必须release
* 如果在callback或者routine调用了PinAPI，调用前必须release
* 在routine调用PinAPI前可能需要调用PIN_LockClient，在这之前必须release

