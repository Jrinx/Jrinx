# 外设探测初始化

在一般的外设代码中，一般会定义如下的设备结构体：

```c
static struct device some_device = {
    .d_probe = some_probe_func,
    .d_pred = some_pred_func,
};

device_init(some_device, medium);
```

其中，结构体的 `d_probe` 指定了此外设的探测逻辑（包括初始化），`d_pred` 指定了如何判定某个 DTB 结点是此外设。`device_init` 是一个定义于 [include/kern/drivers/device.h](../include/kern/drivers/device.h) 中的宏：

```c
#define device_init(dev, priority)                                                             \
  struct device *dev##_init __attribute__((section(".ksec.dev_init_" #priority "." #dev))) =   \
      &dev
```

该宏的第二个参数指示了该设备探测的优先级，优先级越高将越早被探测，同优先级设备的探测顺序是不确定的，目前在链接脚本 [kern.ld.S](../kern.ld.S) 中支持 `highest`、`high`、`medium`、`low`、`lowest` 五种优先级。上述代码的 `device_init` 预处理后展开为：

```c
struct device *some_device_init __attribute__((section(".ksec.dev_init_" "medium" "." "some_device"))) = &some_device;
```

其作用是将一个指向上述结构体的指针放置于 `.ksec.dev_init_*` 中。如上述代码，则最终将一个指针放入 `.ksec.dev_init_medium.some_device` 中。链接器会将 `.ksec.dev_init_*` 放到一段连续的内存空间中。`device_probe` 会顺序遍历此连续内存空间，从中取出这些结构体的指针，从而实现带有优先级的设备探测。
