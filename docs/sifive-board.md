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
=> setenv env_addr_r 0x83000000
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
$ tftp localhost
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

另外，也可通过 [sysconf](../scripts/sysconf) 脚本来生成 [sys-conf](../sys-conf/) 目录下系统配置文件对应的启动参数。如：

```console
$ ./scripts/sysconf ./sys-conf/ping-pong.yml
--pa-conf name=ping-pong,prog=app_ping_pong,memory=8388608,period=200000,duration=100000
```

参数过长时，可能将无法通过命令行将其传入 u-boot，此时可借助 [u-bootargs](../scripts/u-bootargs) 脚本来生成 `uEnv.txt` 文件，并根据其提示来输入 u-boot 命令。如：

```console
$ ./scripts/sysconf ./sys-conf/ping-pong.yml | ./scripts/u-bootargs -o /srv/tftp/uEnv.txt
tftp ${env_addr_r} uEnv.txt; env import -t ${env_addr_r} 98
```

### u-boot 启动命令

在 u-boot 命令行中，设置 `bootcmd` 环境变量以设置 u-boot 启动命令以方便调试，如：

```console
=> setenv bootcmd 'tftp ${kernel_addr_r} jrinx.uImage; bootm ${kernel_addr_r} - ${fdtcontroladdr}'
=> saveenv
```

## 启动举例

设置 `bootargs` 环境变量：

```console
=> setenv bootargs '--pa-conf name=ping-pong,prog=app_ping_pong,memory=8388608,period=200000,duration=100000'
```

随后用：

```console
=> run bootcmd
```

启动内核。以下是完整的输出：

```plaintext
U-Boot SPL 2023.04-rc4-g318af476 (Apr 30 2023 - 14:50:15 +0800)
Trying to boot from MMC1


U-Boot 2023.04-rc4-g318af476 (Apr 30 2023 - 14:50:15 +0800)

CPU:   rv64imafdc
Model: SiFive HiFive Unmatched A00
DRAM:  16 GiB
Core:  34 devices, 21 uclasses, devicetree: separate
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
Working FDT set to ff72f830
Hit any key to stop autoboot:  0
=> setenv bootargs '--pa-conf name=ping-pong,prog=app_ping_pong,memory=8388608,period=200000,duration=100000'
=> run bootcmd
ethernet@10090000: PHY present at 0
ethernet@10090000: Starting autonegotiation...
ethernet@10090000: Autonegotiation complete
ethernet@10090000: link up, 1000Mbps full-duplex (lpa: 0x2800)
Using ethernet@10090000 device
TFTP from server 10.0.0.5; our IP address is 10.0.0.6
Filename 'jrinx.uImage'.
Load address: 0x80200000
Loading: #################################################################
         ##########################################
         236.3 KiB/s
done
Bytes transferred = 1556544 (17c040 hex)
## Booting kernel from Legacy Image at 80200000 ...
   Image Name:   Jrinx
   Image Type:   RISC-V Linux Kernel Image (uncompressed)
   Data Size:    1556480 Bytes = 1.5 MiB
   Load Address: 80200000
   Entry Point:  80200000
   Verifying Checksum ... OK
## Flattened Device Tree blob at ff72f830
   Booting using the fdt blob at 0xff72f830
Working FDT set to ff72f830
   Loading Kernel Image
   Loading Device Tree to 00000000fe720000, end 00000000fe72a5a7 ... OK
Working FDT set to fe720000

Starting kernel ...


Jrinx OS (revision: 2d88976)

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

[ ?.?????? hart#4 ] init.c:81 <kernel_init> Hello Jrinx, I am master hart!
[ 38.205505 hart#4 ] cpus.c:112 <cpus_probe> cpu@1 (slave)  probed (stack top: 000000008038d000)
[ 38.214028 hart#4 ] cpus.c:112 <cpus_probe> cpu@2 (slave)  probed (stack top: 000000008039d000)
[ 38.223316 hart#4 ] cpus.c:112 <cpus_probe> cpu@3 (slave)  probed (stack top: 00000000803ad000)
[ 38.232610 hart#4 ] cpus.c:116 <cpus_probe> cpu@4 (master) probed (stack top: 000000008037c000)
[ 38.242030 hart#4 ] mems.c:36 <mem_probe> memory@80000000 probed (consists of 1 memory):
[ 38.250586 hart#4 ] mems.c:40 <mem_probe>      memory[0] locates at [80000000, 480000000) (size: 16 GiB)
[ 38.260400 hart#4 ] plic.c:156 <plic_probe> interrupt-controller@c000000 probed (phandle: 10) to handle user external int
[ 38.271866 hart#4 ] plic.c:158 <plic_probe>    locates at [c000000, 10000000) (size: 64 MiB)
[ 38.280796 hart#4 ] plic.c:98 <plic_init> set all interrupt sources priority to MIN
[ 38.288980 hart#4 ] plic.c:105 <plic_init> disable all interrupt sources for all context (0 - 15871)
[ 38.298705 hart#4 ] plic.c:106 <plic_init> set all context priority threshold to MAX
[ 38.309157 hart#4 ] plic.c:112 <plic_init> all interrupt sources shall be handled by context 2
[ 38.317796 hart#4 ] sifiveuart0.c:165 <sifiveuart0_probe> serial@10010000 probed, interrupt 00000027 registered to intc 10
[ 38.329223 hart#4 ] sifiveuart0.c:167 <sifiveuart0_probe>      locates at [10010000, 10011000) (size: 4 KiB)
[ 38.339407 hart#4 ] sifiveuart0.c:165 <sifiveuart0_probe> serial@10011000 probed, interrupt 00000028 registered to intc 10
[ 38.350937 hart#4 ] sifiveuart0.c:167 <sifiveuart0_probe>      locates at [10011000, 10012000) (size: 4 KiB)
[ 38.361197 hart#4 ] aliases.c:43 <aliases_probe> aliases probed, props listed:
[ 38.368865 hart#4 ] aliases.c:49 <aliases_probe>       spi0: /soc/spi@10050000
[ 38.376248 hart#4 ] aliases.c:49 <aliases_probe>       serial1: /soc/serial@10011000
[ 38.384152 hart#4 ] aliases.c:49 <aliases_probe>       ethernet0: /soc/ethernet@10090000
[ 38.392403 hart#4 ] aliases.c:49 <aliases_probe>       serial0: /soc/serial@10010000
[ 38.400281 hart#4 ] chosen.c:18 <chosen_probe> chosen probed, props listed:
[ 38.407830 hart#4 ] chosen.c:24 <chosen_probe>         bootargs: --pa-conf name=ping-pong,prog=app_ping_pong,memory=8388608,period=200000,duration=100000
[ 38.421553 hart#4 ] chosen.c:30 <chosen_probe>         stdout-path: serial0
[ 38.428508 hart#4 ] serialport.c:62 <serial_select_out_dev> select serial@10010000 as serial output device
[ 38.438751 hart#4 ] serialport.c:73 <serial_select_in_dev> select serial@10010000 as serial input device
[ 38.449032 hart#4 ] vmm.c:164 <vmm_setup_mmio> set up serial@10011000 mmio at [ffffffc010011000, ffffffc010012000) (size: 4 KiB)
[ 38.461016 hart#4 ] vmm.c:164 <vmm_setup_mmio> set up serial@10010000 mmio at [ffffffc010010000, ffffffc010011000) (size: 4 KiB)
[ 38.473146 hart#4 ] vmm.c:164 <vmm_setup_mmio> set up interrupt-controller@c000000 mmio at [ffffffc00c000000, ffffffc010000000) (size: 64 MiB)
[ 38.489612 hart#4 ] vmm.c:181 <vmm_setup_kern> set up kernel vmm at 0000000080200000
[ 39.316549 hart#4 ] vmm.c:206 <vmm_setup_kern> init physical memory management
[ 39.708231 hart#4 ] vmm.c:209 <vmm_setup_kern> switch to physical frame allocator
[ 39.715537 hart#4 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 8000000000080322)
[ 39.724221 hart#4 ] asid.c:32 <asid_init> asid in cpu@4 impl probed [0, 0]
[ 39.732035 hart#4 ] vmm.c:228 <vmm_summary> os kernel reserves memory [80200000, 883e2000) (size: 129 MiB + 904 KiB)
[ 39.743217 hart#3 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 8000000000080322)
[ 39.751497 hart#2 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 8000000000080322)
[ 39.760183 hart#1 ] vmm.c:220 <vmm_start> enable virtual memory (satp: 8000000000080322)
[ 39.768865 hart#3 ] asid.c:32 <asid_init> asid in cpu@3 impl probed [0, 0]
[ 39.776334 hart#2 ] asid.c:32 <asid_init> asid in cpu@2 impl probed [0, 0]
[ 39.783804 hart#3 ] init.c:103 <kernel_init> Hello Jrinx, I am slave hart!
[ 39.791274 hart#2 ] init.c:103 <kernel_init> Hello Jrinx, I am slave hart!
[ 39.798744 hart#1 ] asid.c:32 <asid_init> asid in cpu@1 impl probed [0, 0]
[ 39.806252 hart#1 ] init.c:103 <kernel_init> Hello Jrinx, I am slave hart!
[ 39.813762 hart#1 ] partition.c:310 <part_create> create partition: name='ping-pong',prog='app_ping_pong',memory=8 MiB,period=200000 us,duration=100000 us
[ 39.828102 hart#1 ] partition.c:318 <part_create> program 'app_ping_pong' found at [802191c0, 80220268) (size: 28 KiB + 168 B)
[ 39.840636 hart#1 ] partition.c:324 <part_create> create main process for partition 'ping-pong': name='main',entrypoint=0000000000400994,stacksize=4 KiB
[ 39.854787 hart#1 ] partition.c:330 <part_create> remaining memory of partition 'ping-pong': 7 MiB + 932 KiB
[ 39.864759 hart#1 ] channel.c:55 <channel_mem_setup> channel memory setup: [3500000000, 3500000000) (size: 0)
[ 39.875273 hart#1 ] sched.c:71 <sched_launch> sched major frame: 200000 us
[ 39.882760 part#1 proc#1 ] runtime.c:12 <_runtime> partition 1 init with status:
[ 39.882785 part#1 proc#1 ] runtime.c:13 <_runtime> - period: 200000 us
[ 39.882800 part#1 proc#1 ] runtime.c:14 <_runtime> - duration: 100000 us
[ 39.882814 part#1 proc#1 ] runtime.c:15 <_runtime> - lock level: 0
[ 39.882828 part#1 proc#1 ] runtime.c:16 <_runtime> - start cond: 0
[ 39.882842 part#1 proc#1 ] runtime.c:17 <_runtime> - assigned cores: 1
[ 39.883850 part#1 proc#2(1) ] ping_pong.c:8 <player1> I am player 1, and let me serve
[ 39.883872 part#1 proc#2(1) ] ping_pong.c:15 <player1> (round=1) send ball 0
[ 39.883947 part#1 proc#2(1) ] ping_pong.c:17 <player1> (round=1) wait ball with timeout 2 s...
[ 39.884018 part#1 proc#3(2) ] ping_pong.c:39 <player2> I am player 2
[ 39.884038 part#1 proc#3(2) ] ping_pong.c:49 <player2> (round=1) wait ball ...
[ 39.884058 part#1 proc#3(2) ] ping_pong.c:51 <player2> (round=1) got ball 0
[ 39.884073 part#1 proc#3(2) ] ping_pong.c:53 <player2> (round=1) send ball 1
[ 39.884102 part#1 proc#2(1) ] ping_pong.c:22 <player1> (round=1) got ball 1
[ 39.884119 part#1 proc#2(1) ] ping_pong.c:15 <player1> (round=2) send ball 2
[ 39.884140 part#1 proc#2(1) ] ping_pong.c:17 <player1> (round=2) wait ball with timeout 2 s...
[ 39.884200 part#1 proc#3(2) ] ping_pong.c:58 <player2> (round=2) wait ball ...
[ 39.884219 part#1 proc#3(2) ] ping_pong.c:60 <player2> (round=2) got ball 2
[ 39.884234 part#1 proc#3(2) ] ping_pong.c:61 <player2> (round=2) oh, ball lost
[ 41.884165 part#1 proc#2(1) ] ping_pong.c:29 <player1> (round=2) suspend timed out
[ 41.884185 part#1 proc#2(1) ] ping_pong.c:15 <player1> (round=3) send ball 3
[ 41.884203 part#1 proc#2(1) ] ping_pong.c:17 <player1> (round=3) wait ball with timeout 2 s...
[ 41.884265 part#1 proc#3(2) ] ping_pong.c:68 <player2> (round=3) wait ball ...
[ 41.884283 part#1 proc#3(2) ] ping_pong.c:70 <player2> (round=3) got ball 3
[ 41.884298 part#1 proc#3(2) ] ping_pong.c:71 <player2> (round=3) oh, ball sent but out of table
[ 41.884364 part#1 proc#3(2) ] ping_pong.c:75 <player2> (round=4) wait ball with timeout 4 s...
[ 43.884327 part#1 proc#2(1) ] ping_pong.c:24 <player1> (round=3) recv timed out
[ 43.884347 part#1 proc#2(1) ] ping_pong.c:15 <player1> (round=4) send ball 4
[ 43.884366 part#1 proc#2(1) ] ping_pong.c:17 <player1> (round=4) wait ball with timeout 2 s...
[ 43.884427 part#1 proc#3(2) ] ping_pong.c:77 <player2> (round=4) got ball 4
[ 43.884444 part#1 proc#3(2) ] ping_pong.c:79 <player2> (round=4) send ball 5
[ 43.884468 part#1 proc#2(1) ] ping_pong.c:22 <player1> (round=4) got ball 5
[ 43.884484 part#1 proc#2(1) ] halt at ping_pong.c:35 <player1> game over
[ 43.932894 hart#1 ] syscalls.c:65 <do_halt> shutdown from syscall
```
