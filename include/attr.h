#ifndef _ATTR_H_
#define _ATTR_H_

#define __aligned(x) __attribute__((aligned(x)))
#define __section(x) __attribute__((section(x)))
#define __used __attribute__((used))
#define __unused __attribute__((unused))
#define __warn_unused_result __attribute__((warn_unused_result))
#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))
#define __inline __attribute__((always_inline))
#define __weak __attribute__((weak))
#define __format(f, ...) __attribute__((format(f, ##__VA_ARGS__)))

#endif
