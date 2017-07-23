# Joker
## 简介
Joker是个快速并且肮脏的(很牛逼的?)iOS kernelcache 处理工具，用于辅助逆向。苹果尽最大的努力使得内核调试变得困难：每次iOS的发布，越来越多的符号被裁掉(strip)。已经预先链接好的kernelcache也会需要较少的符号(并且内存中的tables，像LINKEDIT段都被抛弃，放弃使用了)。而且，kernelcache是被加密的。虽然32-bit的kernelcache在@xerub等人的努力下可以被解密，但是目前还没有公开的64-bit的kernelcache解密密钥，因此唯一的研究途径就是去dump~
重重的困难并不会阻碍越狱者的脚步。**Joker**和jtool都使用了相同的machlib，两者可以很好的配合使用。
Joker主要还是为iOS系统设计的，但是Mac os x和iOS内核中XNU的数据结构是一样的，所以joker也可以适用于OS X的kernel(使用选项 -m -s)

## 准备工作
### 下载joker
[可执行文件下载](http://newosxbook.com/tools/joker.tar)
里面有两个bin，分别是ELF和Mach-O的~我们当然是用Mach-O的啦~
### 获取解密后的kernelcache
iOS10后，kernelcache不再加密...
所以，我们直接去下载iOS 10.1的OTA（Over－the－Air Technology）
1. 下载：[iOS 10b1 OTA包下载地址](https://pastebin.com/FRMfanmT)
2. 解压定位：解压OTA压缩包，定位到`/AssetData/boot/kernelcache.release.***`
3. 编译lzssdec.cpp：[源码地址](http://nah6.com/~itsme/cvs-xdadevtools/iphone/tools/lzssdec.cpp) `clang++ lzssdec.cpp -o lzssdec`
4. 定位kernelcache偏移量：用010 UE Hooper IDA都可以，定位magic的偏移 0xFEEDFACE 0xFEEDFAFE (可能会是小端)
5. `./lzssdec -o 0x1B7 < kernelcache.release.*** > kernelcache.decrypted`
6. 后续我们就可以来“戏耍”kernelcache.decrypted

## Joker的使用

### 参数选项
 -m: dump Mach Traps and MIG tables (NEW)
 -a: dump everything
 -k: dump kexts
 -K: kextract [kext_bundle_id_or_name_shown_in_-k|all] to JOKER_DIR or /tmp
 -S: dump sysctls
 -s: dump UNIX syscalls
 -j: Jtool compatible output (to companion file)
