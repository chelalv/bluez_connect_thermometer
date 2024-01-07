/*
* 通用的函数
*
*/
#include "utils.h"

/*
GVariant*
g_dbus_connection_call_sync (
  GDBusConnection* connection,
  const gchar* bus_name,
  const gchar* object_path,
  const gchar* interface_name,
  const gchar* method_name,
  GVariant* parameters,
  const GVariantType* reply_type,
  GDBusCallFlags flags,
  gint timeout_msec,
  GCancellable* cancellable,
  GError** error
)
*/

int bluez_call_method(GDBusConnection* connection, const gchar* object_path, const gchar* interface_name, const char *method, GVariant *param, method_cb_t method_cb)
{
    GError *error = NULL;

    g_dbus_connection_call(connection,
                        "org.bluez",
                        object_path,
                        interface_name,
                        method,
                        param,
                        NULL,
                        G_DBUS_CALL_FLAGS_NONE,
                        -1,
                        NULL,
                        method_cb,
                        &error);
    if(error != NULL)
        return 1;
    return 0;
}


void bluez_property_value(const gchar *key, GVariant *value)
{
    const gchar *type = g_variant_get_type_string(value);

    g_print("\t%s : ", key);
    switch(*type) {
        case 's':
            g_print("%s\n", g_variant_get_string(value, NULL));
            break;
        case 'b':
            g_print("%d\n", g_variant_get_boolean(value));
            break;
        case 'u':
            g_print("%d\n", g_variant_get_uint32(value));
            break;
        case 'a':
            if(g_strcmp0(type, "as") == 0){      
                g_print("\n");
                const gchar *uuid;
                GVariantIter i;
                g_variant_iter_init(&i, value);
                while(g_variant_iter_next(&i, "s", &uuid))
                    g_print("\t\t%s\n", uuid);
            }
            break;
        case 'n':
            g_print("%d\n", g_variant_get_int16(value));
            break;
        default:
            g_print("Other\n");
            break;
    }
}