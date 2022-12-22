#ifndef JRINX
#error macro JRINX not defined
#endif

target extended-remote :1234
add-inferior
inferior 2
attach 2
i threads
set schedule-multiple on
file JRINX
