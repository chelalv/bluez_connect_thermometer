#ifndef BLUEZDEVICEH
#define BLUEZDEVICEH

#include "utils.h"

gchar* get_device_object_path(void);
const char *get_name(void);
void set_device_object_path(const gchar *path);
void prog_quit_bluez_hci0_device(void);
void connect_peripheral();
//sem_t *get_device_mutex(void);



#if 0
int bluez_device_call_method(const char *object_path, const char *method, GVariant *param, method_cb_t method_cb);
void bluez_signal_device_changed(GDBusConnection *conn,
					const gchar *sender,
					const gchar *path,
					const gchar *interface,
					const gchar *signal,
					GVariant *params,
					void *userdata);
#endif

#endif