https://ethereum.org/zh/developers/docs/

# 基础

## 账户

有两种账户

* 外部所有的账户  由任何拥有私钥的人控制

* 合约账户  部署到网络上的智能合约，由代码控制

两种账户都可以

* 接收、持有和发送ETH/token

* 与已部署的智能合约进行交互

区别：

* 外部账户
  
  * 创建免费
  
  * 可以发起交易
  
  * 所有账户间只能进行以太币和代币交易
  
  * 由一对加密密钥组成：公私钥

* 合约
  
  * 创建有成本，因为需要使用网络存储空间
  
  * 只能在收到交易时发送交易
  
  * 外部账户向合约账户发起的交易能触发各种代码
  
  * 没有私钥，而是由代码逻辑控制

以太坊账户的四个字段

* nonce  计数器，显示外部账户发送的交易数量或合约账户创建的合约数量。每个账户只能执行具有一个给定随机数的一笔交易，用于防重放

* balance  地址拥有Wei的数量

* codeHash  对于合约账户是代码的哈希，对于外部账户则为空字符串的哈希

* storageRoot  存储哈希，编码了账户的存储内容，默认情况下为空

创建账户时，会使用椭圆曲线加密生成一对公私钥，私钥由64个十六进制字符组成。公共地址为公钥的Keccack256哈希的最后20个字节（实际由一个42字符的字符串表示，前两个字符为0x，后40个字符为20个字节地址的十六进制表示）

以太坊还有一种密钥叫验证者密钥，用于识别验证者

## 交易

一笔交易包含以下信息：

* from  发送者地址

* to  接收地址

* signature  发送者标识符

* nonce  一个有序递增的计数器，即账户的交易数量

* value  转移的以太币数量

* input data  可选字段，可包含任意数据

* gasLimit  交易可以小号的最大数量gas

* maxPriorityFeePerGas   作为费用提供给验证者的已消耗燃料的最高价格

* maxFeePerGas  愿意为交易支付的每单位燃料的最高费用

Gas费用分为两块：base fee和priority fee。前者由协议指定，即调用的协议规定必须至少支付这些金额才视为有效，后者是给验证者的小费。每个区块都有一个基础费作为底价，该价格是由当前区块之前的区块决定的，由一个公式计算得出，该公式将上一个区块大小与目标区块大小进行比较，如果超过目标区块大小则新区块的基础费将最多增加12.5%。这可以限制区块的大小不会一直保持在高位（因为这样的话基础费会指数级增加）

计算最大支付费用的方式为

```
(maxPriorityFeePerGas + maxFeePerGas) * Gas数量
```

一次简单的转账交易需要的Gas数量为21000单位

一笔交易的生命周期如下：

* 生成交易哈希

* 交易被广播到网络，并添加到由所有其他待处理的网络交易组成的交易池中

* 验证者选择你的交易并将它包含在一个区块中

* 随着区块信息的验证和打包，交易区块将升级成“合理”状态，并最后成为“最后确定”状态

交易编码：

早期以太坊每笔交易都为RLP编码

```
RLP([nonce, gasPrice, gasLimit, to, value, data, v, r, s])
```

现在已支持多种类型的交易，编码为

```
TransactionType || TransactionPayload
```

* TransactionType 0~0x7f间的数组，指定交易类型

* TransactionPayload  由交易类型定义的任意字节数组

交易有以下几类

* Type 0  传统交易，即RLP编码的交易

* Type 1  包含一个 `accessList` 参数，该列表指定了交易预期访问的地址和存储密钥，有助于降低潜在燃料花费

* Type 2  目前的标准交易类型，将交易费分为基础费用和优先费用，在交易头包含`maxPriorityFeePerGas` 和 `maxFeePerGas` 的字段

## 区块

内容比较多但是大多是细节，直接看网页吧 https://ethereum.org/zh/developers/docs/blocks/

几个比较关键的信息

以太坊的区块每12秒提交一次，且每个区块的目标大小为1500万单位Gas，上限为3000万单位Gas，保证区块不会过于膨胀

区块的权益证明协议如下：

* 验证节点向存款合约中质押32个以太币

* 每个时隙中（12秒时间间隔），随机选择一个验证者作为区块提议者，他们将交易打包并执行，之后将信息包装到一个区块中，并传送给其他验证者

* 验证者再次执行交易，若验证为有效，则验证者将该区块添加进各自的数据库；若验证者发现在同一时隙内有两个冲突区块，则使用分叉选择算法表决，获得最多质押以太币支持的区块作为最终区块

# 高级

## 标准

### 令牌标准

#### ERC-20：同质化代币

是一个能实现智能合约中代币的应用程序接口标准

功能示例包括

* 将代币从一个账户转到另一个账户

* 获取帐户的当前代币余额

* 获取网络上可用代币的总供应量
- 批准一个帐户中一定的代币金额由第三方帐户使用

##### 方法

```solidity
function name() public view returns (string)
function symbol() public view returns (string)
function decimals() public view returns (uint8)
function totalSupply() public view returns (uint256)
function balanceOf(address _owner) public view returns (uint256 balance)
function transfer(address _to, uint256 _value) public returns (bool success)
function transferFrom(address _from, address _to, uint256 _value) public returns (bool success)
function approve(address _spender, uint256 _value) public returns (bool success)
function allowance(address _owner, address _spender) public view returns (uint256 remaining)
```

##### 事件

```solidity
event Transfer(address indexed _from, address indexed _to, uint256 _value)
event Approval(address indexed _owner, address indexed _spender, uint256 _value)
```

##### ERC-20代币接收问题

**当 ERC-20 代币被发送到并非为处理 ERC-20 代币而设计的智能合约时，这些代币可能会永久丢失**

主要原因有：

* 代币转移机制  当用户使用transfer或transferFrom函数转移代币时，无论接收合约是否为处理它们而设计，代币都会被转移

* 缺乏通知  接收合约不会向发送代币的合约通知或回调。若接收合约缺乏处理代币的机制，代币会卡在合约的地址中

* 无内置处理  标准不包含用于接收待实现合约的强制函数，导致许多合约无法正确管理传入的代币


