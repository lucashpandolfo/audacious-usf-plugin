#include <stdint.h>
#ifndef _MAIN_H_
#define _MAIN_H_

#define CPU_Default       -1
#define CPU_Interpreter    0
#define CPU_Recompiler     1

void DisplayError(const char *Message, ...);
void StopEmulation(void);

#endif /*  */
