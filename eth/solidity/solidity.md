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

如上例，可以使用`type(ActionChoices).min` `type(ActionChoices).max`获取枚举的最小值和最大值

##### 用户定义值

使用`type C is V`定义，使用`.wrap`和`.unwrap`进行类型转换

下面是官方的例子，自定义了一个`UFixed256x18`，即具有18位小数的十进制定点类型。这边直接借助uint256的基础运算实现其自身的运算

```solidity
pragma solidity ^0.8.8;


type UFixed256x18 is uint256;

library FixedMath {
    uint constant multiplier = 10**18;

    function add(UFixed256x18 a, UFixed256x18 b) internal pure returns (UFixed256x18) {
        return UFixed256x18.wrap( UFixed256x18.unwrap(a) + UFixed256x18.unwrap(b) );
    }

    function mul(UFixed256x18 a, uint256 b) internal pure returns (UFixed256x18) {
        return UFixed256x18.wrap( UFixed256x18.unwrap(a) * b );
    }

    function floor(UFixed256x18 a) internal pure returns (uint256) {
        return UFixed256x18.unwrap(a) / multiplier;
    }

    function toUFixed256x18(uint256 a) internal pure returns (UFixed256x18) {
        return UFixed256x18.wrap(a*multipiler);
    }
}
```

##### 函数类型

是函数的类型，直观理解有点像C语言的函数指针。函数类型的变量可以使用一个函数赋值，函数类型的参数可以用于将函数传递给函数调用，也可以用于返回一个函数类型的值

分为内部函数和外部函数，内部函数只能在当前合约内部调用，不能在当前合约的上下文之外执行；外部函数由地址和函数签名组成，可以通过外部函数调用

```
function (<parameter types>) {internal|external} [pure|view|payable] [returns (<return types>)]
```

状态可变性

* pure 不读取或修改状态变量和区块数据

* view 仅读取状态变量，不修改

* payable 可接收ETH

作为函数的注解可以优化gas消耗

###### 隐式转换

函数类型的隐式转换要求

* 参数类型相同

* 返回值类型相同

* 内部/外部属性相同

* 状态可变性只能往更严格的方向转换，即
  
  * pure -> view | non-payable
  
  * view -> non-payable
  
  * payable -> non-payable

对于最后一条规则，是因为一个payable的函数可以接受支付0以太，由此可以视为non-payable。此外，non-payable是拒绝以太的，比接受以太更为严格

未初始化或delete后调用函数类型的变量会导致panic error

在solidity上下文之外使用外部函数类型，将被视为function类型，该类型会将地址和函数标识符一起编码为`bytes24`类型

合约的公共函数可以同时用作内部和外部函数，如对于合约的公共函数f，内部形式即为f，外部形式则为this.f

成员包含：

* address  返回函数的合约地址

* selector  返回ABI函数选择器

其他细节和示例代码见 [类型 &mdash; Solidity 中文文档 — 登链社区](https://learnblockchain.cn/docs/solidity/types.html#function-types)

#### 引用类型

包括结构体、数组和映射

##### 数据位置

若使用引用类型，必须指明存储该类型的数据区域，称为数据位置

* memory  生命周期仅限于外部函数调用

* storage  存储状态变量的地点，生命周期限于合约本身的生命周期

* calldata  包含函数参数的特殊数据位置，是一个不可修改，非持久的区域

任何改变数据位置的赋值或类型转换都会引发复制，而同一数据位置内的幅值仅在某些情况下会引发复制

因为calldata可以避免复制，且确保数据无法被更改，因此建议使用。此外具有calldata数据位置的数组和结构体可以从函数返回，但无法直接分配

数据位置影响赋值的语义

* 在storage和memory之间的赋值，或从calldata进行的赋值总是会创建一个副本

* 从memory到memory的幅值仅创建引用

* 从storage到本地存储变量的赋值仅创建引用

* 所有其他对storage的赋值总会复制

文档中有个示例 [类型 &mdash; Solidity 中文文档 — 登链社区](https://learnblockchain.cn/docs/solidity/types.html#data-location-assignment)

##### 数组

###### 特性

可以有编译时的固定大小`T[k]`，也可以有动态大小`T[]`

注意，solidity声明多维数组的顺序与C是相反的，如`uint[][5] memory x`表示包含5个类型为uint的动态数组，但访问顺序与其他语言一致，如`x[2][6]`表示第3个动态数组的第7个元素（下标从0开始）

还有下列特性

* 数组元素可以是任何类型，但一些对类型的限制在数组上适用，如映射只能存储在storage位置、公开可见的函数需要的参数为ABI types

* 可以将状态变量数组标记为public并让solidity创建一个getter

* 可以使用push方法在数组末尾添加元素

* 访问超出数组长度的元素会导致断言失败

* bytes和string是特殊的数组，bytes等同于`bytes1[]`，string则是不允许索引访问的bytes

* 可以使用new分配内存数组，这里必须指定数组长度且无法改变
  
  ```solidity
  uint[] mmeory a = new uint[](7);
  ```

成员如下：

* length

* push() / push(x)

* pop()

###### 悬垂引用

这边有几个比较重要的例子

```solidity
// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.8.0 <0.9.0;

contract C {
    uint[][] s;

    function f() public {
        // 存储对 s 的最后一个数组元素的指针。
        uint[] storage ptr = s[s.length - 1];
        // 移除 s 的最后一个数组元素。
        s.pop();
        // 写入不再在数组中的数组元素。
        ptr.push(0x42);
        // 现在向 ``s`` 添加新元素不会添加空数组，
        // 而是会导致长度为 1 的数组，其元素为 ``0x42``。
        s.push();
        assert(s[s.length - 1][0] == 0x42);
    }
}
```

注意，`s.push()`没有指定内容时只是为s数组添加一个存储槽，并不会将其初始化为0，因此push后一般会重用之前pop出去的槽，而该槽与ptr指向同一个数组

下面是另一个例子

```solidity
// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.8.0 <0.9.0;

contract C {
    uint[] s;
    uint[] t;
    constructor() {
        // 向存储数组推送一些初始值。
        s.push(0x07);
        t.push(0x03);
    }

    function g() internal returns (uint[] storage) {
        s.pop();
        return t;
    }

    function f() public returns (uint[] memory) {
        // 以下将首先评估 ``s.push()`` 为对索引 1 处的新元素的引用。
        // 之后，对 ``g`` 的调用弹出这个新元素，导致最左边的元组元素变为悬空引用。
        // 赋值仍然发生，并将写入 ``s`` 的数据区域之外。
        (s.push(), g()[0]) = (0x42, 0x17);
        // 随后的对 ``s`` 的推送将揭示前一个句写入的值，
        // 语即在此函数结束时 ``s`` 的最后一个元素将具有值 ``0x42``。
        s.push();
        return s;
    }
}
```

运行f的第一条元组赋值语句时，执行顺序如下

* `s.push()`

* `g()`，其中将刚刚s中push的元素又pop出来了

* 此时作为左值的元素引用的数据为s[1]和t[0]，其中s[1]已经越界，但这里不会报错而是将0x42赋值到s的存储外

* `s.push()` 此时push一个空元素，但实际上这个元素已经被赋值为0x42

还有一种可能导致悬垂引用的，与bytes和string的短存储与长存储有关。对于小于等于31字节的bytes或string来说，元素和数组长度一起存储在同一个槽中，其中最低字节存储长度`length*2`；而对于大于31字节的bytes或string来说，主槽p存储长度`length*2+1`，数据则存在`keccak256(p)`中

```solidity
// SPDX-License-Identifier: GPL-3.0
pragma solidity >=0.8.0 <0.9.0;

// 这将发出一个警告
contract C {
    bytes x = "012345678901234567890123456789";

    function test() external returns(uint) {
        (x.push(), x.push()) = (0x01, 0x02);
        return x.length;
    }
}
```

这边x初始长度为30，第一次push后变为31，第二次push后变为32，由短存储变为长存储，但这里第一个push已经返回了原来短存储的引用，导致了混淆

**因此为了安全，建议单个赋值期间最多仅将字节数组扩大一个元素，且不要在同一语句中同时使用索引访问数组**

##### 数组切片

`x[start:end]` 目前仅可用于calldata数组。数组切片没有任何成员，也没有对应的类型名称，因此仅存在于中间表达式中

数组切片对于 ABI 解码通过函数参数传递的二级数据非常有用

##### 结构体

```solidity
struct Funder {
    address addr;
    uint amount;
}
```

访问一样通过`.`

结构体不能包含其自身类型的成员

##### 映射类型

`mapping(KeyType KeyName? => ValueType ValueName?)`

这里KeyName和ValueName可选。映射可以视为哈希表，默认初始化为所有键都可能存在，并映射到一个字节表示全为0的值。键的keccak256作为索引来查找值

由于上述初始化配置，映射没有长度或某个值是否被设置的概念

映射只能被放在storage里，因此允许作为状态变量、函数中的存储引用类型、库函数参数，不能作为公开可见的合约函数参数或返回参数

若将映射类型变量标记为public，solidity会自动创建getter函数，参数为KeyType，名称为KeyName，返回值为ValueType。若ValueType是数组或映射，则getter需要递归传入每个KeyType参数

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

##### 数组字面量

会生成一个静态大小的内存数组，长度为表达式的数量，基本类型是第一个表达式的类型

```solidity
[1, 2, 3]        // 默认生成uint8[3]
[uint(1), 2, 3]  // 生成uint[3]
```

目前不能使用数组字面量直接给动态数组赋值，因为固定长度数组与动态数据类型不一致。动态数组数据必须逐个赋值

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
