
# iOS 10.3.3 kernelcache dump

```
➜  10.3.3 pwd
/kanxue/201707/10.3.3
```

### Mach & MIG

```
➜  10.3.3 ../joker/joker.universal -m kernelcache.release.n66 > Mach
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 12799142, Uncompressed: 25116672. Unknown (CRC?): 0xf87098cf, Unknown 1: 0x1
btw, KPP is at 12799579 (0xc34e5b)..And I saved it for you in /tmp/kpp
Got kernel at 438
NOTE: Found an actual trap at #95, where kern_invalid was expected. Apple must have added a Mach trap!
```

**RESULT: [Mach](../10.3.3/Mach)**

### Kexts

```
➜  10.3.3 ../joker/joker.universal -k kernelcache.release.n66 > kexts
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 12799142, Uncompressed: 25116672. Unknown (CRC?): 0xf87098cf, Unknown 1: 0x1
btw, KPP is at 12799579 (0xc34e5b)..And I saved it for you in /tmp/kpp
Got kernel at 438
Got 194 kexts
```

**RESULT: [kexts](../10.3.3/kexts)**

### Syscalls

```
➜  10.3.3 ../joker/joker.universal -s  kernelcache.release.n66 > syscalls
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 12799142, Uncompressed: 25116672. Unknown (CRC?): 0xf87098cf, Unknown 1: 0x1
btw, KPP is at 12799579 (0xc34e5b)..And I saved it for you in /tmp/kpp
Got kernel at 438
```

**RESULT: [syscalls](../10.3.3/syscalls)**

# iOS 11.0 Beta 4 kernelcache dump

```
➜  11.0 pwd
/kanxue/201707/11.0
```

### Mach & MIG

```
➜  11.0 ls
kernelcache.release.iphone7 kernelcache.release.n66
➜  11.0 ../joker/joker.universal -m kernelcache.release.n66 > Mach
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 13928522, Uncompressed: 27639808. Unknown (CRC?): 0x28af47e9, Unknown 1: 0x1
btw, KPP is at 13928957 (0xd489fd)..And I saved it for you in /tmp/kpp
Got kernel at 436
NOTE: Found an actual trap at #50, where kern_invalid was expected. Apple must have added a Mach trap!
NOTE: Found an actual trap at #95, where kern_invalid was expected. Apple must have added a Mach trap!
```

**RESULT:[Mach](../11.0/Mach)**

### Kexts

```
➜  11.0 ../joker/joker.universal -k kernelcache.release.n66 > kexts
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 13928522, Uncompressed: 27639808. Unknown (CRC?): 0x28af47e9, Unknown 1: 0x1
btw, KPP is at 13928957 (0xd489fd)..And I saved it for you in /tmp/kpp
Got kernel at 436
Got 194 kexts
```

**RESULT:[kexts](../11.0/kexts)**

### Syscalls

```
➜  11.0 ../joker/joker.universal -s kernelcache.release.n66 > syscalls
Feeding me a compressed kernelcache, eh? That's fine, now. I can decompress! (Type -dec _file_ if you want to save to file)!
Compressed Size: 13928522, Uncompressed: 27639808. Unknown (CRC?): 0x28af47e9, Unknown 1: 0x1
btw, KPP is at 13928957 (0xd489fd)..And I saved it for you in /tmp/kpp
Got kernel at 436
```

**RESULT:[syscalls](../11.0/syscalls)**
