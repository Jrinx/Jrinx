# Jrinx

## 什么是 Jrinx？

Jrinx 是一个基于 RISC-V 的 [ARINC 653](https://wikipedia.org/wiki/ARINC_653) 多分区操作系统。其命名来自于 ARINC 653 与 [JOS](https://pdos.csail.mit.edu/6.828/2014/overview.html)。

> 为什么提到了 ARINC 653：ARINC 653 是航空电子领域中的一个标准，定义了多分区操作系统的核心服务。本项目的目标是在 RISC-V 上实现 ARINC 653 的部分核心服务，为在 RISC-V 上建设 ARINC 653 多分区操作系统探路。
>
> 为什么提到了 JOS：作者 @Coekjan 在本科时期学习操作系统时，完成了 MOS（北京航空航天大学《操作系统》课程设计），而 MOS 的前身是 JOS。为致敬 JOS，本项目名以字母 J 开头。

## 快速开始

### 环境准备

- 安装基础构建工具 [GNU/Make](https://www.gnu.org/software/make/)、[GNU/Bash](https://www.gnu.org/software/bash/)
- 安装 [Python 3](https://www.python.org/)、[PyYAML](https://pyyaml.org/)。
- 安装 [RISC-V 64 位工具链](https://github.com/riscv-collab/riscv-gnu-toolchain)。
- 安装 [QEMU for RISC-V 64 bit](https://github.com/qemu/qemu)。

### 编译 Jrinx

克隆本代码仓库到本地，并进入仓库根目录，然后执行：

```console
$ make
```

以调试模式构建 Jrinx，或执行：

```console
$ make release
```

以发布模式构建 Jrinx。

### 运行 Jrinx

可执行（可按实际设置恰当的环境变量 `CROSS_COMPILE` 来指定交叉编译器）：

```console
$ make run
```

以在 QEMU 中直接运行 Jrinx，此时 Jrinx 将因没有需要调度的分区，而在初始化后停机。

为让 Jrinx 调度分区、进程，可指定 `SYSCONF` 环境变量，其值为某个系统配置文件的路径（系统配置文件均位于 `sys-conf` 下），使 Jrinx 依据该系统配置文件来调度分区、进程。例如：

```console
$ make run SYSCONF=./sys-conf/ping-pong.yml
```

另外，还可通过指定 `BOARD` 环境变量切换运行平台，目前仅支持 virt（默认选项）与 sifive_u 这两个平台。

## 项目目录结构

```plaintext
.
├── .vscode/
├── docs/               # 项目文档
├── include/            # 头文件集
├── kern/
│   ├── boot/           # 内核启动
│   ├── chan/           # 分区间通信
│   ├── comm/           # 分区内通信
│   ├── drivers/        # 设备树解析与设备驱动
│   ├── lib/            # 内核函数库
│   ├── lock/           # 锁
│   ├── mm/             # 内存管理
│   ├── multitask/      # 分区、进程、调度管理
│   ├── tests/          # 内核单元测试集
│   ├── traps/          # 异常、中断处理
│   └── Makefile        # 内核 Makefile
├── lib/                # 公共函数库
├── mk/                 # 项目 Makefile 集
├── opensbi/            # OpenSBI 依赖
├── scripts/            # 项目脚本集
├── sys-conf/           # 系统配置文件集
├── tests-conf/         # 测试配置文件集
├── user/               # 用户程序集
├── .clang-format       # Clang 格式化配置文件
├── .editorconfig       # 编辑器配置文件
├── .gitignore
├── .gitlab-ci.yml      # GitLab CI 配置文件
├── .gitmodules
├── Dockerfile          # CI 使用的 Docker 镜像
├── jrinx.logo          # Jrinx 的 LOGO
├── kern.ld.S           # 内核链接脚本
├── Makefile            # 项目根目录的 Makefile
└── README.md           # 本文件
```

## 现已基本实现的 ARINC 653 核心服务

- 分区管理：
  - [x] `GET_PARTITION_STATUS`
  - [x] `SET_PARTITION_MODE`
- 进程管理：
  - [x] `GET_PROCESS_ID`
  - [x] `GET_PROCESS_STATUS`
  - [x] `CREATE_PROCESS`
  - [x] `SET_PRIORITY`
  - [x] `SUSPEND_SELF`
  - [x] `SUSPEND`
  - [x] `RESUME`
  - [x] `STOP_SELF`
  - [x] `STOP`
  - [x] `START`
  - [x] `DELAYED_START`
  - [ ] `LOCK_PREEMPTION`
  - [ ] `UNLOCK_PREEMPTION`
  - [x] `GET_MY_ID`
  - [ ] `INITIALIZE_PROCESS_CORE_AFFINITY`
  - [ ] `GET_MY_PROCESSOR_CORE_ID`
  - [x] `GET_MY_INDEX`
- 时间管理：
  - [x] `TIMED_WAIT`
  - [ ] `PERIODIC_WAIT`
  - [x] `GET_TIME`
  - [ ] `REPLENISH`
- 分区间通信：
  - [ ] `CREATE_SAMPLING_PORT`
  - [ ] `WRITE_SAMPLING_MESSAGE`
  - [ ] `READ_SAMPLING_MESSAGE`
  - [ ] `GET_SAMPLING_PORT_ID`
  - [ ] `GET_SAMPLING_PORT_STATUS`
  - [x] `CREATE_QUEUING_PORT`
  - [x] `SEND_QUEUING_MESSAGE`
  - [x] `RECEIVE_QUEUING_MESSAGE`
  - [x] `GET_QUEUING_PORT_ID`
  - [x] `GET_QUEUING_PORT_STATUS`
  - [x] `CLEAR_QUEUING_PORT`
- 分区内通信：
  - [x] `CREATE_BUFFER`
  - [x] `SEND_BUFFER`
  - [x] `RECEIVE_BUFFER`
  - [x] `GET_BUFFER_ID`
  - [x] `GET_BUFFER_STATUS`
  - [x] `CREATE_BLACKBOARD`
  - [x] `DISPLAY_BLACKBOARD`
  - [x] `READ_BLACKBOARD`
  - [x] `CLEAR_BLACKBOARD`
  - [x] `GET_BLACKBOARD_ID`
  - [x] `GET_BLACKBOARD_STATUS`
  - [ ] `CREATE_SEMAPHORE`
  - [ ] `WAIT_SEMAPHORE`
  - [ ] `SIGNAL_SEMAPHORE`
  - [ ] `GET_SEMAPHORE_ID`
  - [ ] `GET_SEMAPHORE_STATUS`
  - [ ] `CREATE_EVENT`
  - [ ] `SET_EVENT`
  - [ ] `RESET_EVENT`
  - [ ] `WAIT_EVENT`
  - [ ] `GET_EVENT_ID`
  - [ ] `GET_EVENT_STATUS`
  - [ ] `CREATE_MUTEX`
  - [ ] `ACQUIRE_MUTEX`
  - [ ] `RELEASE_MUTEX`
  - [ ] `RESET_MUTEX`
  - [ ] `GET_MUTEX_ID`
  - [ ] `GET_MUTEX_STATUS`
  - [ ] `GET_PROCESS_MUTEX_STATE`

## 未来需要做的事情

- 对内核的上下文切换、时钟中断等关键功能进行实时性分析，并做针对性优化
- 支持周期进程、进程 deadline 检测等
- 完整实现 ARINC 653 定义的所有核心服务
- 支持多核场景下的分区、进程调度
- 支持更丰富的外设
