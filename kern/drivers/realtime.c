#include <kern/drivers/cpus.h>
#include <kern/drivers/realtime.h>
#include <kern/lib/debug.h>
#include <kern/lib/regs.h>
#include <kern/mm/pmm.h>
#include <lib/hashmap.h>
#include <lib/string.h>

struct rtc_dev {
  char *rt_name;
  read_time_callback_t rt_read_time_callback;
  struct linked_node rt_link;
};

static struct rtc_dev *selected_rtc_dev = NULL;

static const void *rtc_key_of(const struct linked_node *node) {
  const struct rtc_dev *rtc = CONTAINER_OF(node, struct rtc_dev, rt_link);
  return rtc->rt_name;
}

static struct hlist_head rtc_map_array[32];
static struct hashmap rtc_map = {
    .h_array = rtc_map_array,
    .h_num = 0,
    .h_cap = 32,
    .h_code = hash_code_str,
    .h_equals = hash_eq_str,
    .h_key = rtc_key_of,
};

void rt_register_dev(char *name, read_time_callback_t read_time_callback) {
  struct rtc_dev *rtc = alloc(sizeof(struct rtc_dev), sizeof(struct rtc_dev));
  rtc->rt_name = name;
  rtc->rt_read_time_callback = read_time_callback;
  hashmap_put(&rtc_map, &rtc->rt_link);
  selected_rtc_dev = rtc;
}

int rt_select_dev(const char *name) {
  struct linked_node *node = hashmap_get(&rtc_map, name);
  if (node == NULL) {
    return 0;
  }
  struct rtc_dev *rtc = CONTAINER_OF(node, struct rtc_dev, rt_link);
  selected_rtc_dev = rtc;
  return 1;
}

int rt_read_time(uint64_t *re) {
  if (selected_rtc_dev == NULL) {
    return 0;
  }
  catch_e(cb_invoke(selected_rtc_dev->rt_read_time_callback)(re), { return 0; });
  return 1;
}

int sys_read_boot_time_sec_msec(uint64_t *sec, uint64_t *millisec) {
  static uint64_t boot_nanosec = 0;
  uint64_t time = r_time();
  if (boot_nanosec == 0) {
    boot_nanosec = time;
  }
  time -= boot_nanosec;
  if (cpus_get_timebase_freq() == 0) {
    *sec = 0;
    *millisec = 0;
    return 0;
  }
  *sec = time / cpus_get_timebase_freq();
  *millisec = time / (cpus_get_timebase_freq() / 1000);
  return 1;
}
