#include <kern/drivers/realtime.h>
#include <kern/lib/debug.h>
#include <kern/mm/pmm.h>
#include <lib/string.h>
#include <sys/queue.h>

struct realtime_rtc {
  char *rt_name;
  read_time_callback_t rt_read_time_callback;
  LIST_ENTRY(realtime_rtc) rt_link;
};

static struct realtime_rtc *selected_rtc = NULL;
static LIST_HEAD(, realtime_rtc) realtime_rtc_list;

void realtime_register_rtc(const char *name, read_time_callback_t read_time_callback) {
  struct realtime_rtc *rtc = alloc(sizeof(struct realtime_rtc), sizeof(struct realtime_rtc));
  size_t name_len = strlen(name);
  rtc->rt_name = alloc(sizeof(char) * (name_len + 1), sizeof(char));
  strcpy(rtc->rt_name, name);
  rtc->rt_read_time_callback = read_time_callback;
  LIST_INSERT_HEAD(&realtime_rtc_list, rtc, rt_link);
  selected_rtc = rtc;
}

int realtime_select_rtc(const char *name) {
  int found = 0;
  struct realtime_rtc *rtc;
  LIST_FOREACH (rtc, &realtime_rtc_list, rt_link) {
    if (strcmp(rtc->rt_name, name) == 0) {
      info("select %s as rtc\n", name);
      found = 1;
      selected_rtc = rtc;
      break;
    }
  }
  return found;
}

int realtime_read_time(uint64_t *re) {
  if (selected_rtc == NULL) {
    return 0;
  }
  catch_e(cb_invoke(selected_rtc->rt_read_time_callback)(re), { return 0; });
  return 1;
}

void realtime_read_boot_time_sec_millisec(uint64_t *sec, uint64_t *millisec) {
  static uint64_t boot_nanosec = 0;
  uint64_t nanosec = 0;
  if (!realtime_read_time(&nanosec)) {
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
