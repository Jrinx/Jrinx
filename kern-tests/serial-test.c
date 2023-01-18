#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/tests.h>
#include <lib/string.h>

#define SERIAL_TEST_BUFFER 512

static void serial_test(void) {
  size_t len = 0;
  char buffer[SERIAL_TEST_BUFFER];
  const char *std = "abcdefgh1234567890";
  info("test serial input/output, please input '%s' and press enter:\n", std);

  while (len < SERIAL_TEST_BUFFER) {
    buffer[len] = serial_blocked_getc();
    serial_blocked_putc(buffer[len]);
    if (buffer[len] == '\r') {
      serial_blocked_putc('\n');
      break;
    }
    len++;
  }
  buffer[len] = '\0';
  assert(strcmp(buffer, std) == 0);
}

static struct kern_test serial_testcase = {
    .kt_name = "serial-test",
    .kt_test_func = serial_test,
};

kern_test_def(serial_testcase);
