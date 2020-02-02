# 资料

https://bbs.pediy.com/thread-217588.htm  通过编译优化进行VMP代码还原

https://bbs.pediy.com/thread-192434.htm  用Oreans UnVirtualizer还原VM代码

https://bbs.pediy.com/thread-155215.htm  VMP早期版本逆向还原文档

https://bbs.pediy.com/thread-143377.htm  VMP分析插件v1.2

https://wenku.baidu.com/view/9f2c6f162f3f5727a5e9856a561252d380eb2025.html  VMProtect的逆向分析和静态还原

https://github.com/jnraber/VirtualDeobfuscator  VirtualDeobfuscator

https://github.com/anatolikalysch/VMAttack  VMAttack

# VMProtect的逆向分析和静态还原

## vmp handler

基于栈的虚拟机

```
add eax, ecx
    |
    |
 (vm_isa)
push ecx
push eax
add 
pop eax
    |
    |
 (real code)
mov eax, dword ptr [ebp]
add dword ptr [ebp+4], eax
pushfd
pop dword ptr [ebp]
```

## vmp指令分类

* 算术运算和移位运算
* 堆栈操作
* 内存操作
* 系统相关（无法模拟指令）
* 逻辑运算

### 逻辑运算

vmp中逻辑运算最为复杂，只有nor，其他由nor构造

```
nor(a,b) = ~(a|b) = ~a & ~b

not(a) = nor(a,a)
and(a,b) = nor( not(a), not(b) ) = nor( nor(a,a), nor(b,b) )
or(a,b) = not( nor(a,b) ) = nor( nor(a,b), nor(a,b) )
xor(a,b) = a&~b | ~a&b
         = ~( ~(a&~b) & ~(~a&b) )
         = ~(~a|b & a|~b)
         = ~(~a&~b | a&b)
         = nor( nor(nor(a,a),nor(b,b)), nor(a,b) )
```

## 寄存器轮转

vmp的虚拟寄存器映射位置不固定，每次执行一段指令就会改变一次映射，且仅在编译时可见，直接导致了分析难度增加

## 字节码加密和随机校验

vmp解码算法分布于dispatch过程和每个handler中，在取指取数时才会解密，且解密算法不同，seed也会变化

## 哈希校验

vmp会在字节码中插入哈希校验handler以保证不被篡改

## vmp静态跟踪

以虚拟执行代替实际的代码执行，有助于节省时间（虚拟执行在遇到分支时可以保存某个点的上下文，方便在执行完第一个分支时回退到分支前分析第二个分支）

## 字节码反编译

### 中间表示语言

堆栈操作转为SSA（静态单赋值）

```
push c
push b
add
pop efl
pop a
  |
  |
stk1= c
stk2 = b
stk3, stk4 = add stk1, stk2
efl = stk4
a = stk3
```

### 指令化简与优化

#### 常数收缩

```
push 1A2FBCA0
push F80499D8
add
pop efl
   |
   |
push 12345678
pop efl
```

#### 活跃变量分析

```
stk0 = ctx24
stk1 = ctx10
stk2, efl1 = add stk0, stk1
ctx28 = efl1
ctx30 = stk2
stk3 = ctx30
stk4 = 1111
stk5, efl2 = add stk3, stk4
ctx28 = efl2
ctx34 = stk5
     |
     |  去除中间变量
     |
ctx30, ctx28 = add ctx24, ctx10
ctx34, ctx28 = add ctx30, 1111
     |
     |  去除无用变量
     |
 ctx30 = add ctx24, ctx10
 ctx34, ctx28 = add ctx30, 1111
```

#### 删除垃圾指令

通过一定规则删除

### 转换汇编指令——树模式匹配

由上面的式子可以转化为树

```
     ctx34,ctx28 = add
       /           \
 ctx30 = add      1111
   /       \
ctx24     ctx10

匹配结果

    ctx34,ctx28 = add
      /        \
  ctx30        1111
    |
add ctx30,ctx24,ctx10

迭代后变为

add ctx30,ctx24,ctx10
add ctx34,ctx30,1111
```

### 归类映射寄存器

因为在转化规则的确定中发现add的第一个参数与结果在同一个寄存器，因此可以确定ctx24，ctx30和ctx34指向同一寄存器

### 转换汇编指令——动态规划

以下三条指令

```
mov eax, dword ptr [edi+100]
add edi, 100
lea ebx, [edi+100]
```

* 第一条包含了第二条（等于 add edi, 100   mov eax, [edi]）
* 第三条与第二条除操作数外等价

在转化为汇编指令时，为每一个匹配规则设定一个权值，使用计算后值最大的匹配规则进行转换。这里由于第一条信息最多，所以权值较大，第二第三权值相等

### 寄存器染色

* 初始化虚拟机时各项所映射的寄存器
* 根据汇编转换规则映射或结束映射某项到某寄存器
* 退出虚拟机时通过弹出各项时确定各项最终映射的寄存器

#### 基本块内寄存器轮转

基本块内寄存器不会重新轮转，因此较简单。转换规则正确就可以识别

#### 基本块间寄存器轮转

在执行setjmp虚拟指令时，寄存器压栈，跳转后弹出到不同寄存器中，实现一次轮转

##### 寄存器二义性

###### push/pop

* 传值  pop传值时，将源项的值放入目标项，寄存器映射不变‘
* 传引用  pop传引用时，不但将源项放入目标项，目标项映射的寄存器也被覆盖，主要用于寄存器轮转

```
传值
push ctx20(edx)
pop ctx24(esi)
此时寄存器映射为 ctx20->edx    ctx24->esi

传引用
push ctx20(edx)
pop ctx24(edx)
此时寄存器映射由 ctx20->edx 转为 ctx24->edx
```

###### add/lea

* add时pop为传引用
* lea时pop为传值

除add和lea其他指令也存在push/pop二义性

##### 识别寄存器二义性

* 根据转换规则确定一些特性，缩小寄存器范围
* 无法判断的寄存器加入可能性列表，建立传递链表
* 退出虚拟机时可以确定真实寄存器
* 确定后返回排除二义性指令