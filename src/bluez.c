#include "utils.h"
#include "bluez.h"
#include "bluez_hci0.h"
#include "bluez_hci0_device.h"
#include "bluez_hci0_device_char.h"

static  GDBusConnection *con;
static  guint iface_added = 0;
static	guint iface_removed = 0;
static  gboolean found_peripheral = FALSE;
static  gboolean found_service = FALSE;
static  gboolean found_char = FALSE;


gboolean get_found_peripheral(void)
{
    return found_peripheral;
}

void set_found_peripheral(gboolean value)
{
    found_peripheral = value;
}

static void bluez_list_controllers_devices(void)
{
	GVariant *result = NULL;
	GVariantIter i;
	gchar *object_path;
	GVariant *ifaces_and_properties;

	GError *error = NULL;

	result = g_dbus_connection_call_sync(con,
					     "org.bluez",
					     "/",
					     "org.freedesktop.DBus.ObjectManager",
					     "GetManagedObjects",
					     NULL,
					     NULL,
					     G_DBUS_CALL_FLAGS_NONE,
					     -1,
					     NULL,
					     &error);
	if(error != NULL)
		return;


	/* Parse the result */
	if(result) {
		result = g_variant_get_child_value(result, 0);
		g_variant_iter_init(&i, result);
		//会解析到/org/bluez, /org/bluez/hci0, /org/bluez/hci0/dev_xx
		while(g_variant_iter_next(&i, "{&o@a{sa{sv}}}", &object_path, &ifaces_and_properties)) {
			g_print("--%s--\n", object_path);
			const gchar *interface_name;
			GVariant *properties;
			GVariantIter ii;
			g_variant_iter_init(&ii, ifaces_and_properties);
			while(g_variant_iter_next(&ii, "{&s@a{sv}}", &interface_name, &properties)) {
				if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "adapter")) {
					g_print("[ %s ]\n", object_path);
                    set_adapter_object_path(object_path);
 					const gchar *property_name;
					GVariantIter iii;
					GVariant *prop_val;
					g_variant_iter_init(&iii, properties);
					while(g_variant_iter_next(&iii, "{&sv}", &property_name, &prop_val))
						//bluez_property_value(property_name, prop_val);
					g_variant_unref(prop_val);
				}else if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device")) {
					g_print("[ %s ]\n", object_path);
 					const gchar *property_name;
					GVariantIter iii;
					GVariant *prop_val;
					g_variant_iter_init(&iii, properties);
					while(g_variant_iter_next(&iii, "{&sv}", &property_name, &prop_val)){
						//bluez_property_value(property_name, prop_val);
                    }
					g_variant_unref(prop_val);
				}
				g_variant_unref(properties);
			}
			g_variant_unref(ifaces_and_properties);
		}
		g_variant_unref(result);
	}
}

static void bluez_device_appeared(GDBusConnection *sig,
				const gchar *sender_name,
				const gchar *object_path,
				const gchar *interface,
				const gchar *signal_name,
				GVariant *parameters,
				gpointer user_data)
{
	(void)sig;
	(void)sender_name;
	(void)object_path;
	(void)interface;
	(void)signal_name;
	(void)user_data;

	GVariantIter *interfaces;
	const char *object;
	const gchar *interface_name;
	GVariant *properties;

	g_variant_get(parameters, "(&oa{sa{sv}})", &object, &interfaces);
    g_print("[ %s ]\n", object);
	while(g_variant_iter_next(interfaces, "{&s@a{sv}}", &interface_name, &properties)) {
		if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device")) {	
            g_print("\t[ %s ]\n", interface_name);		
			const gchar *property_name;
			GVariantIter i;
			GVariant *prop_val;
			g_variant_iter_init(&i, properties);
			while(g_variant_iter_next(&i, "{&sv}", &property_name, &prop_val))
				//bluez_property_value(property_name, prop_val);
                if(g_strcmp0(property_name, "Name") == 0){
                    if(g_strstr_len(g_variant_get_string(prop_val, NULL), -1, "Thermometer Example")) {
                        g_print("found peripheral\n");
                        found_peripheral = TRUE;
                    }
                }
			g_variant_unref(prop_val);
		}else if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "gattservice")) {
			g_print("\t[ %s ]\n", interface_name);
			const gchar *property_name;
			GVariantIter i;
			GVariant *prop_val;
			g_variant_iter_init(&i, properties);
			while(g_variant_iter_next(&i, "{&sv}", &property_name, &prop_val))
				//bluez_property_value(property_name, prop_val);
                if(g_strcmp0(property_name, "UUID") == 0){
                    if(g_strstr_len(g_variant_get_string(prop_val, NULL), -1, "00001809")) {
                        g_print("found service\n");
                        found_service = TRUE;
                    }
                }
			g_variant_unref(prop_val);
		}else if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "gattcharacteristic")) {
			g_print("\t[ %s ]\n", interface_name);
			const gchar *property_name;
			GVariantIter i;
			GVariant *prop_val;
			g_variant_iter_init(&i, properties);
			while(g_variant_iter_next(&i, "{&sv}", &property_name, &prop_val))
				//bluez_property_value(property_name, prop_val);
                if(g_strcmp0(property_name, "UUID") == 0){
                    if(g_strstr_len(g_variant_get_string(prop_val, NULL), -1, "00002a1c")) {
                        g_print("found char\n");
                        found_char = TRUE;
                    }
                }
			g_variant_unref(prop_val);
		}
		g_variant_unref(properties);
	}
    if(found_peripheral == TRUE){
        set_device_object_path(object);
        connect_peripheral();
        //g_dbus_connection_signal_unsubscribe(con, iface_added);
        //g_print("unref iface_added in bluez_device_appeared\n");
        //sem_post(get_device_mutex()); 
    }else if(found_char == TRUE){
        set_char_object_path(object);
        found_char = FALSE;
        communication();
    }
	return;
}

#define BT_ADDRESS_STRING_SIZE 18
static void bluez_device_disappeared(GDBusConnection *sig,
				const gchar *sender_name,
				const gchar *object_path,
				const gchar *interface,
				const gchar *signal_name,
				GVariant *parameters,
				gpointer user_data)
{
	(void)sig;
	(void)sender_name;
	(void)object_path;
	(void)interface;
	(void)signal_name;
    (void)user_data;

	GVariantIter *interfaces;
	const char *object;
	const gchar *interface_name;
	char address[BT_ADDRESS_STRING_SIZE] = {'\0'};

	g_variant_get(parameters, "(&oas)", &object, &interfaces);
	while(g_variant_iter_next(interfaces, "s", &interface_name)) {
		if(g_strstr_len(g_ascii_strdown(interface_name, -1), -1, "device")) {
			int i;
			char *tmp = g_strstr_len(object, -1, "dev_") + 4;

			for(i = 0; *tmp != '\0'; i++, tmp++) {
				if(*tmp == '_') {
					address[i] = ':';
					continue;
				}
				address[i] = *tmp;
			}
			g_print("\nDevice periperal %s removed\n", address);
		}
	}
	return;
}

void prog_quit(int sig)
{
    g_print("\nprog_quit\n");

    prog_quit_bluez_hci0_device_char();
    prog_quit_bluez_hci0_device();
    prog_quit_bluez_hci0();

    if(sig)
        g_print("quit from signal:%d\n",sig);
    if(iface_added){
        g_dbus_connection_signal_unsubscribe(con, iface_added);
        g_print("unref iface_added\n");
    }
    if(iface_removed){
        g_dbus_connection_signal_unsubscribe(con, iface_removed);
        g_print("unref iface_removed\n");
    }
    if(con){
        g_object_unref(con);
        g_print("unref root path connection\n");
    }

    exit(1);
}

void enable_bt(void)
{    
    signal(SIGINT,prog_quit);
    signal(SIGKILL,prog_quit);
    signal(SIGTERM,prog_quit);
    signal(SIGQUIT,prog_quit);

	con = g_bus_get_sync(G_BUS_TYPE_SYSTEM, NULL, NULL);
	if(con == NULL) {
		g_print("Not able to get connection to system bus\n");
		return;
	}

    bluez_list_controllers_devices();

	iface_added = g_dbus_connection_signal_subscribe(con,
							"org.bluez",
							"org.freedesktop.DBus.ObjectManager",
							"InterfacesAdded",
							"/",
							NULL,
							G_DBUS_SIGNAL_FLAGS_NONE,
							bluez_device_appeared,
							NULL,
							NULL);
							
	iface_removed = g_dbus_connection_signal_subscribe(con,
							"org.bluez",
							"org.freedesktop.DBus.ObjectManager",
							"InterfacesRemoved",
							"/",
							NULL,
							G_DBUS_SIGNAL_FLAGS_NONE,
							bluez_device_disappeared,
                            NULL,
							NULL);
    //sem_t *mutex;
    //mutex = get_device_mutex();
    //sem_init(mutex, 0, 0);

    start_scan();

    return;
}