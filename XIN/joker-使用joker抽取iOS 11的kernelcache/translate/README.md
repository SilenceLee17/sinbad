# 关于Joker

joker是一个iOS kernelcache处理工具。Apple尽可能的使kernel的逆向变得困难：每一个新版本发布时，都会有更多的符号被移除。预链接的kernelcache需要少量的符号来开始，而且kernelcache是被加密的。在@xerub和其他人的努力下，32位的kernelcache可以被解密，但是64位的kernelcache由于没有公开的key，唯一的方式就是dump kernelcache。

这些困难并没有阻止越狱者，而且以后也不会。Joker是我向社区和逆向者提供的另一个贡献，它使用与jtool相同的machlib，实际上他们可以很好的一起使用。

joker主要是为iOS开发的，但由于iOS与OS X的XNU数据结果几乎相同，它也可以通过一些参数（-m,-s）对OS X kernel进行处理。

**译者注：** iOS10以后kernelcache不再被加密

#用法示例

### 基础
joker会显示一些帮助信息
```
Zephyr:JTool morpheus$ joker
Usage: joker [-j] [-MmaSsKk] _filename_
 _filename_ should be a decrypted iOS kernelcache, or kernel dump.

 -m: dump Mach Traps and MIG tables (NEW)
 -a: dump everything
 -k: dump kexts
 -K: kextract [kext_bundle_id_or_name_shown_in_-k|all]
 -S: dump sysctls
 -s: dump UNIX syscalls
 -j: Jtool compatible output (to companion file)

2.2.1 (w/64 kextraction, MIG) Stable version (and kextraction on 64-bit!)
 Tested on ARMv7/s 3.x-9.21b1, and ARM64 8.x/9.21b1
 ```
 -m: 导出Mach陷阱与MIG表

 -a: 导出所有信息

 -k: 导出kext信息

 -K: 导出特定或全部kext bundle

 -S: 导出sysctl

 -s: 导出UNIX系统调用

 -j: 与jtool协作使用

 不使用参数，joker可以通过LC_SOURCE_VERSION为你提供文件的简要标示和sysent表地址
 ```
Zephyr:JTool morpheus$ joker ~/Documents/iOS/9b/kernel.9b.4S.decrypted
Source Version:          3216.0.0.1.15
This is iOS 9.x, or later
Found iOS 8+ sysent table @3eb684 (Addr: 0x803ec684)
 ```
### 32位

由于在LINKEDIT和PRELINK_INFO被移除之前，可以完全获取32位kernelcache，joker可以同时列出和提取kexts bundle：

```
Zephyr:JTool morpheus$ joker -k ~/Documents/iOS/9b/kernel.9b.4S.decrypted | more
Source Version:          3216.0.0.1.15
This is iOS 9.x, or later
Found iOS 8+ sysent table @3eb684 (Addr: 0x803ec684)
Processing kexts
OFFSET: 1095000
0x804ba000: Mach Kernel Pseudoextension (com.apple.kpi.mach)
0x804bc000: Unsupported Pseudoextension (com.apple.kpi.unsupported)
0x804bf000: Private Pseudoextension (com.apple.kpi.private)
0x804c5000: I/O Kit Pseudoextension (com.apple.kpi.iokit)
0x804df000: Libkern Pseudoextension (com.apple.kpi.libkern)
..
0x810d2000: tlsnke (com.apple.nke.tls)
0x810da000: AppleDiagnosticDataAccessReadOnly (com.apple.driver.AppleDiagnosticDataAccessReadOnly)
0x810dd000: AppleA5IOP (com.apple.driver.AppleA5IOP)
Got 171 kexts
```

只需要指定kext名称就可以提取对应的kext bundle。

```
# BackEndBilly from the forum noted that extraction by name fails on "/" in name,
# (and messy with lots of spaces) so this is now easier, by bundle ID, as of v2.1.1:
bash-3.2# ./joker -k ~/Documents/iOS/9b/kernel.9b.4S.decrypted | grep MobileFil
0x80728000: AppleMobileFileIntegrity (com.apple.driver.AppleMobileFileIntegrity)
bash-3.2# ./joker -K com.apple.driver.AppleMobileFileIntegrity  ~/Documents/iOS/9b/kernel.9b.4S.decrypted
This is a 32-bit kernel from iOS 9.x, or later (3216.0.0.1.15)
Found iOS 8+ sysent table @3eb684 (Addr: 0x803ec684)
Processing kexts
Attempting to kextract com.apple.driver.AppleMobileFileIntegrity
Found com.apple.driver.AppleMobileFileIntegrity at load address: 80728000, offset: 6dd000
Extracted com.apple.driver.AppleMobileFileIntegrity
bash-3.2# file com.apple.driver.AppleMobileFileIntegrity
com.apple.driver.AppleMobileFileIntegrity: Mach-O kext bundle arm
```
### 32位&64位


使用-S选项可以获取到完整的__sysctl_set的dump。这个很有用，因为sysctl描述符包含很多有用的内核地址、变量、处理程序：

**译者注：** 导出sysctl应使用-S选项，而原文使用的是-s，译文已更正

```
# especially designed to be grep(1) friendly - you can get your sysctl + 6 lines
Zephyr:JTool morpheus$ joker -S ~/Documents/iOS/9b/kernel.9b.4S.decrypted |
			 	grep -A 6 vnode_enfor
0x80411fc0: security.mac.vnode_enforce	Description: Enforce MAC policy on vnode operations
		Handler: 0x802b5171
		Format: IU
		Parent: 0x80414b90
		Arg1: 80406028
		Arg2: 0x0
```

-S 同时适用于OS X。

最有效的符号化的参数是"-m"，它会符号化Mach陷阱和UN*X系统调用。joker会自动压缩enosys和旧的系统调用，但仍然会提供对应的地址。

**译者注：** [enosys](https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html) 为UNIX的一种系统错误名称 

```
Zephyr:JTool morpheus$ joker -a ~/Documents/iOS/9b/kernel.9b.4S.decrypted  | more
Source Version:          3216.0.0.1.15
This is iOS 9.x, or later
ARM Exception Vector is at file offset @0xc6000 (Addr: 0x800c7000)
Found iOS 8+ sysent table @3eb684 (Addr: 0x803ec684)
Syscalls at address 0x803ec684
Suppressing enosys (0xffffffff802c14bd) and old (0xffffffff802c14cd)
1. exit                 802a0f48 T
2. fork                 802a5734 T
3. read                 802c14e8 T
4. write                802c1924 T
5. open                 800f5344 T
6. close                80290e68 T
7. wait4                802a288c T
9. link                 800f5c70 T
10. unlink               800f5d24 T
..
490. netagent_trigger     80152f38 T
491. stack_snapshot_with_config 80287fc4 T
493. grab_pgo_data        80303030 T
Zephyr:JTool morpheus$$ joker -m -m -d  ~/Documents/iOS/9b/13A4254v.bin | more
This is a 64-bit Kernel
Source Version:          3216.0.0.1.15
This is iOS 9.x, or later
Syscalls at address 0xffffff8005ef60c0
Sysent offset in file (for patching purposes):  4f20c0
Suppressing enosys (0xffffff8005d73640) and old (0xffffff8005d73620)
1.. exit                 0xffffff8005d49f5c
2.. fork                 0xffffff8005d4f338
3.. read                 0xffffff8005d73668
..
489.. mremap_encrypted     0xffffff8005d5111c
490.. netagent_trigger     0xffffff8005bac3f4
491.. stack_snapshot_with_config 0xffffff8005d29a98
493.. grab_pgo_data        0xffffff8005dc91b0
# Unexpected bonus - also works on OS X kernels :-)
Zephyr:JTool morpheus$ joker -m /System/Library/Kernels/kernel  | grep mmap
This is a 64-bit Kernel
# Albeit not as useful since most symbols are still exported (and there's the KDK)
197.. mmap                 0xffffff80007c8990
Zephyr:JTool morpheus$ jtool -S /System/Library/Kernels/kernel | grep mmap | grep 8990
ffffff80007c8990 T _mmap
```
### 与jtool协作

joker最新和最强大的特性是对JTool的支持。使用joker -j 来生成一个简单的规则文本文件（0xaddress:symbol:comment），在JTool中可以直接调用这个文件。

```
Zephyr:JTool morpheus$ joker  -j -m ~/Documents/iOS/9b/13A4254v.bin  
This is a 64-bit Kernel
Source Version:          3216.0.0.1.15
This is iOS 9.x, or later
Opened companion File: /Users/morpheus/Documents/iOS/9b/13A4254v.bin.ARM64.397D09B8-4B9B-3771-ACEE-7143647BD873
Opening companion file
Syscalls at address 0xffffff8005ef60c0
Sysent offset in file (for patching purposes):  4f20c0
Suppressing enosys (0xffffff8005d73640) and old (0xffffff8005d73620)
Output written to /Users/morpheus/Documents/iOS/9b/13A4254v.bin.ARM64.397D09B8-4B9B-3771-ACEE-7143647BD873 in Jtool-compatible format. Run jtool with --jtooldir . or set JTOOLDIR=
# File format is crude, I'll admit, but it makes it all the more easier for manual editing
Zephyr:JTool morpheus$ head -5 /Users/morpheus/Documents/iOS/9b/13A4254v.bin.ARM64.397D09B8-4B9B-3771-ACEE-7143647BD873
0xffffff8005d49f5c:exit
0xffffff8005d4f338:fork
0xffffff8005d73668:read
0xffffff8005d73d40:write
0xffffff8005b33e44:open
# We can now use jtool, since the companion file was created for us - and use symbols from it as we disassemble!
Zephyr:JTool morpheus$ jtool -d stack_snapshot_with_config ~/Documents/iOS/9b/13A4254v.bin
Opened companion File: 13A4254v.bin.ARM64.397D09B8-4B9B-3771-ACEE-7143647BD873
Disassembling from file offset 0x325a98, Address 0xffffff8005d29a98  to next symbol
stack_snapshot_with_config:
ffffff8005d29a98        STP    X22, X21, [SP,#-48]!
ffffff8005d29a9c        STP    X20, X19, [SP,#16]
ffffff8005d29aa0        STP    X29, X30, [SP,#32]
ffffff8005d29aa4        ADD    X29, SP, #32; R29 = SP + 0x20
ffffff8005d29aa8        SUB    X31, X31, #64
ffffff8005d29aac        MOV    X19, X1
ffffff8005d29ab0        MOV    X20, X0
ffffff8005d29ab4        BL     _kauth_cred_get  ; 0xffffff8005d2f300
ffffff8005d29ab8        MOV    X21, X0
ffffff8005d29abc        ADD    X8, X21, #1; ..R8 = R21 (0x0) + 0x1 = 0x1
ffffff8005d29ac0        CMP    X8, #1
ffffff8005d29ac4        B.HI   0xffffff8005d29ae0       ; 0xffffff8005d29ae0
ffffff8005d29ac8        ADRP   X8, 390; ->R8 = 0xffffff8005eaf000
ffffff8005d29acc        ADD    X8, X8, #1478; ..R8 = R8 (0xffffff8005eaf000) + 0x5c6 = 0xffffff8005eaf5c6 "kauth_cred_getuid"
ffffff8005d29ad0        STP    X8, X21, [SP,#0]
ffffff8005d29ad4        ADRP   x0, 390; ->R0 = 0xffffff8005eaf000
ffffff8005d29ad8        ADD    X0, X0, #1454; ..R0 = R0 (0xffffff8005eaf000) + 0x5ae = 0xffffff8005eaf5ae ""%s: bad credential %p""
ffffff8005d29adc        BL     _panic   ; 0xffffff8005a2b960
; _panic("%s: bad credential %p", arg2, arg3);
...
```
### Mach Traps

### MIG Tables
Mach使用消息、MIG来序列化消息。 Mach Interface Generator生成解析消息所需要的样本代码，这就会留下容易识别的特征。joker现在可以识别dispatch表，定位符号化的函数。

注意这些符号化的函数是一种代理，真正执行的函数是通过他们来进行调用的。但是我们可以很容易的找出真正执行的函数，因为基本上代理都会去调用消息检查函数。

这对iOS很有用，但是所有的符号都是被去掉了。在OS X xnu中可以更容易的看到被调用的真实函数，因为在OS X xnu函数都是被导出的，

**译者注：** 样本代码 = [boilerplate code](https://en.wikipedia.org/wiki/Boilerplate_code) 

```
host_priv_subsystem is  @0xffffff8000a02a90!
	0: host_get_boot_info_mig_wrapper: ffffff800032ea60 (400)
	..
_host_subsystem is  @0xffffff8000a030d8!
	0: host_info_mig_wrapper: ffffff8000332300 (200)
	19: mach_vm_page_info_mig_wrapper: ffffff8000338cd0 (4819)
..# Note joker can detected if Apple updates their defs
	Warning: This kernel is newer than joker is (224 < 227)!
mach_port_subsystem is  @0xffffff8000a03558!
	0: mach_port_names_mig_wrapper: ffffff80003346b0 (3200)
task_subsystem is  @0xffffff8000a04728!
	0: task_create_mig_wrapper: ffffff800033fec0 (3400)
	..
	41: task_swap_mach_voucher_mig_wrapper: ffffff8000342c50 (3441)
thread_act_subsystem is  @0xffffff8000a04dd8!
	0: thread_terminate_mig_wrapper: ffffff8000342f30 (3600)
	26: thread_set_mach_voucher_mig_wrapper: ffffff8000344d40 (3626)
	27: thread_swap_mach_voucher_mig_wrapper: ffffff8000344ee0 (3627)

# Unfortunately, jtool doesn't do x86, so use otool:
$ otool -tV /System//Library/Kernels/kernel.3247 |grep -A 100 ffffff8000344ee0  
ffffff8000344ee0        pushq   %rbp
ffffff8000344ee1        movq    %rsp, %rbp
ffffff8000344ee4        pushq   %r15
ffffff8000344ee6        pushq   %r14
ffffff8000344ee8        pushq   %r13
ffffff8000344eea        pushq   %r12
...
ffffff8000344ee0        pushq   %rbp
ffffff8000344ee1        movq    %rsp, %rbp
ffffff8000344ee4        pushq   %r15
..
ffffff8000344fe3        movq    %r12, %rsi
ffffff8000344fe6        callq   _thread_swap_mach_voucher
```
再次注意，我在这里使用x86_64就是为了说明这点，你可以看到内部被调用的真实函数是_thread_swap_mach_voucher，这表示joker得到了当前的MIG调度内容。在iOS上，你可以通过交叉编译ARM64和x86_64来达到这个目的。


### Version 3.0β1!

随着iOS10和OS X 12的发布，内核段布局被修改，是时候更新joker了。v3.0得到大幅度修改，通过与我的machlib集成，可以使用joker进行反汇编来找到函数。目前我已经实现了简单的功能（functions taking in strings），但可以被扩展到复杂情况和智能匹配模式。

由于Apple提供了不加密的kernelcache，你可能想知道joker没有被放弃的原因。

* kexts仍然被严密的集成，因此你需要-k/-K参数
* Mach陷阱&MIG，系统调用和sysctls没有被导出，因此你需要-m，-s和-S。我更新了系统调用表，包含全部520个系统调用和大部分的MIG。
* 许多其他有趣的符号没有被导出，XNU有O（20k）符号，但是iOS XNU仅导出了O（4k）。

反编译并不是你想要的最有用的功能，而且它有点慢，因此尝试使用-j，然后你会看到：

```
morpheus@Zephyr (~/Documents/iOS/10) % joker -j xnu.3757.j99a                                                 13:00
This is a 64-bit kernel from iOS 10.x, or later (3757.0.0.0.0)
Opened companion File: xnu.3757.j99a.ARM64.9612A9D5-FA9B-3663-ADD0-C1852BE6674B
Opening companion file
Got 4498 syms
Looking for _secure_monitor ...  Found _secure_monitor at offset 0x7aec
Looking for _start_cpu ...  Found _start_cpu at offset 0x7018
Auto-Disassembling __TEXT_EXEC.__text from 0xfffffff00747c000 to find rest
This may take a little while, but you only need to do this once
Disassembling from file offset 0x78000, Address 0xfffffff00747c000
GOT PE_Parse_boot_argn: 0xfffffff0078db018
GOT lck_grp_alloc_init: 0xfffffff0074ac1b4
GOT OSKextLog: 0xfffffff00783bf48
GOT lck_grp_init: 0xfffffff0074ac1b4
GOT PE_get_default: 0xfffffff0078db608
GOT __ZN9IOService15publishResourceEPKcP8OSObject: 0xfffffff007874e88
GOT __ZN8OSSymbol17withCStringNoCopyEPKc: 0xfffffff007849c28
GOT __ZN9IOService15publishResourceEPKcP8OSObject: 0xfffffff007849c28
GOT __ZN9IOService15publishResourceEPKcP8OSObject: 0xfffffff007874e88
GOT IOLog! 0xfffffff00785b5ec
ARM Exception Vector is at file offset @0x7b000 (Addr: 0xfffffff00747f000)
Output written to xnu.3757.j99a.ARM64.9612A9D5-FA9B-3663-ADD0-C1852BE6674B in Jtool-compatible format.
Run jtool with --jtooldir . or set JTOOLDIR=
# Note that now function starts are also auto imported into companion file..
morpheus@Zephyr (~/Documents/iOS/10) % wc -l xnu.3757.j99a.ARM64.9612A9D5-FA9B-3663-ADD0-C1852BE6674B         13:03
   10791 xnu.3757.j99a.ARM64.9612A9D5-FA9B-3663-ADD0-C1852BE6674B
# All these go into companion file, of course, and you can verify those which ARE exported, like IOLog:
morpheus@Zephyr (~/Documents/iOS/10) % jtool -S xnu.3757.j99a| grep IOLog                                     13:04
Opened companion File: ./xnu.3757.j99a.ARM64.9612A9D5-FA9B-3663-ADD0-C1852BE6674B
Loading symbols...
fffffff00785b5ec T _IOLog # Matches!
fffffff00785b680 T _IOLogv
```
在有些情况下，joker比实际的符号表更准确，因为其导出的符号是真实实现的一个包装，能够经常被使用到。

joker3.0也做了一些只适用于sandbox.kext的MACF策略，AMFI现在已经从代码中加载了。这可以在sandbox 570中得到新的hooks。如下：

```
Zephyr:Sierra morpheus$ joker /tmp/161.com.apple.security.sandbox.kext
Found policy at 0xfffffff0055a05d0
	Policy name: Sandbox
	Full name of policy: Seatbelt sandbox policy
	Flags: 0
	Ops: fffffff0055a0620
		mpo_cred_check_label_update_execve: 0xfffffff00552a34c
		..
                mpo_mount_check_snapshot_create: 0xfffffff005520970
		mpo_check_snapshot_delete: 0xfffffff005520a08
		mpo_vnode_check_clone: 0xfffffff005520aa0
		mpo_proc_check_get_cs_info: 0xfffffff005520c68 # The Hedgehog hook :-)
		mpo_proc_check_set_cs_info: 0xfffffff005520d1c
		mpo_iokit_check_hid_control: 0xfffffff005520d98
..
com.apple.security.sandbox(570.0.0.0.0)(570.0.0.0.0)
```
V3.0的其他增强是JTOOLDIR=...和几个令人兴奋的功能（如IOUserClients的自动检测），但是直到最终版本才会出现，第二个Beta版本会增加安全监视器（SMC）检测和ARM异常向量

### Version 3.0β2!

* 现在可以通过#1和#2两种方法来获取到所有的内核扩展
* 修复ID=... bug
* joker现在可以在kexts中自动符号化桩（stubs），大部分的桩指向内核中的符号，感谢APPL提供这些导出。joker可以自动处理这些桩的反汇编。

```
fffffff00552fa64        ADRP   X16, 113                 ; ->R16 = 0xfffffff0055a0000
fffffff00552fa68        LDR    X16, [X16, #0]   ; -R16 = *(R16 + 0) =  *(0xfffffff0055a0000) = 0xfffffff007843ae4 ... ?..
fffffff00552fa6c        BR     X16                              ;  0xfffffff007843ae4
```

正如你所看到的，machlib可以跟踪寄存器，找出地址。因此，只要是X16导出的符号，你都可以快速的找到它。

```
morpheus@Zephyr (~)$ jtool -S ~/Documents/iOS/10/xnu.3705.j99a | grep fffffff007843ae4
fffffff007843ae4 T _IOFree
```

你为什么还要自己做这些呢？？joker现在可以自动完成这些

```
morpheus@Zephyr (~)$ joker -K com.apple.security.sandbox ~/Documents/iOS/10/xnu.3705.j99a
This is a 64-bit kernel from iOS 10.x, or later (3705.0.0.2.3)
ARM Exception Vector is at file offset @0x7b000 (Addr: 0xfffffff00747f000)
Found com.apple.security.sandbox at load address: 551c000, offset: 1a8c000
Writing kext out to /tmp/com.apple.security.sandbox.kext
Symbolicating stubs for com.apple.security.sandbox  from off 0x13a64
# Note that there are still symbols that are not exported by XNU, but ARE linked from stubs
Unable to resolve kernel symbol at fffffff0040e4e84
..
Symbolicated stubs to /tmp/com.apple.security.sandbox.kext.ARM64.00066DE6-8BDC-36E0-90CD-A872522D821
morpheus@Zephyr (~)$ cat  /tmp/com.apple.security.sandbox.kext.ARM64.00066DE6-8BDC-36E0-90CD-A872522D8211
fffffff00552fa64:_IOFree.stub
fffffff00552fa70:_IOMalloc.stub
..
fffffff00552faac:_PE_i_can_has_debugger.stub
...
fffffff00552fff8:_sock_gettype.stub
morpheus@Zephyr (~)$ jtool -d /tmp/com.apple.security.sandbox.kext | grep "^; _panic"
Opened companion File: /tmp/com.apple.security.sandbox.kext.ARM64.00066DE6-8BDC-36E0-90CD-A872522D8211
Loading symbols...
Disassembling from file offset 0x770, Address 0xfffffff00551c770
; _panic.stub("released collection's reference to profile");
; _panic.stub("cannot register builtin profile with no name");
; _panic.stub("failed to initialize platform sandbox");
; _panic.stub("unexpected filter type");
; _panic.stub("sfree on invalid allocation at %p"0xfe);
; _panic.stub("no profile to evaluate");
; _panic.stub("%s: matchlen (%zu) > pathlen (%zu)\n""(null)");
; _panic.stub("%s: failed to allocating matching dict""(null)");
; _panic.stub("%s: failed to lookup AMFI service\n""(null)");
; _panic.stub("failed to allocate OSSymbol for string \"%s\"""(null)");
```

### Version 3.0β3!

一些次要的但有用的改动：如果你提供给joker一个未压缩（未加密）的kernelcache，joker依然会抱怨，但是会在最后把KPP Mach-O 提取到/tmp中。

### Version 3.0β3!

* kexts分离（iOS 10b2+）

### Version 3.1

* 我使用Apple旧iBoot中的LZSS.c来处理压缩的kernelcache。同时ARM64版本的joker回来了，你可以直接在64位设备上使用它。

### Version 3.2

Joker gets in the zone - Mach zones, that is:

```
morpheus@Zephyr (~/Documents/Work/JTool) % ./joker -j ~/Downloads/kernelcache.release.n66
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 12537772, Uncompressed: 24854528. Unknown (CRC?): 0x95944450, Unknown 1: 0x1
btw, KPP is at 12538208 (0xbf5160)..And I saved it for you in /tmp/kpp
Got kernel at 437
This is a 64-bit kernel from iOS 10.x (b7+), or later (3789.32.1.0.0)
Opened companion File: /Users/morpheus/Downloads/kernelcache.release.n66.ARM64.977182E2-2525-386E-8808-457275346A3E
Opening companion file
Found _secure_monitor at offset 0x7b0c, Addr: 0xfffffff007087b0c
Found _start_cpu at offset 0x7018, Addr: 0xfffffff007087018
Auto-Disassembling __TEXT_EXEC.__text from 0xfffffff007080000 to find rest..
This may take a little while, but you only need to do this once
Disassembling from file offset 0x7c000, Address 0xfffffff007080000
GOT zinit: 0xfffffff0070e5700
GOT PE_Parse_boot_argn: 0xfffffff0074e5a40
GOT zp_factor: 0xfffffff007556340
GOT PE_Parse_boot_argn: 0xfffffff0074e5a40
GOT zp_scale: 0xfffffff007556344
fffffff007585568:vm objects zone
fffffff0075711f0:vm object hash entries zone
fffffff007570cd0:maps zone
fffffff007570b10:Reserved VM map entries zone
fffffff007570cc0:VM map copies zone
fffffff007570b18:VM map holes zone
fffffff00758ad60:pmap zone
fffffff007588bb0:vm pages array zone
fffffff007588bb8:vm pages zone
GOT lck_grp_alloc_init: 0xfffffff0070b23a8
fffffff007570a90:mem_obj_control zone
fffffff007570a70:device node pager structures zone
fffffff007556338:waitq sets zone
fffffff007537300:ipc spaces zone
fffffff0075372d0:ipc ports zone
fffffff0075372d8:ipc port sets zone
fffffff0075372c0:ipc kmsgs zone
fffffff007538428:ipc vouchers zone
fffffff007538420:ipc voucher attr controls zone
fffffff007537150:ipc task importance zone
... you get the idea
```
# Version 3.99

不是4.0。因为USerClients还不能工作。同时更新对iOS 11 β的支持，sandbox decomp出现了问题(待修复)。

```
morpheus@Chimera (~/Documents/Work/JTool) %./joker.universal -s ~/Downloads/kernelcache.release.ipad4
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 12693336, Uncompressed: 24969216. Unknown (CRC?): 0xc809f8d1, Unknown 1: 0x1
btw, KPP is at 12693775 (0xc1b10f)..And I saved it for you in /tmp/kpp
Got kernel at 440
This is a 64-bit kernel from iOS 11.x (b1+), or later (4397.0.0.2.4)
#
# Note also - as apparently two new Mach traps
# KPP updated (haven't checked yet, so not sure it will defeat Luca's bypass, but nice AAPL are trying)
#
...
186.. thread_selfcounts    0xfffffff0071a6c7c
..
375.. kevent_id            0xfffffff0073cb0d0
...
520.. terminate_with_payload 0xfffffff0073efdac
521.. abort_with_payload   0xfffffff0073d9410
522.. necp_session_open    0xfffffff0072465e8
523.. necp_session_actrion 0xfffffff00724c538
524.. setattrlistat        0xfffffff0071ae720
525.. net_qos_guideline    0xfffffff0072d85c4
526.. fmount               0xfffffff0071cf0a0
527.. ntp_adjtime          0xfffffff007466ba4
528.. ntp_gettime          0xfffffff007466a30
529.. os_fault_with_payload 0xfffffff0073d9468
```



## 译者注
* 为方便理解，本文部分内容并未按照joker原文进行逐句翻译；
* 鉴于翻译水平有限，本文可能会存在翻译不准确的地方，如发现，请及时指正，不胜感激。

## 原文链接
[joker](http://newosxbook.com/tools/joker.html)
