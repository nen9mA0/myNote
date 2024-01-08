## 访问控制

### C2安全级别要求

* 每个资源必须有对不同用户和用户组间的访问控制

* 内存必须被保护以防其进程释放资源后被其他进程所读取。同样文件系统中的文件在删除后必须防止其被二次读取

* 用户登录时必须以一定方式认证他们，如密码。所有可审核的操作必须标识执行操作的用户。

* 系统管理员必须能够审计与安全相关的事件。且只有经过认证的管理员有权限审计。

* 必须保护系统免受外部干扰或篡改，例如修改正在运行的系统或存储在磁盘上的系统文件。

### 访问控制模型

当用户登录时，系统创建一个访问令牌。所有该用户创建及使用的进程都有一份令牌的副本。访问令牌包括安全标识符，其标识了用户及其所在的用户组，还包括用户及用户组拥有的权限。当进程试图访问一个有安全对象或执行一个系统管理任务时，都使用该用户的访问令牌请求。

当一个安全对象被创建时，若没有指定，系统分配一个与它创建者安全设置相同的安全描述符或是一个默认设置的安全描述符。程序可以使用API改变这些对象的安全设置。

**安全描述符**除了标识对象的所有者外还可能有以下信息：

* DACL(discretionary access control list) 一个标识用户或用户组是否有访问权限的列表
* SACL(system access control list) 控制系统访问对象的方式

ACL是包含多个ACE(access control entries)的列表，每个ACE指定了一个对应某**trustee**的SID及其一系列的访问权限。

#### 访问令牌

访问令牌定义了一个进程或线程的安全上下文。当用户登录系统时，系统会为其创建一个访问令牌。所有该用户执行的程序都使用令牌的副本。

访问令牌内容

* 用户SID
* 用户所在的组SID
* 登录SID，用于标识当前会话
* 用户及用户组的权限列表
* 所有者的SID
* 主用户组的SID
* 默认DACL，当用户不指定安全描述符创建一个安全对象时将使用这个DACL
* 访问令牌的源
* 令牌是否为模拟令牌
* 当前模拟等级
* 其他数据

每个进程有一个主令牌，用以描述用户的安全上下文。默认情况下，系统使用主令牌访问一个安全对象。此外，线程允许模拟一个其他用户的账户，并使用其令牌。

##### 相关API

| [**AdjustTokenGroups**](https://msdn.microsoft.com/en-us/library/Aa375199(v=VS.85).aspx)     | Changes the group information in an access token.                                                                                                                      |
| -------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [**AdjustTokenPrivileges**](https://msdn.microsoft.com/en-us/library/Aa375202(v=VS.85).aspx) | Enables or disables the privileges in an access token. It does not grant new privileges or revoke existing ones.                                                       |
| [**CheckTokenMembership**](https://msdn.microsoft.com/en-us/library/Aa376389(v=VS.85).aspx)  | Determines whether a specified SID is enabled in a specified access token.                                                                                             |
| [**CreateRestrictedToken**](https://msdn.microsoft.com/en-us/library/Aa446583(v=VS.85).aspx) | Creates a new token that is a restricted version of an existing token. The restricted token can have disabled SIDs, deleted privileges, and a list of restricted SIDs. |
| [**DuplicateToken**](https://msdn.microsoft.com/en-us/library/Aa446616(v=VS.85).aspx)        | Creates a new impersonation token that duplicates an existing token.                                                                                                   |
| [**DuplicateTokenEx**](https://msdn.microsoft.com/en-us/library/Aa446617(v=VS.85).aspx)      | Creates a new primary token or impersonation token that duplicates an existing token.                                                                                  |
| [**GetTokenInformation**](https://msdn.microsoft.com/en-us/library/Aa446671(v=VS.85).aspx)   | Retrieves information about a token.                                                                                                                                   |
| [**IsTokenRestricted**](https://msdn.microsoft.com/en-us/library/Aa379137(v=VS.85).aspx)     | Determines whether a token has a list of restricting SIDs.                                                                                                             |
| [**OpenProcessToken**](https://msdn.microsoft.com/en-us/library/Aa379295(v=VS.85).aspx)      | Retrieves a handle to the primary access token for a process.                                                                                                          |
| [**OpenThreadToken**](https://msdn.microsoft.com/en-us/library/Aa379296(v=VS.85).aspx)       | Retrieves a handle to the impersonation access token for a thread.                                                                                                     |
| [**SetThreadToken**](https://msdn.microsoft.com/en-us/library/Aa379590(v=VS.85).aspx)        | Assigns or removes an impersonation token for a thread.                                                                                                                |
| [**SetTokenInformation**](https://msdn.microsoft.com/en-us/library/Aa379591(v=VS.85).aspx)   | Changes a token's owner, primary group, or default DACL.                                                                                                               |

##### 相关数据结构

| Structure                                                                                                           | Description                                                                                           |
| ------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| [**TOKEN_CONTROL**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_control)             | Information that identifies an access token.                                                          |
| [**TOKEN_DEFAULT_DACL**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_default_dacl)   | The default DACL that the system uses in the security descriptors of new objects created by a thread. |
| [**TOKEN_GROUPS**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_groups)               | Specifies the SIDs and attributes of the group SIDs in an access token.                               |
| [**TOKEN_OWNER**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_owner)                 | The default owner SID for the security descriptors of new objects.                                    |
| [**TOKEN_PRIMARY_GROUP**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_primary_group) | The default primary group SID for the security descriptors of new objects.                            |
| [**TOKEN_PRIVILEGES**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_privileges)       | The privileges associated with an access token. Also determines whether the privileges are enabled.   |
| [**TOKEN_SOURCE**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_source)               | The source of an access token.                                                                        |
| [**TOKEN_STATISTICS**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_statistics)       | Statistics associated with an access token.                                                           |
| [**TOKEN_USER**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ns-winnt-_token_user)                   | The SID of the user associated with an access token.                                                  |

##### 相关枚举类型

| Enumeration type                                                                                                            | Specifies                                                                       |
| --------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| [**TOKEN_INFORMATION_CLASS**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ne-winnt-_token_information_class) | Identifies the type of information being set or retrieved from an access token. |
| [**TOKEN_TYPE**](https://docs.microsoft.com/en-us/windows/desktop/api/Winnt/ne-winnt-_token_type)                           | Identifies an access token as a primary or impersonation token.                 |

#### SID

SID是一个用于唯一标识trustee的值。每个账号都有一个唯一的SID并存放在安全数据库中。每次登录时系统将用户SID赋值给访问令牌的对应成员。

SID在以下元素中被使用：

* 安全描述符使用SID以描述对象的所有者和主用户组
* ACE中用于标识其定义的访问权限对应的trustee
* 访问令牌中用于标识用户及其所在组
