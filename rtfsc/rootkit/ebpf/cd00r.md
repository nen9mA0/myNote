该rootkit是早期通过bpf隐藏网络通信的rootkit，网上展示的代码主要是通过构造pcap的程序（pcap程序会被编译为bpf程序），抓取网络包，并以此来触发一个反弹shell

#### pcap

这里构造的pcap命令如下

```
port <port1>[ or port <port2> ...]
```

其中，port1 port2 ... 是提前定义的需要抓取的port列表

#### 后门

##### 后门触发

这里首先设置了pcap程序，并将其绑定到网卡设备，此后fork一个子进程（主进程随后退出）

子进程进入一个死循环获取pcap抓取的包，对抓取的包进行如下判断

* 是否为IP包

* 是否为IPv4

* 是否为TCP包

* 若不是SYN包则略过
  
  * 若是SYN-ACK包则略过

* 目的端口是否为触发端口

若上述的检测通过，则反弹一个shell

##### 反弹shell

这里反弹shell的方式是fork两次（防止僵尸进程），原理见[c - Why fork() twice - Stack Overflow](https://stackoverflow.com/questions/10932592/why-fork-twice)

* fork
  
  * 主进程wait
  
  * 子进程fork
    
    * 主进程exit
    
    * 子进程执行命令

反弹shell的方式是写入一个inetd的规则文件，这里写的是

```
5002  stream  tcp     nowait  root    /bin/sh  sh
```

之后调用

```
/usr/sbin/inetd /tmp/.ind
```




