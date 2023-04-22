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
  - [ ] `CREATE_BLACKBOARD`
  - [ ] `DISPLAY_BLACKBOARD`
  - [ ] `READ_BLACKBOARD`
  - [ ] `CLEAR_BLACKBOARD`
  - [ ] `GET_BLACKBOARD_ID`
  - [ ] `GET_BLACKBOARD_STATUS`
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

- 实现异步的串口 I/O，提高串口性能
- 对内核的上下文切换、时钟中断等关键功能进行实时性分析，并做针对性优化
