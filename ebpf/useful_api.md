## 映射

### 建立预定义映射

建立一个类型为LRU_HASH，key类型为flow_t，value类型为network_flow_counter_t的映射

```c
struct bpf_map_def SEC("maps/network_flows") network_flows = {
    .type = BPF_MAP_TYPE_LRU_HASH,
    .key_size = sizeof(struct flow_t),
    .value_size = sizeof(struct network_flow_counter_t),
    .max_entries = MAX_FLOW_COUNT,
    .pinning = 0,
    .namespace = "",
};
```

### 增删改查

#### 增改

```c
bpf_map_update_elem(&network_flows, flow, &new_counter, BPF_ANY);
```

#### 删

```c
bpf_map_delete_elem(&network_flows, prev_flow);
```

#### 查

```c
bpf_map_lookup_elem(&network_flows, flow);
```

在映射network_flows中查找key为flow的元素，若为NULL则不在表中

## libbpf

使用libbpf开发ebpf应用，主要流程如下

* 与正常开发ebpf程序一样，此外libbpf支持定义全局变量与user program进行通信

* 编译时
  
  * 会先编译kernel端的ebpf程序
  
  * 编译完成后调用 `bpftools gen skeleton` 生成骨架，所谓骨架实际上是一个头文件，该头文件包含了ebpf kernel端程序的各种信息（如映射、程序本身的内存映射等），包括程序本身会被embeded到该头文件中，全局变量会作为一个映射被加入到骨架中。此外还会生成诸如加载与释放该ebpf程序的函数
  
  * 最后在user程序中应包含该头文件，即可以直接调用各个生成的函数（如open attach destroy）与ebpf程序进行交互
