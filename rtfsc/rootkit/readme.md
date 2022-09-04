## User Mode

### Jynx

[GitHub - chokepoint/jynxkit: JynxKit is an LD_PRELOAD userland rootkit for Linux systems with reverse connection SSL backdoor](https://github.com/chokepoint/jynxkit)

* 进程隐藏

* 文件隐藏

### Azazel

[GitHub - chokepoint/azazel: Azazel is a userland rootkit based off of the original LD_PRELOAD technique from Jynx rootkit. It is more robust and has additional features, and focuses heavily around anti-debugging and anti-detection.](https://github.com/chokepoint/azazel)

### vlany

[GitHub - mempodippy/vlany: Linux LD_PRELOAD rootkit (x86 and x86_64 architectures)](https://github.com/mempodippy/vlany)

#### Feature

* 进程隐藏

* 用户隐藏

* 网络隐藏

* LXC Container

* 反调试

* 反取证

* 持久化安装/重装（Presistent (re)installtion） & 反检测

* linker修改

* 后门
  
  * accept 函数后门（由Jynx2继承）
  
  * PAM后门
    
    * PAM权限logger
  
  * snodew 反弹shell （[GitHub - mempodippy/snodew: PHP root (suid) reverse shell](https://github.com/mempodippy/snodew)）

* 一些vlany自定义的指令

#### 具体描述

##### 文件隐藏

* hook readdir() 和 readdir64()

* 通过扩展文件属性隐藏。作者在文档里写希望将全部功能都迁移为使用扩展文件属性隐藏文件，但仍需要使用GID based的方法来隐藏进程，因为procfs不支持扩展属性

[GitHub - kris-nova/boopkit: Linux eBPF backdoor over TCP. Spawn reverse shells, RCE, on prior privileged access. Less Honkin, More Tonkin.](https://github.com/kris-nova/boopkit)
