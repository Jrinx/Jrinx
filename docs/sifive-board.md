# Sifive Hifive Unmatched 板上实验

主要流程：opensbi 引导进入 u-boot（u-boot 提供设备树，而不是 opensbi 提供设备树），使用 u-boot 的 tftpboot 功能从网络加载操作系统到内存。

## 硬件准备

1. Sifive Hifive Unmatched 开发板
2. Micro USB 线（连接开发板的串口）
3. RJ45 网线
4. SD 卡与 SD 读卡器

## 软件准备

### 构建 opensbi

在本仓库中使用下述命令以构建 opensbi：

```console
$ cd <path to this repo>
$ make sbi-fw
```

### 构建 u-boot

将 opensbi 的构建结果中的 `fw_dynamic.bin` 作为 u-boot 的负载进行编译：

```console
$ cd <path to u-boot repo>
$ export OPENSBI=<path to this repo>/opensbi/build/platform/generic/firmware/fw_dynamic.bin
$ export CROSS_COMPILE=<riscv64 toolchains prefix>
$ make sifive_unmatched_defconfig
$ make -j$(nproc)
```

### 烧写 SD 卡

将 SD 卡放入 SD 读卡器，用个人计算机（Linux 系统）按下述步骤读取并修改 SD 卡内容。

#### 格式化

使用下述命令格式化 SD 卡（`/dev/sdX` 应替换为实际设备路径），并创建相应的分区：

```console
$ sudo sgdisk -g --clear -a 1 \
  --new=1:34:2081         --change-name=1:spl   --typecode=1:5B193300-FC78-40CD-8002-E86C45580B47 \
  --new=2:2082:10273      --change-name=2:uboot --typecode=2:2E54B353-1271-4842-806F-E436D6AF6985 \
  --new=3:16384:282623    --change-name=3:boot  --typecode=3:0x0700 \
  --new=4:286720:13918207 --change-name=4:root  --typecode=4:0x8300 \
  /dev/sdX
```

#### 烧写 u-boot

将 u-boot 的编译结果烧写到 SD 卡中（`/dev/sdX` 应替换为实际设备路径）：

```console
$ cd <path to u-boot repo>
$ sudo dd if=spl/u-boot-spl.bin of=/dev/sdX seek=34
$ sudo dd if=u-boot.itb of=/dev/sdX seek=2082
```

## 开发板上实验

在个人计算机与开发板之间连接串口线与网线，将 SD 卡插入开发板，启动串口软件，开发板上电。上电后，开发板会自动启动 u-boot，若看到：

```plaintext
Hit any key to stop autoboot
```

应及时按下任意键，以防止 u-boot 自动执行预设的 `bootcmd`。

### 构建操作系统镜像

先使用 `make` 或 `make release` 等命令构建出内核 ELF 文件，然后在本仓库中使用下述命令以构建操作系统镜像：

```console
$ make mkimage
```

即可在 `target` 目录下找到 `jrinx.uImage` 文件。

### 设置 u-boot 环境变量

在 u-boot 命令行中输入如下命令，以设置 u-boot 环境变量：

```console
=> setenv serverip 10.0.0.5
=> setenv ipaddr 10.0.0.6
=> setenv netmask 255.255.255.0
=> saveenv
```

### 配置 tftp 服务器

在个人计算机上搭建 tftp 服务器，以提供操作系统镜像文件。如：

```console
$ sudo apt install tftpd-hpa
$ sudo mkdir -p /srv/tftp
```

在 `/etc/default/tftpd-hpa` 中设置 `TFTP_DIRECTORY` 为 `/srv/tftp`，并重启 tftp 服务器：

```console
$ sudo systemctl restart tftpd-hpa
```

配置网线接口的 IP 地址为 `10.0.0.5`，并将操作系统镜像文件 `jrinx.uImage`、开发板设备树二进制文件 `<path to u-boot repo>/arch/riscv/dts/hifive-unmatched-a00.dtb` 放入 `/srv/tftp` 目录下。随后可用如下命令测试 tftp 服务器：

```console
$ tftp
tftp> connect
tftp> get jrinx.uImage
```

可以在工作目录中找到 `jrinx.uImage` 文件。

### 从网络加载操作系统

先在 u-boot 命令行中用 ping 命令测试网络连接：

```console
=> ping 10.0.0.5
```

然后在 u-boot 命令行中输入如下命令，以从网络加载并运行操作系统：

```console
=> tftp 0x80200000 jrinx.uImage; tftp 0x90200000 hifive-unmatched-a00.dtb; bootm 0x80200000 - 0x90200000
```

### 系统启动命令

可在 u-boot 命令行中设置 `bootargs` 环境变量以传递内核启动参数，如：

```console
=> setenv bootargs '--debug-dt'
```

即可让内核打印出设备树信息。

### u-boot 启动命令

在 u-boot 命令行中，设置 `bootcmd` 环境变量以设置 u-boot 启动命令以方便调试，如：

```console
=> setenv bootcmd 'tftp 0x80200000 jrinx.uImage; tftp 0x90200000 hifive-unmatched-a00.dtb; bootm 0x80200000 - 0x90200000'
=> saveenv
```

## 启动举例

在使用：

```console
=> tftp 0x80200000 jrinx.uImage; tftp 0x90200000 hifive-unmatched-a00.dtb
```

加载内核与 dtb 到内存后，设置 `bootargs` 环境变量：

```console
=> setenv bootargs '--pa-conf name=loop1,prog=bare_loop,memory=4194304;name=loop2,prog=bare_loop,memory=8388608 --debug-as-switch --debug-kalloc-used'
```

随后用 `bootm 0x80200000 - 0x90200000` 启动内核。以下是完整的输出：

```plaintext
U-Boot SPL 2022.01-00566-g4e81f3be34-dirty (Apr 15 2022 - 14:24:34 +0800)
Trying to boot from SPI


U-Boot 2022.01-00566-g4e81f3be34-dirty (Apr 15 2022 - 14:24:34 +0800)

CPU:   rv64imafdc
Model: SiFive HiFive Unmatched A00
DRAM:  16 GiB
Core:  21 devices, 16 uclasses, devicetree: separate
MMC:   spi@10050000:mmc@0: 0
Loading Environment from SPIFlash... SF: Detected is25wp256 with page size 256 Bytes, erase size 4 KiB, total 32 MiB
OK
EEPROM: SiFive PCB EEPROM format v1
Product ID: 0002 (HiFive Unmatched)
PCB revision: 3
BOM revision: B
BOM variant: 0
Serial number: SF105SZ212500165
Ethernet MAC address: 70:b3:d5:92:fa:fc
CRC: 522ad10a
In:    serial@10010000
Out:   serial@10010000
Err:   serial@10010000
Model: SiFive HiFive Unmatched A00
Net:   eth0: ethernet@10090000
Hit any key to stop autoboot:  0
=> tftp 0x80200000 jrinx.uImage; tftp 0x90200000 hifive-unmatched-a00.dtb
=> setenv bootargs '--pa-conf name=loop1,prog=bare_loop,memory=4194304;name=loop2,prog=bare_loop,memory=8388608 --debug-as-switch --debug-kalloc-used'
=> bootm 0x80200000 - 0x90200000
ethernet@10090000: PHY present at 0
ethernet@10090000: Starting autonegotiation...
ethernet@10090000: Autonegotiation complete
ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x6800)
Using ethernet@10090000 device
TFTP from server 10.0.0.5; our IP address is 10.0.0.6
Filename 'jrinx.uImage'.
Load address: 0x80200000
Loading: #################################################################
         ######
         168.9 KiB/s
done
Bytes transferred = 1028160 (fb040 hex)
ethernet@10090000: PHY present at 0
ethernet@10090000: Starting autonegotiation...
ethernet@10090000: Autonegotiation complete
ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x6800)
Using ethernet@10090000 device
TFTP from server 10.0.0.5; our IP address is 10.0.0.6
Filename 'hifive-unmatched-a00.dtb'.
Load address: 0x90200000
Loading: ##
         3.9 KiB/s
done
Bytes transferred = 22036 (5614 hex)
## Booting kernel from Legacy Image at 80200000 ...
   Image Name:   Jrinx
   Image Type:   RISC-V Linux Kernel Image (uncompressed)
   Data Size:    1028096 Bytes = 1004 KiB
   Load Address: 80200000
   Entry Point:  80200000
   Verifying Checksum ... OK
## Flattened Device Tree blob at 90200000
   Booting using the fdt blob at 0x90200000
   Loading Kernel Image
   Loading Device Tree to 00000000ff72f000, end 00000000ff737613 ... OK

Starting kernel ...


Jrinx OS (revision: 15948f5)

      .                           .
  .x88888x.                      @88>
 :8**888888X.  :>    .u    .     %8P      u.    u.      uL   ..
 f    `888888x./   .d88B :@8c     .     x@88k u@88c.  .@88b  @88R
'       `*88888~  ="8888f8888r  .@88u  ^"8888""8888" '"Y888k/"*P
 \.    .  `?)X.     4888>'88"  ''888E`   8888  888R     Y888L
  `~=-^   X88> ~    4888> '      888E    8888  888R      8888
         X8888  ~   4888>        888E    8888  888R      `888N
         488888    .d888L .+     888E    8888  888R   .u./"888&
 .xx.     88888X   ^"8888*"      888&   "*88*" 8888" d888" Y888*"
'*8888.   '88888>     "Y"        R888"    ""   'Y"   ` "Y   Y"
  88888    '8888>                 ""
  `8888>    `888
   "8888     8%
    `"888x:-"

[ ?.??? hart#1 ] init.c:72 <kernel_init> Hello Jrinx, I am master hart!
[ 0.040 hart#1 ] cpus.c:97 <cpus_probe> cpu@1 (master) probed (stack top: 00000000802fb000)
[ 0.048 hart#1 ] cpus.c:93 <cpus_probe> cpu@2 (slave)  probed (stack top: 000000008030c000)
[ 0.057 hart#1 ] cpus.c:93 <cpus_probe> cpu@3 (slave)  probed (stack top: 000000008031d000)
[ 0.066 hart#1 ] cpus.c:93 <cpus_probe> cpu@4 (slave)  probed (stack top: 000000008032e000)
[ 0.075 hart#1 ] mems.c:36 <mem_probe> memory@80000000 probed (consists of 1 memory):
[ 0.083 hart#1 ] mems.c:40 <mem_probe>  memory[0] locates at [80000000, 480000000) (size: 16 GiB)
[ 0.092 hart#1 ] plic.c:156 <plic_probe> interrupt-controller@c000000 probed (phandle: 10) to handle user external int
[ 0.103 hart#1 ] plic.c:158 <plic_probe>        locates at [c000000, 10000000) (size: 64 MiB)
[ 0.112 hart#1 ] plic.c:98 <plic_init> set all interrupt sources priority to MIN
[ 0.120 hart#1 ] plic.c:105 <plic_init> disable all interrupt sources for all context (0 - 15871)
[ 0.129 hart#1 ] plic.c:106 <plic_init> set all context priority threshold to MAX
[ 0.144 hart#1 ] plic.c:112 <plic_init> all interrupt sources shall be handled by context 2
[ 0.152 hart#1 ] sifiveuart0.c:100 <sifiveuart0_probe> serial@10010000 probed, interrupt 00000027 registered to intc 10
[ 0.163 hart#1 ] sifiveuart0.c:102 <sifiveuart0_probe>  locates at [10010000, 10011000) (size: 4 KiB)
[ 0.173 hart#1 ] sifiveuart0.c:100 <sifiveuart0_probe> serial@10011000 probed, interrupt 00000028 registered to intc 10
[ 0.184 hart#1 ] sifiveuart0.c:102 <sifiveuart0_probe>  locates at [10011000, 10012000) (size: 4 KiB)
[ 0.194 hart#1 ] aliases.c:43 <aliases_probe> aliases probed, props listed:
[ 0.201 hart#1 ] aliases.c:49 <aliases_probe>   spi0: /soc/spi@10050000
[ 0.209 hart#1 ] aliases.c:49 <aliases_probe>   serial1: /soc/serial@10011000
[ 0.216 hart#1 ] aliases.c:49 <aliases_probe>   ethernet0: /soc/ethernet@10090000
[ 0.224 hart#1 ] aliases.c:49 <aliases_probe>   serial0: /soc/serial@10010000
[ 0.231 hart#1 ] chosen.c:18 <chosen_probe> chosen probed, props listed:
[ 0.239 hart#1 ] chosen.c:24 <chosen_probe>     bootargs: --pa-conf name=loop1,prog=bare_loop,memory=4194304;name=loop2,prog=bare_loop,memory=8388608 --debug-as-switch --debug-kalloc-used
[ 0.256 hart#1 ] chosen.c:30 <chosen_probe>     stdout-path: serial0
[ 0.262 hart#1 ] serialport.c:56 <serial_select_out_dev> select serial@10010000 as serial output device
[ 0.272 hart#1 ] serialport.c:67 <serial_select_in_dev> select serial@10010000 as serial input device
[ 0.282 hart#1 ] vmm.c:164 <vmm_setup_mmio> set up serial@10011000 mmio at [3010011000, 3010012000) (size: 4 KiB)
[ 0.293 hart#1 ] vmm.c:164 <vmm_setup_mmio> set up serial@10010000 mmio at [3010010000, 3010011000) (size: 4 KiB)
[ 0.303 hart#1 ] vmm.c:164 <vmm_setup_mmio> set up interrupt-controller@c000000 mmio at [300c000000, 3010000000) (size: 64 MiB)
[ 0.321 hart#1 ] vmm.c:181 <vmm_setup_kern> set up kernel vmm at 0000000080200000
[ 1.867 hart#1 ] vmm.c:206 <vmm_setup_kern> init physical memory management
[ 2.544 hart#1 ] vmm.c:209 <vmm_setup_kern> switch to physical frame allocator
[ 2.551 hart#1 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802a9)
[ 2.560 hart#1 ] asid.c:34 <asid_init> asid in cpu@1 impl probed [0, 0]
[ 2.567 hart#1 ] vmm.c:228 <vmm_summary> os kernel reserves memory [80200000, 8a393000) (size: 161 MiB + 588 KiB)
[ 2.577 hart#1 ] logger.c:27 <log_localize_output> switch to local serial output
[ 2.586 hart#4 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802a9)
[ 2.594 hart#3 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802a9)
[ 2.602 hart#2 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802a9)
[ 2.610 hart#4 ] asid.c:34 <asid_init> asid in cpu@4 impl probed [0, 0]
[ 2.618 hart#3 ] asid.c:34 <asid_init> asid in cpu@3 impl probed [0, 0]
[ 2.625 hart#4 ] init.c:94 <kernel_init> Hello Jrinx, I am slave hart!
[ 2.632 hart#2 ] asid.c:34 <asid_init> asid in cpu@2 impl probed [0, 0]
[ 2.639 hart#3 ] init.c:94 <kernel_init> Hello Jrinx, I am slave hart!
[ 2.646 hart#2 ] init.c:94 <kernel_init> Hello Jrinx, I am slave hart!
[ 2.653 hart#1 ] partition.c:181 <part_create> create partition: name='loop1',prog='bare_loop',memory=4 MiB
[ 2.663 hart#1 ] partition.c:189 <part_create> program 'bare_loop' found at [80215a40, 80217070) (size: 5 KiB + 560 B)
[ 2.675 hart#1 ] partition.c:194 <part_create> create main process for partition 'loop1': name='main',entrypoint=0000000000400000,stacksize=4 KiB
[ 2.688 hart#1 ] partition.c:199 <part_create> remaining memory of partition 'loop1': 3 MiB + 1012 KiB
[ 2.698 hart#1 ] sched.c:28 <sched_assign_proc> assign 'main' of partition 1 to cpu@1
[ 2.706 hart#1 ] partition.c:181 <part_create> create partition: name='loop2',prog='bare_loop',memory=8 MiB
[ 2.716 hart#1 ] partition.c:189 <part_create> program 'bare_loop' found at [80215a40, 80217070) (size: 5 KiB + 560 B)
[ 2.728 hart#1 ] partition.c:194 <part_create> create main process for partition 'loop2': name='main',entrypoint=0000000000400000,stacksize=4 KiB
[ 2.741 hart#1 ] partition.c:199 <part_create> remaining memory of partition 'loop2': 7 MiB + 1012 KiB
[ 2.751 hart#1 ] sched.c:28 <sched_assign_proc> assign 'main' of partition 2 to cpu@1
[ 2.759 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop1' (asid: 0)
[ 2.768 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.778 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.784 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop2' (asid: 0)
[ 2.793 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.803 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.809 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop1' (asid: 0)
[ 2.827 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.834 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop2' (asid: 0)
[ 2.852 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.858 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop1' (asid: 0)
[ 2.877 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.883 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop2' (asid: 0)
[ 2.902 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.908 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop1' (asid: 0)
[ 2.926 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.933 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop2' (asid: 0)
[ 2.951 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.957 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop1' (asid: 0)
[ 2.976 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
[ 2.982 hart#1 ] process.c:126 <proc_run> switch to address space of 'loop2' (asid: 0)
[ 3.001 hart#1 ] traps.c:44 <handle_trap> kalloc used: 262 KiB + 128 B
```
