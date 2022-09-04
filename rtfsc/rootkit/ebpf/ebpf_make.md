# eBPF编译与配置

## 内核makefile编译选项

### kbuild相关

https://opensource.com/article/18/10/kbuild-and-kconfig

### defconfig

```
make defconfig KBUILD_DEFCONFIG=xxx
```

该命令用于配置内核选项

[kbuild - What exactly does Linux kernel&#39;s `make defconfig` do? - Stack Overflow](https://stackoverflow.com/questions/41885015/what-exactly-does-linux-kernels-make-defconfig-do)

大概意思就是，make defconfig可以加入一个变量 `KBUILD_DEFCONFIG=xxx` ，该文件用于指定一些与内核默认配置不一样的选项。当运行make defconfig，会遍历内核目录下的子文件夹，比较各个子文件夹下的kconfig选项，若遍历到的选项在KBUILD_DEFCONFIG中被指定，则将新的选项写入 `.config` ，否则写入的是kconfig中的默认选项

所以若没有指定KBUILD_DEFCONFIG，则使用默认选项

### oldconfig

```
make oldconfig
```

该选项会根据当前正在运行的linux所指定的选项创建 `.config` 文件

### prepare

```
make prepare
```

生成一些架构相关的头文件和并执行一些脚本，包括生成一系列汇编相关的头文件

## eBPF编译方法

### 编译示例程序

https://www.kernel.org/doc/readme/samples-bpf-README.rst

此外我自己实践过程中还需要一些别的编译选项，最终编译成功的指令如下：

```
make defconfig
make headers_install
make prepare
make M=samples/bpf
```

此时可以成功编译samples/bpf下的示例程序

也可以通过

```
make M=samples/bpf clean
```

清理编译后的程序

### 加入自己的程序

假设我的程序放在sample/bpf/helloworld文件夹内，分别为hello_load.c（user）和hello_kern.c（kernel），此外user程序需要链接sample/bpf目录下的bpf_load.c

```
sample/bpf
  |____ bpf_load.c
  |____ helloworld
            |_______ hello_load.c
            |_______ hello_kern.c
```

#### 加入目标程序

在makefile中的tprogs-y加入新的目标程序

```
tprogs-y += hello_load
```

#### 加入user程序.o

加入 `${目标程序名}-objs` 变量

```
hello_load-objs := bpf_load.o helloworld/hello_load.o
```

#### 加入bpf程序

在makefile中的always-y加入新的目标程序

```
always-y += helloworld/hello_kern.o
```

#### 编译

完成以上步骤即可编译，编译完成后文件夹布局如下

```
sample/bpf
  |____ bpf_load.c
  |____ bpf_load.o
  |____ hello_load   user程序
  |____ helloworld
            |_______ hello_load.c
            |_______ hello_load.o
            |_______ hello_kern.c
            |_______ hello_kern.o  bpf程序
```
