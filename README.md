# Jrinx

## 什么是 Jrinx？

Jrinx 是一个多分区、多核操作系统，其命名来源于 ARINC 653 与 Jos，也具有 **J**ust **R**un **I**n jri**NX** 的意义。

## ARINC 653 核心服务

- 分区管理：
  - [x] `GET_PARTITION_STATUS`
  - [x] `SET_PARTITION_MODE`：待完善
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
  - [ ] `TIMED_WAIT`
  - [ ] `PERIODIC_WAIT`
  - [x] `GET_TIME`
  - [ ] `REPLENISH`

## 未来需要做的事情

- 实现异步的串口 I/O，提高串口性能
