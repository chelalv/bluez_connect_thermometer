/*
 * bluez_connect_thermometer - In Raspberry Pi 3B+, act as a central, connect a peripheral(Silabs BG22) 
 * Author: Matt idle911@163.com
 * Example run: ./bluez_connect_thermometer
 * - This example scans for a device has a name Thermometer Example
 * - 代码分别对几个object path操作：根目录，/org/bluez/hci0, 
 *   /org/bluez/hci0/dev_xx_xx, /org/bluez/hci0/dev_xx_xx/charXXXX
 *   如果已经有device注册了，也需要scan。
 *   当开启scan后，扫描到BG22，就会在DBus下生成/org/hciX/dev_XX_YY_ZZ_AA_BB_CC object，
 *   里面会有org.bluez.device1 interface
 *   连接设备后，bluez会去自动discovering gatt，这时候也会添加很多object path，
 *   每一个服务，特征值，描述符都有一个object path，
 *   使能从设备上的notify。接收notification
 *   接收到10个notifications，就断连设备
 * - 
 */
#include "bluez.h"
#include <unistd.h>
#include "bluez_hci0_device_char.h"
#include "bluez_hci0_device.h"

int main(void)
{
    GMainLoop *loop;
    
    loop = g_main_loop_new(NULL, FALSE);
    //使能蓝牙
    enable_bt();
    //连接外设
    connect_peripheral();
    //使能notify
    communication();
    //在收到notification 10次后结束通信
    stop();

    g_main_loop_run(loop);
    return 0;
}