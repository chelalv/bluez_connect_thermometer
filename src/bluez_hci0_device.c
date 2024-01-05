
/*
 * bluez_device.c
 * - 主要负责/org/bluez/hci0/dev_xx_xx path下的interface的操作
 * - 包括：
 *   org.freedesktop.Adapter1
 *       method: 
 *          Disconnect
 *          
 *          
 *      Properties:
 *          Connected
 *          
 *   org.freedesktop.DBus.Porperties
 *       methods: 
 *          Get
 *          Set
 *       signals: 
 *          PropertiesChanged
 * - 功能包括：
 *   1. Disconnect 断连 BG22
 *   2. 订阅PropertiesChanged信号，得到是否Connected属性，
 *
 *
 *
 *    
 */
#include "bluez_hci0_device.h"
#include "bluez_hci0.h"
#include "bluez.h"
const char peripheral_name[] = "Thermometer Example";

static gchar *device_object_path = NULL; 
static GDBusConnection *con;
static const gchar *interface_name="org.bluez.Device1";
static guint device_connected_signal = 0;
static gboolean device_connected = FALSE;


const char *get_name(void)
{
	return peripheral_name;
}

gchar* get_device_object_path()
{
	return device_object_path;
}

void set_device_object_path(const gchar *path)
{
	device_object_path = malloc(strlen(path)+1);
    strcpy(device_object_path, path);
}


void bluez_signal_device_changed(GDBusConnection *conn,
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
		if(!g_strcmp0(key, "Connected")) {
			if(!g_variant_is_of_type(value, G_VARIANT_TYPE_BOOLEAN)) {
				g_print("Invalid argument type for %s: %s != %s", key,
						g_variant_get_type_string(value), "b");
				goto done;
			}
            gboolean ret = g_variant_get_boolean(value);
			g_print("Device is \"%s\"\n", ret ? "connected" : "disconnected");
            if(ret == TRUE){
                device_connected = TRUE;
            }else{
                device_connected = FALSE;
            }
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


void prog_quit_bluez_hci0_device(void)
{
    g_print("\n");

    if(device_connected == TRUE){
        int rc;
        rc = bluez_call_method(con, device_object_path, interface_name, 
                    "Disconnect", NULL, NULL);
        if(rc) {
            g_print("Not able to disconnect the device\n");
        }
        g_print("disconnect device\n");
    }
    if(device_connected_signal){
        g_dbus_connection_signal_unsubscribe(con, device_connected_signal);
        g_print("unsubscribe device_connected_signal\n");
    }
    if(con){
        g_object_unref(con);
        g_print("unref device bluez connection\n");
    }
    if(device_object_path){
        free(device_object_path);
        g_print("free device_object_path\n");
    }
    g_print("\n");

}

void connect_peripheral(void)
{
   gboolean ret = get_found_peripheral();
    if(ret == TRUE){
        g_print("device is added in %s, connect\n", get_device_object_path());
        set_found_peripheral(FALSE);

    }else{
        g_print("device is not added, try to scan\n");
        return;
    }
    
 
    con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if(con == NULL) {
		g_print("Not able to get connection to system bus\n");
		return;
	}

    device_connected_signal = g_dbus_connection_signal_subscribe(con,
                    "org.bluez",
                    "org.freedesktop.DBus.Properties",
                    "PropertiesChanged",
                    device_object_path,
                    NULL,
                    G_DBUS_SIGNAL_FLAGS_NONE,
                    bluez_signal_device_changed,
                    NULL,
                    NULL);


    int rc;
	rc = bluez_call_method(con, device_object_path, interface_name, 
                "Connect", NULL, NULL);
    if(rc) {
        g_print("Not able to connect the device\n");
    }
    return;
}