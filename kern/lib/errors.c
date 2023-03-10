#include <kern/lib/errors.h>
#include <stddef.h>

static const char *messages[KER_ERR_MAX] = {
    [KER_SUCCESS] = "success",
    [KER_ARG_ER] = "args error",
    [KER_LOCK_ER] = "lock error",
    [KER_DTB_ER] = "device tree blob error",
    [KER_ELF_ER] = "elf error",
    [KER_DEV_ER] = "device error",
    [KER_MEM_ER] = "memory error",
    [KER_INT_ER] = "int error",
    [KER_SRL_ER] = "serial device error",
    [KER_ASID_ER] = "asid error",
    [KER_PART_ER] = "partition error",
    [KER_PROC_ER] = "process error",
    [KER_SCHED_ER] = "schedule error",
};

inline const char *msg_of(long err) {
  if (-err >= 0 && -err < KER_ERR_MAX) {
    return messages[-err];
  }
  return NULL;
}
