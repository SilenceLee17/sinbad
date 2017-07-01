# Tick (FPU) Tock (IRQ)
与iOS9一起推出的内核完整性保护又称为“KPP”，这对于arm64越狱带来了新的问题。 有效地观察到，在修补内核代码（通常在较旧的越狱中）之后，设备会一段时间后会静默地恐慌。 很明显，有些东西会检查内核代码。 这个东西在内核之外，很快就显现出一种在工作中的管理程序。

到目前为止，我仍然感到惊讶的是，没有人对KPP进行了一个写作 - 至少没有人知道。 所以，我将尝试解释hypervision如何工作。 具体的实施细节可能会在稍后的时间提供，只要我有时间。现在，不用多说，我们走

## 第1部分（设置）
KPP位于一个Mach-O可执行文件，紧随压缩内核块之后，在kernelcache img4内。 iBoot刻录出KPP图像，将其加载到0x4100000000，并在EL3中运行

### EL3
`_start(monitor_boot_args *mba)`

这结构体有如下几项

```
struct monitor_boot_args {
    uint64_t version;
    uint64_t virtBase;
    uint64_t physBase;
    uint64_t memSize;
    struct kernel_boot_args *kernArgs;
    uint64_t kernEntry;
    uint64_t kernPhysBase;
    uint64_t kernPhysSlide;
    uint64_t kernVirtSlide;
};

```
KPP使用调用`_start（NULL）`的蹦床来覆盖自己的`Mach-O`头，`0x4100000000`，并安装了两个异常处理程序`sync_handler和irq_handler`。 回想一下AArch64异常表：

|VBAR_ELN|异常类型|描述|
|:-:|:-:|:-:|
|+0x000|Synchronous|Current EL with SP0|
|+0x080|IRQ/vIRQ||
|+0x100|FIQ/vFIQ||
|+0x180|SError/vSError||
|+0x200|Synchronous|Current EL with SPx|
|+0x280|IRQ/vIRQ||
|+0x300|FIQ/vFIQ||
|+0x380|SError/vSError||
|+0x400|Synchronous|Lower EL using AArch64|
|+0x480|IRQ/vIRQ||
|+0x500|FIQ/vFIQ||
|+0x580|SError/vSError||
|+0x600|Synchronous|Lower EL using AArch32|
|+0x680|IRQ/vIRQ||
|+0x700|FIQ/vFIQ||
|+0x780|SError/vSError||

ExceptionVector具有两个处理程序的位置

接下来，它解析内核及其kexts（来自`__PRELINK_INFO`）

* save __TEXT, __DATA segments to map list
* save __TEXT, __DATA::__const zones to hash list

最后，如果启用 – which  – it（以及其他）以下寄存器

```
CPACR_EL1 = 0x100000;  // CPACR_EL1.FPEN=1, causes instructions in EL0 that use the Floating Point execution to be trapped
CPTR_EL3 = 0x80000000; // CPTR_EL3.TCPAC=1, accesses to CPACR_EL1 will trap from EL2 and EL1 to EL3
SCR_EL3 = 0x631;       // SCR_EL3.IRQ=0, When executing at any Exception level, physical IRQ interrupts are NOT taken to EL3
                       // SCR_EL3.SMD=0, SMC instructions are ENABLED at EL1 and above
                       // SCR_EL3.SIF=1, Secure state instruction fetches from Non-secure memory are NOT permitted
```
### EL1

内核在EL1中开始执行
```
_start() => start_first_cpu() => arm_init():
    => cpu_machine_idle_init() => monitor_call(0x800)
    => machine_startup() => kernel_bootstrap() => kernel_bootstrap_thread() => monitor_call(0x801)
_start_cpu() => arm_init_cpu() => cpu_machine_idle_init() => monitor_call(0x800)

```
`monitor_call（）`将升级到EL3到管理程序的`sync_handler`
### EL3

`sync_handler`:
```
if (ESR_EL3 == 0x5E000011) { // ESR_EL3.EC==0x17 && ESR_EL3.IL==1 && ESR_EL3.ISS==0x11 aka "SMC #0x11" aka monitor_call() inside the kernel
    switch (arg0) {
        case 0x800: // called by cpu_machine_idle_init()
            /* save kernel entrypoint */
            return ok;
        case 0x801: // called by kernel_bootstrap_thread()
            if (enabled) {
                if (locked) {
                    FAIL(4);
                }
                /* do lockdown:
                 * hash all regions from hash list
                 * initialize some vars
                 * save SCTLR_EL1, TCR_EL1, TTBR1_EL1, VBAR_EL1
                 */
                ...
                SCR_EL3.SMD=1;
                locked = 1;
            }
            return OK;
        case 0x802: // wtf is this shit?
            FAIL(5);
    }
}
(to be continued)

```
当出现问题时，FAIL（代码）设置一个全局变量，并通过以下方式向内核发出信号：
```
ESR_EL1 = 0xBF575400 | code // ESR_EL1.EC=0x2F, ESR_EL1.ISV=1, ESR_EL1.IS=0x575400|code
```

代码含义
1. 帧违规
2. 坏系统调用
3. 未锁定
4. 已锁定
5. 软件请求
6. TTE / PTE无效
7. 违反绘图
8. 违反系统注册

然后，执行再转移到SError回到内核的ExceptionTable中：

```
SError => fleh_serror() => sleh_serror() => kernel_integrity_error_handler() => panic()

```

否则，如果一切都会好的，就在monitor_call（）之后，执行在内核中恢复。
这是设置阶段，为了让内核设置一次写入内存位置所需的。 接下来，进入心跳阶段

## 第二部分 (the ticking)
同时在用户空间运行代码，当FPU指令被执行后，`CPACR_EL1.FPEN==1`会产生一个内核陷阱。

### EL1
在内核中，`fleh_synchronous()` `fleh_irq()` `fleh_fiq()` 和 `fleh_serror()` 所有都会这样结束  
`exception_return_dispatch() => check_user_asts() => MSR CPACR_EL1, X0`
一旦来自于EL3内核陷阱，CPACR_EL1就会执行。当`CPTR_EL3.TCPAC==1`时，执行权就转交到EL3 `sync_handler`  
### EL3
This time, it means business.  
sync_handler:

```
(continued)
else if (ESR_EL3 == 0x62340400) { // ESR_EL3.EC==0x18 && ESR_EL3.IL==1 && ESR_EL3.ISS==0x340400 aka trapped by "MSR CPACR_EL1, X0"
    if (violated) {
        FAIL(1);
    }
    if (!locked) {
        FAIL(3);
    }
    if (!(++number_of_hits & watchtower_throttle)) {
        if (!(++flip_flop & 1)) {
            if (hash_is_ready) {
                blake2b_final(&hash, digest);
                if (memcmp(cur->digest, digest, 32)) {
                    FAIL(1);
                }
                cur = get_next_region();
                if (!cur) {
                    cur = get_first_region();
                }
                cur_data_ptr = cur->base;
                cur_data_left = cur->size;
                blake2b_init(&hash);
                hash_is_ready = 0;
            } else {
                chunk = min(cur_data_left, 128);
                blake2b_update(&hash, cur_data_ptr, chunk);
                cur_data_ptr += chunk;
                cur_data_left -= chunk;
                if (!cur_data_left) {
                    hash_is_ready = 1;
                }
            }
        } else {
            /* walk and check TTE/PTE
             * verify map list
             * check system registers SCTLR_EL1, TCR_EL1, TTBR1_EL1, VBAR_EL1
             */
            ...
        }
    }
    ELR_EL3 += 4;         // skip insn
    CPTR_EL3 = 0;         // CPTR_EL3.TCPAC=0, accesses to CPACR_EL1 will not trap from EL2 and EL1 to EL3
    CPACR_EL1 = 0x300000; // CPACR_EL1.FPEN=3, does not cause any FPU instruction to be trapped
    SCR_EL3 = 0x6B3;      // SCR_EL3.IRQ=1, When executing at any Exception level, physical IRQ interrupts are taken to EL3
                          // SCR_EL3.SMD=1, SMC instructions are UNDEFINED at EL1 and above
                          // SCR_EL3.SIF=1, Secure state instruction fetches from Non-secure memory are NOT permitted
    return OK;
}
```
如果所有的检查都通过的话，管理程序就会禁用FPU陷阱（允许FPU最终执行）使得IRQ执行到EL3(确保再次命中)，在CPACR_EL1命中后恢复内核然后等待。
### EL1
内核和用户空间愉快的运行着。当下一次IRQ激起，就由EL3 IRQ的管理程序接管。
### EL3
irq_handler:

```
CPACR_EL1 = 0x100000;  // CPACR_EL1.FPEN=1, causes instructions in EL0 that use the Floating Point execution to be trapped
CPTR_EL3 = 0x80000000; // CPTR_EL3.TCPAC=1, accesses to CPACR_EL1 will trap from EL2 and EL1 to EL3
SCR_EL3 = 0x431;       // SCR_EL3.IRQ=0, When executing at any Exception level, physical IRQ interrupts are NOT taken to EL3
                       // SCR_EL3.SMD=0, SMC instructions are ENABLED at EL1 and above
                       // SCR_EL3.SIF=0, Secure state instruction fetches from Non-secure memory are permitted
```

也就是说：重置IRQs到EL1，使其重新能够FPU 陷阱，能够处理CPACR_EL1访问的陷阱，最后的4步然后一直重复。

### ***
总结：KPP确保FPU陷阱且不会被禁用。当FPU命中，内核就会尝试禁用陷阱但同时也会由KPP接管。KPP然后运行检查，释放FPU,运行IRQs本身。只要任何IRQ触发，就会使FPU进入内核陷阱并结束IRQs  

这是保持管理程序跳动的引擎。 如果你修改触发器，即CPACR_EL1访问，则FPU无法执行。 但是，有一个catch。 我们可以“窃取”CPACR_EL1访问单独的蹦床：

1. 取消patch
2. 触发CPACR_EL1，管理程序然后运行恢复执行权 
3. 再次patch
4. profit
这种绕过方式在[@qwertyoruiop](https://twitter.com/qwertyoruiopz)的yalu102中使用过。
