#ifndef _ALIGNS_H_
#define _ALIGNS_H_

#define align_down(x, align) ((x) - ((x) % (align)))
#define align_up(x, align) align_down((x) + (align)-1, (align))

#endif
