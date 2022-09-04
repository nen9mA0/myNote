该列表主要来源于 https://github.com/milabs/awesome-linux-rootkits

### 🙈 user mode rootkits

- [GitHub - mempodippy/vlany: Linux LD_PRELOAD rootkit (x86 and x86_64 architectures)](https://github.com/mempodippy/vlany)
  
  Linux LD_PRELOAD rootkit (x86 and x86_64 architectures)
  
  😆 [**Doing**](user_mode/vlany/vlany.md) 

- https://github.com/Error996/bdvl
  
  Loosely based on mempodippy previous rootkit project, [vlany](https://github.com/mempodippy/vlany).

- [GitHub - unix-thrust/beurk: BEURK Experimental Unix RootKit](https://github.com/unix-thrust/beurk)
  
  BEURK is an userland preload rootkit for GNU/Linux, heavily focused around anti-debugging and anti-detection.
  
  🤡 [**Done**](user_mode\beurk\beurk.md) 

- [GitHub - chokepoint/azazel: Azazel is a userland rootkit based off of the original LD_PRELOAD technique from Jynx rootkit. It is more robust and has additional features, and focuses heavily around anti-debugging and anti-detection.](https://github.com/chokepoint/azazel)
  
  Azazel is a userland rootkit based off of the original LD_PRELOAD technique from Jynx rootkit.
  
  🤡 [**Done**](user_mode\azazel\azazel.md) 

- [GitHub - chokepoint/Jynx2: JynxKit2 is an LD_PRELOAD userland rootkit based on the original JynxKit. The backdoor has been replaced with an &quot;accept()&quot; system hook.](https://github.com/chokepoint/Jynx2)
  
  JynxKit2 is an LD_PRELOAD userland rootkit based on the original JynxKit.
  
  🤡 [**Done**](user_mode\jynx\jynx.md) 

- [GitHub - chokepoint/jynxkit: JynxKit is an LD_PRELOAD userland rootkit for Linux systems with reverse connection SSL backdoor](https://github.com/chokepoint/jynxkit)
  
  JynxKit is an LD_PRELOAD userland rootkit for Linux systems with reverse connection SSL backdoor
  
  🤡 [**Done**](user_mode\jynx\jynx.md) 

- [GitHub - NexusBots/Umbreon-Rootkit](https://github.com/NexusBots/Umbreon-Rootkit)
  
  LD_PRELOAD based
  
  😆 [**Doing**](user_mode/Umberon/Umberon.md) 

- [GitHub - ChristianPapathanasiou/apache-rootkit: A malicious Apache module with rootkit functionality](https://github.com/ChristianPapathanasiou/apache-rootkit)
  
  A malicious Apache module with rootkit functionality

### 🙉 kernel mode rootkits

* [.:: Phrack Magazine ::.](http://phrack.org/issues/58/7.html) SucKIT
  
  😆 [**Doing**](kernel_mode/SucKIT/SucKIT.md) 
- https://github.com/jermeyyy/rooty
  
  Academic project of Linux rootkit made for Bachelor Engineering Thesis.
  
  😆 [**Doing**](kernel_mode/rooty/rooty.md) 

- https://github.com/trailofbits/krf
  
  A kernelspace randomized syscall faulter for Linux 4.15+

- https://github.com/f0rb1dd3n/Reptile ⚡ [details](https://github.com/milabs/awesome-linux-rootkits/blob/master/details/reptile.md) ⚡
  
  Reptile is a LKM rootkit written for evil purposes that runs on Linux kernel 2.6.x/3.x/4.x

- https://github.com/QuokkaLight/rkduck ⚡ [details](https://github.com/milabs/awesome-linux-rootkits/blob/master/details/rkduck.md) ⚡
  
  rkduck - Rootkit for Linux v4

- https://github.com/croemheld/lkm-rootkit
  
  A LKM rootkit for most newer kernel versions.

- https://github.com/mncoppola/suterusu
  
  An LKM rootkit targeting Linux 2.6.x/3.x on x86, and ARM

- https://github.com/shekkbuilder/ARPRootKit
  
  An open source rootkit for the Linux Kernel to develop new ways of infection/detection.
  
  该rootkit还有改进版本
  
  [GitHub - akpotter/ARP-RootKit: An open source rootkit for the Linux Kernel to develop new ways of infection/detection.](https://github.com/akpotter/ARP-RootKit)

- https://github.com/nurupo/rootkit
  
  Linux rootkit for Ubuntu 16.04 and 10.04 (Linux Kernels 4.4.0 and 2.6.32), both i386 and amd64

- https://github.com/m0nad/Diamorphine
  
  LKM rootkit for Linux Kernels 2.6.x/3.x/4.x/5.x (x86 and x86_64)

- https://github.com/ivyl/rootkit
  
  Sample Rootkit for Linux
  
  🤡 [**Done**](kernel_mode/ivyl_rootkit.md) 

- https://github.com/deb0ch/toorkit
  
  A simple useless rootkit for the linux kernel
  
  就是个简单的systable hook的demo，没有对应笔记

- https://github.com/vrasneur/randkit
  
  Random number rootkit for the Linux kernel

- https://github.com/Eterna1/puszek-rootkit
  
  Yet another LKM rootkit for Linux. It hooks syscall table.

- https://github.com/trimpsyw/adore-ng
  
  linux rootkit adapted for 2.6 and 3.x

- https://github.com/bones-codes/the_colonel
  
  An experimental linux kernel module (rootkit) with a keylogger and built-in IRC bot

- https://github.com/David-Reguera-Garcia-Dreg/enyelkm
  
  LKM rootkit for Linux x86 with the 2.6 kernel. It inserts salts inside system_call and sysenter_entry.

- https://github.com/falk3n/subversive
  
  x86_64 linux rootkit using debug registers

- https://github.com/jiayy/lkm-rootkit
  
  An lkm rootkit support x86/64,arm,mips

- https://github.com/a7vinx/liinux
  
  A linux rootkit works on kernel 4.0.X or higher

- https://github.com/hanj4096/wukong
  
  Wukong: a LKM rootkit for Linux kernel 2.6.x, 3.x and 4.x

- https://github.com/varshapaidi/Kernel_Rootkit
  
  Linux Kernel Rootkit - To hide modules and ssh service

- https://github.com/kacheo/KernelRootkit
  
  Linux kernel rootkit to hide certain files and processes.

- https://github.com/dsmatter/brootus
  
  bROOTus is a Linux kernel rootkit that comes as a single LKM (Loadable Kernel Module) and it is totally restricted to kernel 2.6.32.

- https://github.com/jarun/keysniffer
  
  A Linux kernel module to grab keys pressed in the keyboard.

- https://github.com/PinkP4nther/Sutekh
  
  An example rootkit that gives a userland process root permissions (x86, 4.x)

- [GitHub - En14c/LilyOfTheValley: Simple LKM linux kernel rootkit (x86 / x86_64)](https://github.com/En14c/LilyOfTheValley)
  
  LilyOfTheValley is a simple LKM linux kernel rootkit for v4.x that works on (x86 and x86_64)

- https://github.com/NoviceLive/research-rootkit
  
  This is LibZeroEvil & the Research Rootkit project, in which there are step-by-step, experiment-based courses that help to get you started and keep your hands dirty with offensive or defensive development in the Linux kernel (LibZeroEvil).
  
  😆 [**Doing**](kernel_mode/research_rootkit/research_rootkit.md) 

- [GitHub - NinnOgTonic/Out-of-Sight-Out-of-Mind-Rootkit: Rootkit](https://github.com/NinnOgTonic/Out-of-Sight-Out-of-Mind-Rootkit) ⚡ [writeup](https://github.com/NinnOgTonic/Out-of-Sight-Out-of-Mind-Rootkit/blob/master/osom.pdf) ⚡
  
  Out of Sight, Out of Mind is a study and implementation of Linux rootkit methods. In addition a new covert network channel using additional Domain Name System (DNS) is implemented.

- [GitHub - h3xduck/Umbra: A LKM rootkit targeting 4.x and 5.x kernel versions which opens a backdoor that can spawn a reverse shell to a remote host, launch malware and more.](https://github.com/h3xduck/Umbra)
  
  An experimental LKM rootkit for v4.x/5.x kernels which opens a backdoor that can be used to get a reverse shell remotely.

- https://github.com/milabs/kopycat
  
  KOPYCAT - Linux Kernel module-less implant (backdoor).

- https://github.com/carloslack/KoviD
  
  Linux 4.18+ rootkit with multiple reverse backdoors, task management, CPU usage hiding, stealth techniques, ELF infection and evasion from anti-rooktiks based on eBPF.

- [GitHub - reveng007/reveng_rtkit: Linux Loadable Kernel Module (LKM) based rootkit (ring-0), capable of hiding itself, processes/implants, rmmod proof, has ability to bypass infamous rkhunter antirootkit.](https://github.com/reveng007/reveng_rtkit)
  
  Linux Loadable Kernel Module (LKM) based rootkit capable of hiding itself, processes/implants, rmmod proof, has ability to bypass infamous rkhunter antirootkit.

### 🙊 eBPF rootkit

* [GitHub - kris-nova/boopkit: Linux eBPF backdoor over TCP. Spawn reverse shells, RCE, on prior privileged access. Less Honkin, More Tonkin.](https://github.com/kris-nova/boopkit)
  
  Linux backdoor, rootkit, and eBPF bypass tools. Remote command execution over raw TCP.

* [GitHub - h3xduck/TripleCross: A Linux eBPF rootkit with a backdoor, C2, library injection, execution hijacking, persistence and stealth capabilities.](https://github.com/h3xduck/TripleCross) 
  
  A Linux eBPF rootkit with a backdoor, C2, library injection, execution hijacking, persistence and stealth capabilities.

* [GitHub - carloslack/KoviD: LKM Rootkit](https://github.com/carloslack/KoviD)
  
  Linux 4.18+ rootkit with multiple reverse backdoors, task management, CPU usage hiding, stealth techniques, ELF infection and evasion from anti-rooktiks based on eBPF.

* [GitHub - Gui774ume/ebpfkit: ebpfkit is a rootkit powered by eBPF](https://github.com/Gui774ume/ebpfkit)

### Ftrace

* [GitHub - h3xduck/Umbra: A LKM rootkit targeting 4.x and 5.x kernel versions which opens a backdoor that can spawn a reverse shell to a remote host, launch malware and more.](https://github.com/h3xduck/Umbra)
