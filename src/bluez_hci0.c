
/*
 * bluez_hci0.c
 * - 主要负责/org/bluez/hci0 path下的interface的操作
 * - 包括：
 *   org.freedesktop.Adapter1
 *       method: 
 *          Powered
 *          StartDiscovery
 *      Properties:
 *          Powered
 *          Discovering
 *   org.freedesktop.DBus.Porperties
 *       methods: 
 *          Set
 *       signals: 
 *          PropertiesChanged
 * - 功能包括：
 *   1. 给controller上电
 *   2. 设置扫描filter，只上报我们想得到的外设
 *   3. 开启扫描
 *   4. 订阅PropertiesChanged信号，得到是否Powered属性，是否开始扫描
 *
 */

#include "bluez.h"
#include "bluez_hci0.h"
#include "bluez_hci0_device.h"

static const gchar *interface_name = "org.bluez.Adapter1";
static  GDBusConnection *con;
static gchar *adapter_object_path = NULL;
static guint hci0_prop_changed = 0;



gchar* get_adapter_object_path()
{
    return adapter_object_path;
}

void set_adapter_object_path(gchar *path )
{
    adapter_object_path = malloc(strlen(path)+1);
    strcpy(adapter_object_path, path);
}


int bluez_adapter_set_property(const char *prop, GVariant *value)
{
    GVariant *result;
    GError *error = NULL;

    result = g_dbus_connection_call_sync(con,
                         "org.bluez",
                         adapter_object_path,
                         "org.freedesktop.DBus.Properties",
                         "Set",
                         g_variant_new("(ssv)", "org.bluez.Adapter1", prop, value),
                         NULL,
                         G_DBUS_CALL_FLAGS_NONE,
                         -1,
                         NULL,
                         &error);
    if(error != NULL)
        return 1;

    g_variant_unref(result);
    return 0;
}



int bluez_set_discovery_filter()
{
    int rc;
    GVariantBuilder *b = g_variant_builder_new(G_VARIANT_TYPE_VARDICT);

    //这里一定要使用Thermometer才行，只写Example不行
    g_variant_builder_add(b, "{sv}", "Pattern", g_variant_new_string(get_name()));

    GVariant *device_dict = g_variant_builder_end(b);
    g_variant_builder_unref(b);
    rc = bluez_call_method(con, adapter_object_path, interface_name, 
                "SetDiscoveryFilter", g_variant_new_tuple(&device_dict, 1), NULL);
    if(rc) {
        g_print("Not able to set discovery filter\n");
        return 1;
    }

    rc = bluez_call_method(con, adapter_object_path, interface_name, 
            "GetDiscoveryFilters",
            NULL,
            NULL);
    if(rc) {
        g_print("Not able to get discovery filter\n");
        return 1;
    }
    return 0;
}

void bluez_signal_adapter_changed(GDBusConnection *conn,
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
    while(g_variant_iter_next(properties, "{&sv}", &key, &value)) {
        if(!g_strcmp0(key, "Powered")) {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "b");
                goto done;
            }
            g_print("Adapter is Powered \"%s\"\n", g_variant_get_boolean(value) ? "on" : "off");
        }
        if(!g_strcmp0(key, "Discovering")) {
            if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
                g_print("Invalid argument type for %s: %s != %s", key,
                        g_variant_get_type_string(value), "b");
                goto done;
            }
            g_print("Adapter scan \"%s\"\n", g_variant_get_boolean(value) ? "on" : "off");
        }
    }
done:
    if(properties != NULL)
        g_variant_iter_free(properties);
    if(value != NULL)
        g_variant_unref(value);
}


void prog_quit_bluez_hci0(void)
{
    g_print("\n");
    if(adapter_object_path){
        free(adapter_object_path);
        g_print("free adapter_object_path\n");
    }
    if(hci0_prop_changed){
        g_dbus_connection_signal_unsubscribe(con, hci0_prop_changed);
        g_print("unsubscribe hci0_prop_changed\n");
    }
    
    if(con){
        g_object_unref(con);
        g_print("unref hci0 bluez connection\n");
    }
    g_print("\n");

}

void start_scan(void)
{

    int rc;
    con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
    if(con == NULL) {
        g_print("Not able to get connection to system bus\n");
        return;
    }

    //g_print("adapter path %s\n", get_adapter_object_path());
    gboolean ret = get_found_peripheral();
    if(ret == TRUE){
        g_print("device is added in %s, no need to scan again\n", get_device_object_path());
        return;
    }
    hci0_prop_changed = g_dbus_connection_signal_subscribe(
                    con,
                    "org.bluez",
                    "org.freedesktop.DBus.Properties",
                    "PropertiesChanged",
                    adapter_object_path,
                    NULL,
                    G_DBUS_SIGNAL_FLAGS_NONE,
                    bluez_signal_adapter_changed,
                    NULL,
                    NULL);

    //power on adapter                    
    rc = bluez_adapter_set_property("Powered", g_variant_new("b", TRUE));
    if(rc) {
        g_print("Not able to enable the adapter\n");
        return;
    }

    //设置扫描匹配名称
    rc = bluez_set_discovery_filter();
    if(rc){
        return;
    }
    
    //开始扫描
    rc = bluez_call_method(con, adapter_object_path, interface_name, 
                            "StartDiscovery", NULL, NULL);
    if(rc) {
        g_print("Not able to scan for new devices\n");
    }

    return;
}