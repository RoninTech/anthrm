

#include "conio.h"

extern "C" int __cdecl _kbhit (void);

int kbhit(void)
{
  return _kbhit();
}

