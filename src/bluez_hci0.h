#ifndef BLUEZADAPTER
#define BLUEZADAPTER
#include "utils.h"

gchar* get_adapter_object_path(void);
void set_adapter_object_path(gchar *path);
void prog_quit_bluez_hci0(void);
void start_scan(void);



					
#endif				