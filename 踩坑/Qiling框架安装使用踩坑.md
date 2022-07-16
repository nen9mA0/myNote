qiling框架是一款Binary Emulation Framework，它的底层通过unicorn来模拟不同架构上的代码执行，但与unicorn不同，后者只是一个不同架构CPU的emulator，而qiling框架基于此为不同架构的软件提供了一套比较完整的执行环境。

### 安装

直接使用pip安装

```python
pip install qiling
```

### IDA插件

#### 插件安装

使用pip安装后，插件位于

```
%PYTHONHOME%\Lib\site-packages\qiling\extensions\idaplugin
```

直接将qilingida.py扔到

```
%IDA_PATH%\plugins
```

#### rootfs安装

由上所述，qiling框架旨在为模拟各个平台下的程序运行，所以对于不同的架构和系统都准备了一套rootfs。下载地址为

[GitHub - qilingframework/rootfs](https://github.com/qilingframework/rootfs)

#### 坑点

- python 3.7.9不能用，会报错，3.8.0也报错，测试3.8.6可以用。若要为IDA替换python解释器可以使用 `%IDA_PATH%\idapyswitch.exe`替换
- windows的rootfs是不完整的，[原因见此](https://docs.qiling.io/en/latest/install/)。所以得到 [这里](https://github.com/qilingframework/qiling/tree/master/examples/scripts) 下载一个脚本来补全文件
- 如果在start的时候报了个找不到可执行文件的错，记得回想一下自己是不是在IDA建立了idb后给可执行文件挪过位置
- 挺离谱的，pip下下来的代码里有bug，见 [bug fix](#buf fix)

#### bug fix

- 若没有使用qiling框架，退出IDA时会报错 （ref: [修复ida插件在ida pro 7.7上异常报错 by BRAlNDead · Pull Request #1165 · qilingframework/qiling · GitHub](https://github.com/qilingframework/qiling/pull/1165)）

```
qinglingida.py: line 881

-     self.ql: Qiling
+     self.ql: Qiling = None
```

- 无法正常打开reg窗口，第509行arch没有定义

```
qinglingida.py: line 509

+             arch = ql.arch.type
```


