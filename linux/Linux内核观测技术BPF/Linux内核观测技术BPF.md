本书代码仓库

[GitHub - bpftools/linux-observability-with-bpf: Code snippets from the O&#39;Reilly book](https://github.com/bpftools/linux-observability-with-bpf)

## 第2章

### 运行第一个BPF程序

#### BPF模块

```c
#include <linux/bpf.h>
#define SEC(NAME) __attribute__((section(NAME), used))

SEC("tracepoint/syscalls/sys_enter_execve")
int bpf_prog(void *ctx)
{
    char msg[] = "Hello, BPF World!";
    bpf_trace_printk(msg, sizeof(msg));
    return 0;
}


char _license[] SEC("license") = "GPL";
```

一些要点

* SEC宏  使用该属性告诉BPF虚拟机何时运行此程序，这里的监测点是execve系统调用

* license这里必须指定一个license，且因为linux是GPL的，所以只接收GPL的程序

#### 加载程序

```c
#include <stdio.h>
#include <uapi/linux/bpf.h>
#include "bpf_load.h"


```
