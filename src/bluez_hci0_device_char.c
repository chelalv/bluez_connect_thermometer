
/*
 * bluez_hci0_device_char.c
 * - 主要负责/org/bluez/hci0/dev_xx_xx/serviceXXXX/charXXXX path下的interface的操作
 * - 包括：
 *   org.org.bluez.GattCharacteristic1
 *       method: 
 *          StartNotify
 *      Properties:
 *          Notifying
 *          Value
 *   org.freedesktop.DBus.Porperties
 *       signals: 
 *          PropertiesChanged
 * - 功能包括：
 *   1. 开启notify
 *   2. 订阅PropertiesChanged信号，得到是否Notifying属性，
 *   3. 得到Value属性的值，也就是温度值
 *
 */

#include "utils.h"
#include "bluez.h"
#include "bluez_hci0_device_char.h"

static GDBusConnection *con;
static const gchar *interface_name="org.bluez.GattCharacteristic1";
static gchar *char_object_path = NULL; 
static gboolean found_char = FALSE;
static guint char_signal = 0;
static guint cnt = 0;


void set_char_object_path(const gchar *path)
{
    char_object_path = malloc(strlen(path)+1);
    strcpy(char_object_path, path);
    found_char = TRUE;
}

void bluez_signal_char_changed(GDBusConnection *conn,
                    const gchar *sender,
                    const gchar *path,
                    const gchar *interface,
                    const gchar *signal,
                    GVariant *params,
                    void *userdata)
{
    (void)conn;
    (void)sender;
    (void)path;
    (void)interface;
    (void)userdata;

    GVariantIter *properties = NULL;
    GVariantIter *unknown = NULL;
    const char *iface;
    const char *key;
    GVariant *value = NULL;
    const gchar *signature = g_variant_get_type_string(params);

    if(strcmp(signature, "(sa{sv}as)") != 0) {
        g_print("Invalid signature for %s: %s != %s", signal, signature, "(sa{sv}as)");
        goto done;
    }

    g_variant_get(params, "(&sa{sv}as)", &iface, &properties, &unknown);
    //g_print("%s\n", iface);
    while(g_variant_iter_next(properties, "{&sv}", &key, &value)) {
        if(!g_strcmp0(key, "NotifyAcquired")) {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "b");
                goto done;
            }
            g_print("NotifyAcquired \"%s\"\n", g_variant_get_boolean(value) ? "True" : "False");
        }else if(!g_strcmp0(key, "Notifying")) {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "b");
                goto done;
            }
            g_print("Notifying \"%s\"\n", g_variant_get_boolean(value) ? "True" : "False");
        }else if(!g_strcmp0(key, "Value")) {
            const gchar *type = g_variant_get_type_string(value);
            if(g_strcmp0(type, "ay") == 0){      
                g_print("New temperature value: ");
                gchar temp[5];
                gint16 j = 0;
                GVariantIter i;
                g_variant_iter_init(&i, value);
                while(g_variant_iter_next(&i, "y", &temp[j])){
                    //g_print("%02x", temp);
                    j++;
                }
                //解析温度值
                gint32 value = temp[1] | (temp[2] << 8) | (temp[3] << 16);
                g_print("%5.2f\n", (float)value/1000);
                cnt++;
                if(cnt == 10){
                    stop();
                }
            }
        }
    }
done:
    if(properties != NULL)
        g_variant_iter_free(properties);
    if(value != NULL)
        g_variant_unref(value);
}

void prog_quit_bluez_hci0_device_char(void)
{
    g_print("\n");
    if(char_signal){
        g_dbus_connection_signal_unsubscribe(con, char_signal);
        g_print("unsubscribe characteristic property changed signal\n");
    }
    if(con){
        g_object_unref(con);
        g_print("unref characteristic bluez connection\n");
    }
    if(char_object_path){
        free(char_object_path);
        g_print("free char_object_path\n");
    }
    g_print("\n");

}

void communication()
{
    if(found_char == TRUE){
        g_print("char is added in %s\n", char_object_path);
    }else{
        g_print("char is not added, wait\n");
        return;
    }

    con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if(con == NULL) {
        g_print("Not able to get connection to system bus\n");
        return;
    }

    char_signal = g_dbus_connection_signal_subscribe(con,
                    "org.bluez",
                    "org.freedesktop.DBus.Properties",
                    "PropertiesChanged",
                    char_object_path,
                    NULL,
                    G_DBUS_SIGNAL_FLAGS_NONE,
                    bluez_signal_char_changed,
                    NULL,
                    NULL);

    int rc;
    rc = bluez_call_method(con, char_object_path, interface_name, 
                "StartNotify", NULL, NULL);
    if(rc) {
        g_print("Not able to acquire notify\n");
    }
}

void stop(void)
{
    if(cnt < 10){
        return;
    }
    g_print("stop communication\n");
    prog_quit(6);
}