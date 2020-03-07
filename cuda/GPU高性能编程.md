### 设备属性及管理

* cudaGetDeviceCount   P30
* cudaGetDeviceProperties   P30
* cudaChooseDevice   P34
* cudaGetDevice   P34
* cudaSetDevice   P34



### 同步

host与device同步

* cudaDeviceSynchronize
* cudaMemcpy   存在一个隐式的synchronize



### 事件

用于获取时间戳

* cudaEventCreate
* cudaEventRecord
* cudaEventElapsedTime
* cudaEventSynchronsize
* cudaEventDestroy



### 内存管理

* cudaMalloc
* cudaFree
* cudaMemcpy  最后一个参数有cudaMemcpyDeviceToHost / cudaMemcpyHostToDevice
* cudaMemset
* cudaMemcpyToSymbol



### 错误获取

* checkCudaErrors
* cudaGetErrorString



### 内置变量

* threadIdx
* blockIdx
* blockDim
* gridDim



### 函数类型

* `__global__`  可以从host调用，若版本号大于3可以从device端调用
* `__device__`  只能从device端调用
* `__host__`      可以忽略

**一个函数可以同时拥有`__host__`和`__device__`修饰符**

#### 限制

##### global修饰的kernel

* **返回值必须为void**
* 只能访问device的内存
* 不支持可变参数
* 不支持静态变量
* 不支持函数指针



### 层次结构

#### 线程(thread)与线程块(block)

线程块由**三维的**线程索引组成

线程的数量限制： maxThreadsPerBlock（一般是65535）

#### 线程块与线程格(grid)

线程格由**二维的**线程块索引组成

#### 例子

若一个kernel启动时，参数为

```c
dim3 blocks(4, 4);
dim3 threads(16, 16);
kernel<<<blocks, threads>>>();
```

则一个线程块包含256个线程，一个线程格包含4*4个线程块

则一个线程在整个线程格中的位置为

```
x = threadIdx.x + blockIdx.x * blockDim.x;
y = threadIdx.y + blockIdx.y * blockDim.y;
```

正确的相对偏移为

```
offset = x + y * blockDim.x * gridDim.x;
```

因为`gridDim.x`指定了x轴上有几个block，`blockDim.x`指定了一个block中有几个thread

![](pic\cuda_1.png)

### 内存

#### shared memory

```c
__shared__ int cache[threadperblock*n];
```

shared memory是**在一个block中共享的**，即对于一个block中的各个thread，访问的是同一块shared memory

而对于不同block，每个block都会生成一个相同大小的副本，即不同block间的shared memory不能互相访问

#### constant memory

kernel中的声明如下：

```c
__constant__ int a[length];
```

host中赋值常量内存操作如下

```c
checkCudaErrors( cudaMemcpyToSymbol(a, tmp, length * sizeof(int)) );
```

**注意**：constant memory的优势在于，当warp中的thread要访问常量内存时，该访问会在一次执行时被广播到半个warp（16个thread），即若16个thread访问的都是**同一个位置**的constant memory，则只需要一次操作。

但是，若16个thread访问了n个不同位置，则gpu会将n次访问串行化执行，其效率还不如global memory

#### texture memory



#### __syncthreads

**同一线程块中各个线程的fence**

遇到`__syncthreads()`语句时，确保同一block中所有的thread都执行到这个语句了才会进行下面的语句

**注意**：这里的同步作用也包括那些在if语句中的`__syncthreads()`语句，即，若`__syncthreads()`在条件语句中，若某个thread没有执行该分支，GPU仍会继续执行代码，直到执行了`__syncthreads()`，因此对于`__syncthreads()`一定不能写在某些thread永远无法执行的分支中，否则会造成死锁。（参考点积例子）

### 事件

可以通过时间记录时间戳分析性能

```c
cudaEvent_t start, stop;
cudaEventCreate(&start);
cudaEventCreate(&stop);
cudaEventRecord(start, 0);
...;						//GPU running
cudaEventRecord(stop, 0);
cudaEventSynchronsize(stop);

float time;
cudaEventElapsedTime(&time, start, stop);

cudaEventDestroy(start);
cudaEventDestroy(stop);
```



### add

#### 单线程块多线程

##### 一维线程

```c
__global__ void vecadd(float *a, float *b, float *c)
{
    int tid = threadIdx.x;
    c[tid] = a[tid] + b[tid];
}

int main()
{
    ...
    vacadd<<<1, N>>>(A, B, C);
    ...
}
```

##### 多维线程

```c
__global__ void vecadd(float *a, float *b, float *c)
{
    int tid = threadIdx.x * blockDim.x + threadIdx.y;
    c[tid] = a[tid] + b[tid];
}

int main()
{
    ...
    int block_num = 1;
    dim3 threadsPerBlock(X, Y);
    vecadd<<<block_num, threadsPerBlock>>>(A, B, C);
    ...
}
```

或使用二维数组索引

```CQL
__global__ void vecadd(float a[X][Y], float b[X][Y], float c[X][Y])
{
    int i = threadIdx.x;
    int j = threadIdx.y;
    c[i][j] = a[i][j] + b[i][j];
}

int main()
{
    ...
    int block_num = 1;
    dim3 threadsPerBlock(X, Y);
    vecadd<<<block_num, threadsPerBlock>>>(A, B, C);
    ...
}
```

#### 多线程块单线程

```c
__global__ void vecadd(float *a, float *b, float *c)
{
    int tid = blockIdx.x;
    c[tid] = a[tid] + b[tid];
}

int main()
{
    ...
    vecadd<<<N, 1>>>(A, B, C);
    ...
}
```

#### 多线程块多线程

```c
__global__ void vecadd(float *a, float *b, float *c)
{
    int tid = threadIdx.x + blockDim.x * blockIdx.x;
    c[tid] = a[tid] + b[tid];
}

int main()
{
    ...
    int block_num = M;
    int thread_num = (N + blcok_num -1) / M;
    vecadd<<<block_num, thread_num>>>(A, B, C);
    ...
}
```

### 点积

```c
__global__ void dot(int *a, int *b, int *c)
{
    __shared__ int cache[thread_per_block];
    int i;
    int tmp = 0;
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    
    for(index; index < N; index += blockDim.x * gridDim.x)
    {
        tmp += a[index] * b[index];
    }
    cache[threadIdx.x] = tmp;
    
    __syncthreads();
    
    i = blockDim.x / 2;
    while(i > 0)
    {
        if(threadIdx.x < i)
            cache[threadIdx.x] += cache[threaadIdx.x + i];
        __syncthreads();
        i /= 2;
    }
    
    c[blockIdx.x] = cache[0];
}
```

因为点积的输入是N位向量a和b，输出是一个和值，因此这里用到的思想为：以块为单位，计算该块中所有对应元素的值，即设一共有n个block，每个block有m个thread

* 按照线程分配的原则，第x个block的第y个thread应该计算`a[x*m+y], a[x*m+y + mn], a[x*m+y + 2mn], ...`的与`b[x*m+y], b[x*m+y + mn], b[x*m+y + 2mn], ...`的点积，并存到cache[y]中
* 上述运行完后，每个block的cache变量中存放着对应线程处理的元素的点积，接下来应该求其之和
* 求和方法为：1）前m/2个thread运行，计算`cache[i]+cache[i+m/2]`  2） 存放在`cache[i]`  3） m = m/2循环
* 最后一个block的处理结果放在了cache[0]里，再将其赋值给c[blockIdx.x]返回到cpu
* cpu执行最后的n次求和

这里运用的求和思想就是归约

#### 注意syncthreads的使用

这里的归约过程中切不可把下述代码

```c
    while(i > 0)
    {
        if(threadIdx.x < i)
            cache[threadIdx.x] += cache[threaadIdx.x + i];
        __syncthreads();
        i /= 2;
    }
```

改为

```c
    while(i > 0)
    {
        if(threadIdx.x < i)
        {
            cache[threadIdx.x] += cache[threaadIdx.x + i];
        	__syncthreads();
        }
        i /= 2;
    }
```

表面看似乎减少了执行__syncthreads()的次数，但实际上因为该函数用于同步，因此所有线程在执行到该函数前都会等待，则最后会造成死锁

## ref

https://www.cnblogs.com/1024incn/p/4541313.html  墙裂推荐 是一个系列的文章