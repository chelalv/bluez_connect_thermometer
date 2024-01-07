#ifndef BLUEZDEVICEH
#define BLUEZDEVICEH

#include "utils.h"

gchar* get_device_object_path(void);
const char *get_name(void);
void set_device_object_path(const gchar *path);
void prog_quit_bluez_hci0_device(void);
void connect_peripheral();



#endif