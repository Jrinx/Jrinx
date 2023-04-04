# Sifive Hifive Unmatched 板上实验

主要流程：u-boot-spl 引导进入 opensbi，opensbi 启动后将控制权转交给 u-boot（u-boot 提供设备树，而不是 opensbi 提供设备树），使用 u-boot 的 tftpboot 功能从网络加载操作系统到内存。

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
=> setenv kernel_addr_r 0x80200000
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

配置网线接口的 IP 地址为 `10.0.0.5`，并将操作系统镜像文件 `jrinx.uImage` 放入 `/srv/tftp` 目录下。随后可用如下命令测试 tftp 服务器：

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
=> tftp ${kernel_addr_r} jrinx.uImage; bootm ${kernel_addr_r} - ${fdtcontroladdr}
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
=> setenv bootcmd 'tftp ${kernel_addr_r} jrinx.uImage; bootm ${kernel_addr_r} - ${fdtcontroladdr}'
=> saveenv
```

## 启动举例

设置 `bootargs` 环境变量：

```console
=> setenv bootargs '--pa-conf name=put-a-b,prog=app_put_a_b,memory=8388608,period=200,duration=100'
```

随后用：

```console
=> tftp ${kernel_addr_r} jrinx.uImage; bootm ${kernel_addr_r} - ${fdtcontroladdr}
```

启动内核。以下是完整的输出：

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
=> setenv bootargs '--pa-conf name=put-a-b,prog=app_put_a_b,memory=8388608,period=200,duration=100'
=> tftp ${kernel_addr_r} jrinx.uImage; bootm ${kernel_addr_r} - ${fdtcontroladdr}
ethernet@10090000: PHY present at 0
ethernet@10090000: Starting autonegotiation...
ethernet@10090000: Autonegotiation complete
ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x6800)
Using ethernet@10090000 device
TFTP from server 10.0.0.5; our IP address is 10.0.0.6
Filename 'jrinx.uImage'.
Load address: 0x80200000
Loading: #########################################################
         140.6 KiB/s
done
Bytes transferred = 831552 (cb040 hex)
## Booting kernel from Legacy Image at 80200000 ...
   Image Name:   Jrinx
   Image Type:   RISC-V Linux Kernel Image (uncompressed)
   Data Size:    831488 Bytes = 812 KiB
   Load Address: 80200000
   Entry Point:  80200000
   Verifying Checksum ... OK
## Flattened Device Tree blob at ff7394d0
   Booting using the fdt blob at 0xff7394d0
   Loading Kernel Image
   Loading Device Tree to 00000000ff72d000, end 00000000ff737937 ... OK

Starting kernel ...


Jrinx OS (revision: 068ef15)

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

[ ?.??? hart#1 ] init.c:77 <kernel_init> Hello Jrinx, I am master hart!
[ 0.043 hart#1 ] cpus.c:97 <cpus_probe> cpu@1 (master) probed (stack top: 00000000802cb000)
[ 0.051 hart#1 ] cpus.c:93 <cpus_probe> cpu@2 (slave)  probed (stack top: 00000000802dc000)
[ 0.060 hart#1 ] cpus.c:93 <cpus_probe> cpu@3 (slave)  probed (stack top: 00000000802ed000)
[ 0.069 hart#1 ] cpus.c:93 <cpus_probe> cpu@4 (slave)  probed (stack top: 00000000802fe000)
[ 0.078 hart#1 ] mems.c:36 <mem_probe> memory@80000000 probed (consists of 1 memory):
[ 0.086 hart#1 ] mems.c:40 <mem_probe>  memory[0] locates at [80000000, 480000000) (size: 16 GiB)
[ 0.095 hart#1 ] plic.c:156 <plic_probe> interrupt-controller@c000000 probed (phandle: 10) to handle user external int
[ 0.106 hart#1 ] plic.c:158 <plic_probe>        locates at [c000000, 10000000) (size: 64 MiB)
[ 0.115 hart#1 ] plic.c:98 <plic_init> set all interrupt sources priority to MIN
[ 0.123 hart#1 ] plic.c:105 <plic_init> disable all interrupt sources for all context (0 - 15871)
[ 0.132 hart#1 ] plic.c:106 <plic_init> set all context priority threshold to MAX
[ 0.147 hart#1 ] plic.c:112 <plic_init> all interrupt sources shall be handled by context 2
[ 0.155 hart#1 ] sifiveuart0.c:100 <sifiveuart0_probe> serial@10010000 probed, interrupt 00000027 registered to intc 10
[ 0.166 hart#1 ] sifiveuart0.c:102 <sifiveuart0_probe>  locates at [10010000, 10011000) (size: 4 KiB)
[ 0.176 hart#1 ] sifiveuart0.c:100 <sifiveuart0_probe> serial@10011000 probed, interrupt 00000028 registered to intc 10
[ 0.187 hart#1 ] sifiveuart0.c:102 <sifiveuart0_probe>  locates at [10011000, 10012000) (size: 4 KiB)
[ 0.197 hart#1 ] aliases.c:43 <aliases_probe> aliases probed, props listed:
[ 0.204 hart#1 ] aliases.c:49 <aliases_probe>   spi0: /soc/spi@10050000
[ 0.211 hart#1 ] aliases.c:49 <aliases_probe>   serial1: /soc/serial@10011000
[ 0.219 hart#1 ] aliases.c:49 <aliases_probe>   ethernet0: /soc/ethernet@10090000
[ 0.227 hart#1 ] aliases.c:49 <aliases_probe>   serial0: /soc/serial@10010000
[ 0.234 hart#1 ] chosen.c:18 <chosen_probe> chosen probed, props listed:
[ 0.242 hart#1 ] chosen.c:24 <chosen_probe>     bootargs: --pa-conf name=put-a-b,prog=app_put_a_b,memory=8388608,period=200,duration=100
[ 0.254 hart#1 ] chosen.c:30 <chosen_probe>     stdout-path: serial0
[ 0.261 hart#1 ] serialport.c:56 <serial_select_out_dev> select serial@10010000 as serial output device
[ 0.271 hart#1 ] serialport.c:67 <serial_select_in_dev> select serial@10010000 as serial input device
[ 0.281 hart#1 ] vmm.c:164 <vmm_setup_mmio> set up serial@10011000 mmio at [ffffffc010011000, ffffffc010012000) (size: 4 KiB)
[ 0.292 hart#1 ] vmm.c:164 <vmm_setup_mmio> set up serial@10010000 mmio at [ffffffc010010000, ffffffc010011000) (size: 4 KiB)
[ 0.304 hart#1 ] vmm.c:164 <vmm_setup_mmio> set up interrupt-controller@c000000 mmio at [ffffffc00c000000, ffffffc010000000) (size: 64 MiB)
[ 0.323 hart#1 ] vmm.c:181 <vmm_setup_kern> set up kernel vmm at 0000000080200000
[ 1.797 hart#1 ] vmm.c:206 <vmm_setup_kern> init physical memory management
[ 2.473 hart#1 ] vmm.c:209 <vmm_setup_kern> switch to physical frame allocator
[ 2.480 hart#1 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802b9)
[ 2.488 hart#1 ] asid.c:34 <asid_init> asid in cpu@1 impl probed [0, 0]
[ 2.496 hart#1 ] vmm.c:228 <vmm_summary> os kernel reserves memory [80200000, 8a363000) (size: 161 MiB + 396 KiB)
[ 2.506 hart#1 ] logger.c:27 <log_localize_output> switch to local serial output
[ 2.514 hart#2 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802b9)
[ 2.522 hart#3 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802b9)
[ 2.531 hart#4 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 80000000000802b9)
[ 2.539 hart#2 ] asid.c:34 <asid_init> asid in cpu@2 impl probed [0, 0]
[ 2.546 hart#4 ] asid.c:34 <asid_init> asid in cpu@4 impl probed [0, 0]
[ 2.553 hart#3 ] asid.c:34 <asid_init> asid in cpu@3 impl probed [0, 0]
[ 2.560 hart#2 ] init.c:99 <kernel_init> Hello Jrinx, I am slave hart!
[ 2.567 hart#3 ] init.c:99 <kernel_init> Hello Jrinx, I am slave hart!
[ 2.574 hart#4 ] init.c:99 <kernel_init> Hello Jrinx, I am slave hart!
[ 2.582 hart#1 ] partition.c:201 <part_create> create partition: name='put-a-b',prog='app_put_a_b',memory=8 MiB,period=200ms,duration=100ms
[ 2.594 hart#1 ] partition.c:209 <part_create> program 'app_put_a_b' found at [80217cc0, 80229f08) (size: 72 KiB + 584 B)
[ 2.607 hart#1 ] partition.c:215 <part_create> create main process for partition 'put-a-b': name='main',entrypoint=00000000004004e8,stacksize=4 KiB
[ 2.621 hart#1 ] partition.c:221 <part_create> remaining memory of partition 'put-a-b': 7 MiB + 920 KiB
[ 2.630 hart#1 ] sched.c:24 <sched_add_part> add partition 1 to scheduler
[ part#1 proc#1 ] osmain.c:12 <_osmain> partition 1 init with status:
[ part#1 proc#1 ] osmain.c:13 <_osmain> - period: 200ms
[ part#1 proc#1 ] osmain.c:14 <_osmain> - duration: 100ms
[ part#1 proc#1 ] osmain.c:15 <_osmain> - lock level: 0
[ part#1 proc#1 ] osmain.c:16 <_osmain> - start cond: 0
[ part#1 proc#1 ] osmain.c:17 <_osmain> - assigned cores: 1
[ part#1 proc#2 index=1 ] put_a_b.c:23 <put_a> put 'A' to console
[ part#1 proc#3 index=2 ] put_a_b.c:34 <put_b> put 'B' to console
ABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABABA
```
