#include <kern/drivers/serialport.h>
#include <kern/lib/debug.h>
#include <kern/lib/regs.h>
#include <kern/tests.h>
#include <lib/string.h>

static void serial_test(void) {
  const char std[] = "abcdefgh1234567890";
  size_t len = 0;
  char buffer[sizeof(std)];
  info("test serial int & i/o, please input '%s' and press enter:\n", std);

  for (rv64_si sip = {.val = csrr_sip()}; !sip.bits.sei; sip.val = csrr_sip()) {
  }

  while (len < sizeof(std)) {
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
