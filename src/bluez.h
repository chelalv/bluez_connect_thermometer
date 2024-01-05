#ifndef BLUEZH
#define BLUEZH
#include "utils.h"

void enable_bt(void);
gboolean get_found_peripheral(void);
void set_found_peripheral(gboolean value);
void prog_quit(int sig);

#endif