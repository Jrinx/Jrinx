#include <kern/drivers/realtime.h>
#include <kern/lib/debug.h>
#include <kern/mm/pmm.h>
#include <lib/string.h>
#include <sys/queue.h>

struct rtc_dev {
  char *rt_name;
  read_time_callback_t rt_read_time_callback;
  LIST_ENTRY(rtc_dev) rt_link;
};

static struct rtc_dev *selected_rtc_dev = NULL;
static LIST_HEAD(, rtc_dev) rtc_dev_list;

void rt_register_dev(const char *name, read_time_callback_t read_time_callback) {
  struct rtc_dev *rtc = alloc(sizeof(struct rtc_dev), sizeof(struct rtc_dev));
  size_t name_len = strlen(name);
  rtc->rt_name = alloc(sizeof(char) * (name_len + 1), sizeof(char));
  strcpy(rtc->rt_name, name);
  rtc->rt_read_time_callback = read_time_callback;
  LIST_INSERT_HEAD(&rtc_dev_list, rtc, rt_link);
  selected_rtc_dev = rtc;
}

int rt_select_dev(const char *name) {
  int found = 0;
  struct rtc_dev *rtc;
  LIST_FOREACH (rtc, &rtc_dev_list, rt_link) {
    if (strcmp(rtc->rt_name, name) == 0) {
      info("select %s as rtc\n", name);
      found = 1;
      selected_rtc_dev = rtc;
      break;
    }
  }
  return found;
}

int rt_read_time(uint64_t *re) {
  if (selected_rtc_dev == NULL) {
    return 0;
  }
  catch_e(cb_invoke(selected_rtc_dev->rt_read_time_callback)(re), { return 0; });
  return 1;
}

void rt_read_boot_time_sec_msec(uint64_t *sec, uint64_t *millisec) {
  static uint64_t boot_nanosec = 0;
  uint64_t nanosec = 0;
  if (!rt_read_time(&nanosec)) {
    *sec = 0;
    *millisec = 0;
    return;
  }
  if (boot_nanosec == 0) {
    boot_nanosec = nanosec;
  }
  nanosec -= boot_nanosec;
  *sec = nanosec / 1000000000UL;
  *millisec = nanosec / 1000000UL - (*sec * 1000UL);
}
