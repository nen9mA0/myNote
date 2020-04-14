## makefile

感觉语法好像跟现在的不一样

```
all -> Image --- bootsect
              |- setup
              |- system
              |- build
```

通过**tools/build**整合整个image

```
tools/build --- tools/build.c
```

disk用来烧写image

```
disk: Image
	dd bs=8192 if=Image of=/dev/PS0
```

下面是Image的依赖

```
bootsect --- boot/bootsect.s
setup    --- boot/setup.s
```

```
system   --- boot/head.o init/main.o
```

head.o依赖于head.s，main.o则直接依赖于所有其他文件

## tools

### build.c

#### 检查

这里的检查实际上是分两次执行的，先打开bootsect后检查，然后写bootsect，再打开setup检查，然后写setup

依次打开`boot/bootsect boot/setup`，检查

* offset 0是否为0x04100301
* offset 8是否为MINIX_HEADER
* offset 24是否为0（data segment）
* offset 32是否为0（bss segment）
* offset 40是否为0（minix header）
* offset 56是否为0（symbol table）

#### 写bootsect

* 执行上述检查
* 读入512字节
* 检查offset 510是否为0xAA55
* 在offset 508写入`minor_root`，offset 509写入`major_root`（启动设备主次编号）
* 输出到stdout

#### 写setup

* 执行上述检查
* 循环读入sizeof(buf)（默认1024B），并输出到stdout
* 计算输出的大小，并补0直到输出长度为`SETUP_SECTS * 512`

#### 写system

* 读入GCC_HEADER长度的字节并检查offset 40是否为0
* 每次读入sizeof(buf)并输出到stdout