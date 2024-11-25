[智能合约概述 &mdash; Solidity 0.8.22 文档](https://docs.soliditylang.org/zh/v0.8.22/introduction-to-smart-contracts.html)

[智能合约简介 &mdash; Solidity 中文文档 0.8.28 文档](https://learnblockchain.cn/docs/solidity/introduction-to-smart-contracts.html)

## 简单示例

### 简单存储合约

```solidity
// SPDX-License-Identifier: GPL-3.0        // 授权信息
pragma solidity >=0.4.16 <0.9.0;           // solidity版本号

contract SimpleStorage {                   // 合约内容
    uint storedData;

    function set(uint x) public {
        storedData = x;                    // 这里成员不需要使用this访问
    }

    function get() public view returns (uint) {
        return storedData;
    }
}
```

该合约允许任何人在合约中存储一个数字，也允许任何人读出

### 简单加密货币

```solidity
// SPDX-License-Identifier: GPL-3.0
pragma solidity ^0.8.4

contract Coin {
    address public minter;            // 定义了一个可以从其他合约中访问的地址变量
    maaping(address => uint) public balances;    // 定义了用户钱包，是一个地址到整型的映射

    // 声明了一个事件，以太坊客户端可以监听区块链上发起的这些事件
    event Sent(address from, address to, uint amount);

    // 合约构造函数，只在创建合约时执行一次
    // msg是一个默认全局变量，msg.sender为当前调用函数的区块链地址
    // 因此minter存储的是创建者的地址
    constructor() {
        minter = msg.sender;
    }

    // 该函数用于增加地址的代币数量，需要调用者地址为合约创建者
    // 因此该合约完全由合约创建者掌控，可以给任意账户加钱
    function mint(address receiver, uint amount) public {
        require(msg.sender == minter);
        balances[receiver] += amount;    // 注意当和超过2**256时会溢出
    }

    // 定义了一个错误，向调用者提供
    error InsufficientBalance(uint requested, uint available);

    // 转账函数
    function send(address receiver, uint amount) public {
        if (amount > balances[msg.sender])    // 若转账数量大于账户余额
            revert InsufficientBalance({      // 返回错误
                    requested: amount,
                    available: balances[msg.sender]
                });

            balances[msg.sender] -= amount;   // 转账并向区块链发送一条信息
            balances[receiver] += amount;
            emit Sent(msg.sender, receiver, amount);
    }
}
```

所有public变量默认会自动生成一个函数，使得外部可以访问该变量的值

```solidity
function minter() external view returns (address) { return minter; }
function balances(address account) external viwe returns (uint) {
    return balances[account];
}
```

## 区块链基本概念

### 交易/事务

区块链是全球共享的事务性数据库，每个人都可以访问数据库中的记录

指一件原子性的事情，比如转账事务保证源账户的金额扣除和目标账户的金额增加是原子性的，要么同时发生要么同时不发生

交易总是由发送人签名，以此为数据库的特定修改增加访问保护机制

### 区块

在比特币中存在双花攻击：若网络中同时存在两笔交易，都要清空一个账户，应该接受哪一个

对于区块链来说，问题的答案是：区块链会决策出一个全球公认的交易顺序，并且以固定的时间间隔将间隔中的所有交易捆绑为“区块”，添加到链上

由于存在一个顺序选择的机制，可能发生区块被回滚的情况，但仅会发生在链的末端

## 以太坊虚拟机

### 账户

#### 账户类型

以太坊存在两类账户，这两类账户共享同一个地址空间

* 外部账户  由人控制（由公私钥对访问）

* 合约账户  由与账户一起存储的代码控制，在合约创建时确定（由创建者地址和从该地址发出的交易数量计算得到地址）

#### 存储

每个账户都有一个持久的键值存储，将256位的字映射到256位的字

#### 余额

每个账户都有一个以太余额（balance），单位是Wei，`1 ether=10**18 Wei`

### 交易

交易可以看做是一个账户发送消息到另一个账户

若目标账户有代码，则执行代码，并且以交易的负载（二进制数据）作为参数

若交易没有设置目标账户（交易没有接收者或接收者为null），交易则会创建一个新的合约，此时交易的有效负载被认为是EVM字节码执行，执行输出的数据被永久存储为合约的代码。因此为创建一个合约，发送的其实不是合约的代码，而是产生合约代码的代码

### Gas

每笔交易都需要消耗一定Gas，Gas由交易的发起人支付。EVM的每条指令都会消耗特定的Gas，若在执行合约过程中Gas耗尽，则会引发异常，并撤销所有修改

该机制主要用于激励对EVM执行时间的经济利用、补偿矿工以及限制验证区块所需的工作量

Gas price是交易发起人设置的值，在发起交易时必须提前向EVM支付，执行完剩下的Gas会退还；如果发生撤销更改的异常，已使用的Gas不会退还

由于EVM执行器可以选择包含一笔交易， 因此交易发送者无法通过设置低燃料价格（Gas price）滥用系统。

### 存储 内存和栈

EVM有三个存储数据的区域：存储器、内存和栈

* 存储  每个账户都有存储，该存储是永久的，读取、初始化和修改的成本较高，因此应尽量减少这类存储的使用。合约无法读取\写入自身存储以外的存储区域数据

* 内存  合约每次进行消息调用时都会有新的内存实例，内存是线性的，为字节级寻址，读取宽度限制为256位，写的宽度则可以是8位或256位。当访问一个先前未访问过的内存时，内存会被扩展，该操作需要支付Gas，因此内存越大成本越高（平方增长）

* 栈  EVM是栈虚拟机，栈最大有1024个元素，每个元素256位，EVM字节码只能对栈顶一定范围的数据进行操作
  
  * 拷贝/交换  拷贝栈顶16个元素之一到最顶端，或交换栈顶元素和栈顶16个元素之一
  
  * 其他操作  只能取栈顶两个元素，且运算结果保存到栈顶
  
  可以将栈上元素放到存储或内存中，但无法直接访问栈上指定深度的元素，只能先将要读的元素放到栈顶

### 指令集

指令在基本的数据类型上操作：256位的字或内存的片断（或其他字节数组）

### 消息调用

一次消息调用包括源、目标、数据、以太币、Gas和返回数据

合约通过消息调用的方式调用其他合约，或发送以太币到非合约账户

每个交易都由一个顶层消息调用组成，而该消息调用可以创建更多消息调用

在调用其他合约时，合约可以决定这次消息调用发送多少gas，若在其他合约那边Gas用尽，则会在本合约产生一个异常。被调用的合约会收到一个新清空的内存实例，并可以访问调用的有效负载提供的数据（存放在一个叫calldata的独立区域）。执行完毕后可以返回数据，被存储在调用者内存中预先分配的位置

调用被限制在1024的深度，且消息调用中只有63/64的Gas可以被转发

### 委托调用和库

委托调用（delegate call）是一种特殊的消息调用，委托调用中目标地址的代码会在调用合约的上下文（地址）中执行，因此msg.sender和msg.value与普通消息调用不一致

这就在solidity中引入了类似库的概念，合约可以再运行时从其他地址加载代码到当前上下文执行

### 日志

一种特殊的可索引的数据结构，其存储的数据可以映射到每个区块的层级。合约创建后其本身无法访问日志数据，但区块链外可以高效的访问，且可以通过布隆过滤器安全加密地访问日志

### 创建

合约可以通过一个特殊的指令创建其他合约，被称为创建调用（create call）

该调用与普通调用的区别是，负载会被执行，执行的结果被存储为合约代码，并且会将新合约的地址返回给调用者

### 停用和自毁

合约地址可以通过执行selfdestruct操作进行自毁，存储在该地址的剩余以太币会被发送到一个指定目标，存储和代码则被从状态中删除。注意：若剩余以太币被发送到一个被删除的合约，则以太币会永久丢失

即使合约进行了自毁，其区块信息仍会是区块链历史的一部分

即使合约代码没有显式调用selfdestruct，仍可能通过delegatecall或callcode进行自毁

### 预编译合约

在一些特殊的地址范围存在预编译合约，可以像其他合约一样被调用，但他们执行的不是该地址的EVM代码，而是EVM环境实现的一些特殊功能，有点类似EVM开出的接口

目前地址1~8之间包含预编译合约，而对这类地址范围的合理预期为1~0xffff

## Solidity详解

### Solidity源文件结构

#### SPDX许可证标识符

标识代码的开源许可证，编译器不会验证许可证是否属于SPDX允许的列表

若非开源，或不想指定许可证，可以用UNLICENSED，注意与UNLICENSE区分，后者是授予所有人所有权利

#### Pragma

标识，用于指定一系列编译器特性，通常只对本文件有效

##### 版本标识

指定代码在大于0.5.2小于0.6.0的版本编译，因为`^`只包含符合该版本号的所有子版本号，所以0.6.0以上不包含在内

```solidity
pragma solidity ^0.5.2;
```

##### ABI Coder Pragma

指定不同的ABI encoder和decoder，可以指定为

```solidity
pragma abicoder v1;  或
pragma abicoder v2;
```

新编码器支持更多类型，也有更广泛的验证和安全检查，但需要更多Gas。0.8.0后默认使用v2

v2可以完美兼容v1，而v1的代码只有在尝试进行只有v2支持的调用时才会发出错误

##### 实验性编译指令

###### ABIEncoderV2

即上述特性

###### SMTChecker

通过SMT求解器对代码进行额外验证

```solidity
pragma experimental SMTChecker;
```

#### 导入其他源文件

```solidity
import "filename";        // 导入文件中的所有全局符号，不推荐，会污染命名空间
import * as symbolName from "filename";    // 将导入的文件符号放到symbolName命名空间内
import "filename" as symbolName;    // 与上条语句相同

// 若导入符号存在命名冲突，可以重命名符号
// 导入filename中的符号symbol1 symbol2，并为symbol1创建别名alias
import {symbol1 as alias, symbol2} from "filename";
```

#### 导入路径

为了支持在所有平台上构建，solidity抽象了源文件存储的文件系统的细节，因此导入路径与主机系统的文件路径不一致，而是由编译器维护一个数据库。详情见

[导入路径解析 &mdash; Solidity 中文文档 0.8.28 文档](https://learnblockchain.cn/docs/solidity/path-resolution.html#path-resolution)

#### 注释

`//` 或 `/* */`

或是NatSpec风格注释，行注释为`///`，块注释为`/** */`

### 合约的结构

#### 状态变量

状态变量的值可能永久存储在合约存储中，也可能临时存储在瞬态存储中（交易结束时清除）

```solidity
contract SimpleStorage {
    uint storedData;
}
```

#### 函数

可执行代码的单元，通常在合约内部定义，也可以在合约外部定义

```solidity
contract SimpleAuction {
    function bid() public payable {                // 合约内部定义
        // ...
    }
}


functionn helper(uint x) pure returns (uint) {    // 合约外部定义的辅助函数
    return x*2;
}
```

函数调用可以在内部或外部发生，并且对其他合约有不同级别的可见性

#### 函数修改器

函数修改器可以以声明的方式修改函数的语义

```solidity
contract Purchase {
    address public seller;

    modifier onlySeller() {                        // 定义函数修改器
        require(
            msg.sender == seller,                  // 只有卖家可以调用函数
        )
        _;
    }

    function abort() public view onlySeller {      // 在声明中用函数修改器限制函数调用方
        // ...
    }
}
```

#### 事件

事件可以方便地调用EVM的日志功能接口

```solidity
event HighestBidIncreased(address bidder, uint amount);

contract SimpleAuction {
    function bid() public payable {
        emit HighestBidIncreased(msg.sender, msg.value);
    }
}
```

#### 错误

用于定义失败情况的描述性名称和数据，可以与revert语句配合使用

```solidity
error NotEnoughFunds(uint requested, uint available);

contract Token {
    mapping(address => uint) balances;
    function transfer(address to, uint amount) public {
        uint balance = balances[msg.sender];
        if (balance < amount)
            revert NotEnoughFunds(amount, balance);
        balances[msg.sender] -= amount;
        balances[to] += amount;
    }
}
```

#### 结构体类型

```solidity
contract Ballot {
    struct Voter {
        uint weight;
        bool voted;
        address delegate;
        uint vote;
    }
}
```

#### 枚举类型

```solidity
contract Purchase {
    enum State { Created, Locked, Inactive }
}
```

### 类型

solidity中没有undefined或null，但每种类型都有默认值

#### 值类型

##### 布尔类型

bool，值为true或false

运算符：`! && || == !=`

##### 整型

int/uint，有/无符号整型，分别代表int256和uint256，此外两类都还有从8开始的长度（uint8 uint16等）

运算符：

* 比较 `<= < == != >= >`

* 位运算 `& | ^ ~`

* 移位 `<< >>`

* 算术 `+ - * / % **`，以及负号`-`

可以使用`type(X).min  type(X).max`获取该类型的最小值和最大值

**注意**：对于算术运算，有两类检查模式：unchecked（wrapping 截断模式）和checked模式。默认是checked，此时若移除，则调用通过断言回退。使用`unchecked {...}`可以回退到unchecked模式

##### 定长浮点

尚未完全支持，目前仅能声明，不能赋值

使用fixedMxN或ufixedMxN声明，其中M表示类型占用的位数，N表示可用的小数位数。M必须为8的倍数，范围8~256

默认的fixed和ufixed分别为fixed128x18和ufixed128x18

运算符：

* 比较 `<= < == != >= >`

* 算术 `+ - * / %`，以及负号`-`

##### 地址

* address  20字节，为以太坊地址的大小

* address payable  与address类似，但额外有成员transfer和send。与address的区别为address payable是一个可以发送以太币的地址，而address则不能直接接收以太币

运算符：`<= < == != >= >`

类型转换：

* address payable => address  可以直接隐式转换

* address => address payable  `payable(<address>)`

* uint160 => address  可显式转换

* bytes20 => address 可显式转换

###### 成员变量和函数

* balance  地址的余额

* transfer  支付以太币的函数（仅有address payable拥有）

* send  transfer的底层函数

* call delegatecall staticcall  底层函数，见[类型 &mdash; Solidity 中文文档 0.8.28 文档](https://learnblockchain.cn/docs/solidity/types.html#members-of-addresses)

* code codehash  地址部署的合约代码

##### Contract 合约类型

由contract定义合约类型，可以显式转换为address，或从address类型转换，但只有合约类型具有接收或可支付的回退函数时才能显式转换为address payable，否则需要使用`payable(address(x))`强制转换

若声明合约类型的局部变量，则可以在其上调用函数，但要注意赋值时合约类型应相同

合约可以实例化，其成员是public标记的外部函数和状态变量

##### 定长字节数组

`bytes1`到`bytes32`，因为寄存器单元长度为256位

运算符：

* 比较 `<= < == != >= >`

* 位运算 `& | ^ ~`

* 移位运算 `<< >>`

* 索引 `x[k]`

##### 枚举

默认从0开始，最多256个成员

```solidity
enum ActionChoices { GoLeft, GoRight, GoStraight, SitStill };
```

##### 用户定义值

使用`type C is V`定义

#### 字面量

##### 地址字面量

十六进制，长度为160位，即20字节，对应40个十六进制数

##### 有理数/整数字面量

支持十进制分数字面量，如1.3

支持科学计数法，如2e10

可以用下划线分割字面量，如123_000

虽然字面量支持小数，但是作为常数在编译时就处理的，类似`a+2.5`的代码不能通过编译

##### 字符串字面量

单引号或双引号包裹，没有00截断，因此`"foo"`类型为bytes3

转义符见[类型 &mdash; Solidity 中文文档 0.8.28 文档](https://learnblockchain.cn/docs/solidity/types.html#string-literals)

##### Unicode字面量

以unicode为前缀，`string memory a = unicode"Hello";`

##### 十六进制字面量

以hex为前缀，`hex"AA55"`

### 运算

#### 比较

比较的是整数值，solidity中没有浮点

#### 位运算

在整数的二进制补码上执行

#### 移位

移位不会有溢出检查，其结果总是被截断。右操作数必须为无符号数

#### 加法、减法、乘法

默认执行检查，可以用个unchecked块关闭

因为整数由补码表示，因此若有`int x = type(int).min;`，则`assert(-x == x);`会触发断言，但`unchecked{ assert(-x == x); }`则不会触发

#### 除法

solidity没有浮点数，会向0舍入

#### 模运算

注意，模运算结果的符号与其左操作数相同，即

```
int256(5) % int256(2) == int256(1)
int256(5) % int256(-2) == int256(1)
int256(-5) % int256(2) == int256(-1)
int256(-5) % int256(-2) == int256(-1)
```

#### 指数

仅适用于无符号类型的指数，产生的结果类型与基数类型一致，会进行溢出检查。当使用较小基数的指数时编译器会进行优化使得Gas消耗较少，如`x**3`优化为`x*x*x`
