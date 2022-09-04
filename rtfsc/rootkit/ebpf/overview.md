# 功能

## 网络相关

### ebpfkit

#### 网络

##### 端口扫描

相关逻辑在 arp.h 中

采用xdp的方式劫持了arp流量，并查找arp包的源地址是否为正在扫描的IP，若是则说明目标存在，则记录结果并将该包drop，从而隐藏端口扫描的通信

##### DNS劫持

这里实现了两种DNS劫持，一种使用XDP，一种使用TC

###### XDP

相关逻辑在xdp.h和dns.h中

采用xdp劫持了dns流量（通过判断协议是否为UDP，端口是否为53），若是则调用对应的bpf程序来劫持dns返回结果。这里调用的是xdp_ingress_handle_dns_resp，具体内容在dns.h。

劫持的过程如下

* 根据dns返回的ip和端口查询是否为需要劫持的ip

* 若是，则解析DNS包并将返回的ip替换为劫持后的ip，这里对DNS包的处理比较完善，可以参考

###### TC

相关逻辑在tc.h和dns.h中

# 技术点

## bpf程序间的调用

首先需要创建一个BPF_MAP_TYPE_PROG_ARRAY类型的映射，该映射保存的是bpf程序，此后可以通过bpf_tail_call来调用对应的程序
