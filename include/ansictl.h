#ifndef _ANSICTL_H_
#define _ANSICTL_H_

#define ANSI_NONE "0"
#define ANSI_FG_RED "31"
#define ANSI_FG_GREEN "32"
#define ANSI_FG_YELLOW "33"
#define ANSI_FG_BLUE "34"
#define ANSI_FG_MAGENTA "35"
#define ANSI_FG_CYAN "36"
#define ANSI_FG_WHITE "37"

#define ANSI_COLOR(color) "\033[" color "m"

#if CONFIG_COLOR
#define ANSI_COLOR_WRAP(color, str) ANSI_COLOR(color) str ANSI_COLOR(ANSI_NONE)
#else
#define ANSI_COLOR_WRAP(_, str) str
#endif

#endif
