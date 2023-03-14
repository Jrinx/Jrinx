# Jrinx

## 什么是 Jrinx？

Jrinx 是一个多分区、多核操作系统，其命名来源于 ARINC 653 与 Jos，也具有 **J**ust **R**un **I**n jri**NX** 的意义。

## 未来需要做的事情

- 使用伙伴系统算法管理 OS 使用的内存空间（目前的 `bare_alloc` 和 `kalloc` 都是线性分配，无法回收），避免内存泄漏
- 使用系统调用机制，实现 ARINC 653 中的核心服务
- 使用 ASID Generation 技术，实现不受体系结构限制的 ASID 分配机制
- 实现异步的串口 I/O，提高串口性能
