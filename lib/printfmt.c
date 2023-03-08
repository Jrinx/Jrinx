#include <lib/printfmt.h>

static void print_mem_range_style(fmt_callback_t, struct fmt_mem_range *, int, int, char);
static void print_bytes_style(fmt_callback_t, size_t);
static void print_char(fmt_callback_t, char, int, int);
static void print_str(fmt_callback_t, const char *, int, int);
static void print_num(fmt_callback_t, unsigned long, int, int, int, int, char, int);

void vprintfmt(fmt_callback_t out, const char *restrict fmt, va_list ap) {
  char c;
  const char *s;
  long num;

  int width;
  int long_flag;
  int neg_flag;
  int ladjust;
  char padc;

  for (;;) {
    int length = 0;
    s = fmt;
    for (; *fmt != '\0'; fmt++) {
      if (*fmt != '%') {
        length++;
      } else {
        cb_invoke(out)(s, length);
        length = 0;
        fmt++;
        break;
      }
    }

    cb_invoke(out)(s, length);

    if (!*fmt) {
      break;
    }

    ladjust = 0;
    padc = ' ';

    if (*fmt == '-') {
      ladjust = 1;
      padc = ' ';
      fmt++;
    } else if (*fmt == '0') {
      ladjust = 0;
      padc = '0';
      fmt++;
    }

    width = 0;
    while ((*fmt >= '0') && (*fmt <= '9')) {
      width = width * 10 + (*fmt) - '0';
      fmt++;
    }

    long_flag = 0;
    while (*fmt == 'l') {
      long_flag = 1;
      fmt++;
    }

    neg_flag = 0;
    switch (*fmt) {
    case 'b':
      if (long_flag) {
        num = va_arg(ap, long int);
      } else {
        num = va_arg(ap, int);
      }
      print_num(out, num, 2, 0, width, ladjust, padc, 0);
      break;

    case 'd':
    case 'D':
      if (long_flag) {
        num = va_arg(ap, long int);
      } else {
        num = va_arg(ap, int);
      }
      neg_flag = num < 0;
      num = neg_flag ? -num : num;
      print_num(out, num, 10, neg_flag, width, ladjust, padc, 0);
      break;

    case 'o':
    case 'O':
      if (long_flag) {
        num = va_arg(ap, long int);
      } else {
        num = va_arg(ap, int);
      }
      print_num(out, num, 8, 0, width, ladjust, padc, 0);
      break;

    case 'u':
    case 'U':
      if (long_flag) {
        num = va_arg(ap, long int);
      } else {
        num = va_arg(ap, int);
      }
      print_num(out, num, 10, 0, width, ladjust, padc, 0);
      break;

    case 'x':
      if (long_flag) {
        num = va_arg(ap, long int);
      } else {
        num = va_arg(ap, int);
      }
      print_num(out, num, 16, 0, width, ladjust, padc, 0);
      break;

    case 'X':
      if (long_flag) {
        num = va_arg(ap, long int);
      } else {
        num = va_arg(ap, int);
      }
      print_num(out, num, 16, 0, width, ladjust, padc, 1);
      break;

    case 'c':
      c = (char)va_arg(ap, int);
      print_char(out, c, width, ladjust);
      break;

    case 's':
      s = (char *)va_arg(ap, char *);
      print_str(out, s, width, ladjust);
      break;

    case 'p':
      fmt++;
      switch (*fmt) {
      case 'M':
        struct fmt_mem_range *mem_range = (struct fmt_mem_range *)va_arg(ap, void *);
        print_mem_range_style(out, mem_range, width, ladjust, padc);
        break;
      case 'B':
        size_t *bytes = (size_t *)va_arg(ap, void *);
        print_bytes_style(out, *bytes);
        break;
      default:
        fmt--;
        break;
      }
      break;
    case '\0':
      fmt--;
      break;

    default:
      cb_invoke(out)(fmt, 1);
    }
    fmt++;
  }
}

static void print_mem_range_style(fmt_callback_t out, struct fmt_mem_range *mem_range,
                                  int length, int ladjust, char padc) {
  cb_invoke(out)("[", sizeof(char));
  print_num(out, mem_range->addr, 16, 0, length, ladjust, padc, 0);
  cb_invoke(out)(", ", sizeof(char) * 2);
  print_num(out, mem_range->addr + mem_range->size, 16, 0, length, ladjust, padc, 0);
  cb_invoke(out)(")", sizeof(char));
}

static void _print_bytes_style(fmt_callback_t out, unsigned long size, size_t shift) {
  static const char *unit[] = {" B", " KiB", " MiB", " GiB", " TiB"};

  if (size == 0 && shift == 0) {
    cb_invoke(out)("0", sizeof(char));
    return;
  }

  unsigned long rem = size & ((1UL << 10) - 1);
  unsigned long quo = size >> 10;
  if (quo != 0) {
    _print_bytes_style(out, quo, shift + 1);
  }
  if (rem != 0) {
    if (quo != 0) {
      cb_invoke(out)(" + ", sizeof(char) * 3);
    }
    print_num(out, rem, 10, 0, 0, 0, ' ', 0);
    cb_invoke(out)(unit[shift], shift ? sizeof(char) * 4 : sizeof(char) * 2);
  }
}

static void print_bytes_style(fmt_callback_t out, size_t size) {
  _print_bytes_style(out, size, 0);
}

static void print_char(fmt_callback_t out, char c, int length, int ladjust) {
  int i;

  if (length < 1) {
    length = 1;
  }
  const char space = ' ';
  if (ladjust) {
    cb_invoke(out)(&c, 1);
    for (i = 1; i < length; i++) {
      cb_invoke(out)(&space, 1);
    }
  } else {
    for (i = 0; i < length - 1; i++) {
      cb_invoke(out)(&space, 1);
    }
    cb_invoke(out)(&c, 1);
  }
}

static void print_str(fmt_callback_t out, const char *s, int length, int ladjust) {
  int i;
  int len = 0;
  const char *s1 = s;
  while (*s1++) {
    len++;
  }
  if (length < len) {
    length = len;
  }

  if (ladjust) {
    cb_invoke(out)(s, len);
    for (i = len; i < length; i++) {
      cb_invoke(out)(" ", sizeof(char));
    }
  } else {
    for (i = 0; i < length - len; i++) {
      cb_invoke(out)(" ", sizeof(char));
    }
    cb_invoke(out)(s, len);
  }
}

static void print_num(fmt_callback_t out, unsigned long u, int base, int neg_flag, int length,
                      int ladjust, char padc, int upcase) {
  int actualLength = 0;
  char buf[length + 70];
  char *p = buf;
  int i;

  do {
    int tmp = u % base;
    if (tmp <= 9) {
      *p++ = '0' + tmp;
    } else if (upcase) {
      *p++ = 'A' + tmp - 10;
    } else {
      *p++ = 'a' + tmp - 10;
    }
    u /= base;
  } while (u != 0);

  if (neg_flag) {
    *p++ = '-';
  }

  actualLength = p - buf;
  if (length < actualLength) {
    length = actualLength;
  }

  if (ladjust) {
    padc = ' ';
  }
  if (neg_flag && !ladjust && (padc == '0')) {
    for (i = actualLength - 1; i < length - 1; i++) {
      buf[i] = padc;
    }
    buf[length - 1] = '-';
  } else {
    for (i = actualLength; i < length; i++) {
      buf[i] = padc;
    }
  }

  int begin = 0;
  int end;
  if (ladjust) {
    end = actualLength - 1;
  } else {
    end = length - 1;
  }

  while (end > begin) {
    char tmp = buf[begin];
    buf[begin] = buf[end];
    buf[end] = tmp;
    begin++;
    end--;
  }

  cb_invoke(out)(buf, length);
}
