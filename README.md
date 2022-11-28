# elf_so_viewer
Show .so requirements for .elf file

## Build
Just run `make`.

```
$ make && ./elf_so_viewer elf_so_viewer
gcc -Wall -I. main.c elf_parser.c -o elf_so_viewer
libc.so.6
ld-linux-x86-64.so.2
```

## TODO
* Add 32-bit support;
* Parse `/etc/ld.so.conf.d/*.conf` to fetch full system path prefixes;
* Replace recursion inspecting on queue-based algorithm;

## Examples
```
$ ./elf_so_viewer /bin/apt
libapt-private.so.0.0
libapt-pkg.so.6.0
libz.so.1
libc.so.6
ld-linux-x86-64.so.2
libbz2.so.1.0
liblzma.so.5
liblz4.so.1
libzstd.so.1
libudev.so.1
libsystemd.so.0
libcap.so.2
libgcrypt.so.20
libgpg-error.so.0
libxxhash.so.0
libstdc++.so.6
libm.so.6
libgcc_s.so.1
```

## Tested on
```
$ uname -a
Linux ubuntu-VirtualBox 5.15.0-53-generic #59-Ubuntu SMP Mon Oct 17 18:53:30 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux
```
