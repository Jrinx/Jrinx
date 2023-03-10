# Jrinx

## 什么是 Jrinx？

Jrinx 是一个多分区、多核操作系统，其命名来源于 Arinc 与 Jos，也具有 **J**ust **R**un **I**n jri**NX** 的意义。

## 未来可能需要做的事情

- 使用伙伴系统算法管理 OS 使用的内存空间（目前的 `bare_alloc` 和 `kalloc` 都是线性分配，无法回收）
