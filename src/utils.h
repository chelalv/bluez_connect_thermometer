#ifndef BLUEZUTILS
#define BLUEZUTILS

#include <glib.h>
#include <gio/gio.h>
#include <pthread.h>
#include <errno.h> //errno and EEXIST
#include <semaphore.h>

typedef void (*method_cb_t)(GObject *, GAsyncResult *, gpointer);

void bluez_property_value(const gchar *key, GVariant *value);
int bluez_call_method(GDBusConnection* connection, const gchar* object_path, const gchar* interface_name, const char *method, GVariant *param, method_cb_t method_cb);


#endif