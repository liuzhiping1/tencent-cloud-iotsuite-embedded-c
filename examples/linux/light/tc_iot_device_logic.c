#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"

void _device_on_message_received(tc_iot_message_data* md);
int _tc_iot_sync_shadow_property(tc_iot_shadow_property_def properties[], bool reported, const char * doc_start, jsmntok_t * json_token, int tok_count);
bool tc_iot_device_sync_reported(tc_iot_shadow_property_def properties[]);

void operate_device(tc_iot_shadow_local_data * device);

/* 设备状态数据 */
static tc_iot_shadow_local_data g_device_vars = 
{
    false,  /* 开关状态 */
    2, /* 设备光颜色控制 */
    100, /* 亮度 */
};

/* 设备本地数据类型及地址、回调函数等相关定义 */
tc_iot_shadow_property_def g_device_property_defs[] = {
    DECLARE_PROPERTY_DEF(device_switch, TC_IOT_SHADOW_TYPE_BOOL, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(color, TC_IOT_SHADOW_TYPE_ENUM, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(brightness, TC_IOT_SHADOW_TYPE_NUMBER, _tc_iot_shadow_property_control_callback),
};

int _tc_iot_shadow_property_control_callback(tc_iot_event_message *msg, void * client,  void * context) {
    char buffer[512];
    int buffer_len = sizeof(buffer);
    tc_iot_shadow_property_def * p_property = NULL;
    tc_iot_shadow_bool device_switch = false;
    tc_iot_shadow_enum color  = TC_IOT_PROP_color_red;
    tc_iot_shadow_number brightness  = 0;

    if (!msg) {
        LOG_ERROR("msg is null.");
        return TC_IOT_FAILURE;
    }

    if (msg->event == TC_IOT_SHADOW_EVENT_SERVER_CONTROL) {
        p_property = (tc_iot_shadow_property_def *)context;
        if (!p_property) {
            LOG_ERROR("p_property is null.");
            return TC_IOT_FAILURE;
        }
        switch (p_property->id) {
            case TC_IOT_PROP_device_switch:
                device_switch = *(tc_iot_shadow_bool *)msg->data;
                if (device_switch) {
                    LOG_TRACE("do something for device_switch on");
                } else {
                    LOG_TRACE("do something for device_switch off");
                }
                g_device_vars.device_switch = device_switch;
                break;
            case TC_IOT_PROP_color:
                color = *(tc_iot_shadow_enum *)msg->data;
                switch (color)
                {
                    case TC_IOT_PROP_color_red:
                        LOG_TRACE("do something for color red");
                        break;
                    case TC_IOT_PROP_color_green:
                        LOG_TRACE("do something for color green");
                        break;
                    case TC_IOT_PROP_color_blue:
                        LOG_TRACE("do something for color blue");
                        break;
                    default:
                        LOG_WARN("do something for color unkown.");
                        break;
                }
                g_device_vars.color = color;
                break;
            case TC_IOT_PROP_brightness:
                brightness = *(tc_iot_shadow_number *)msg->data;
                LOG_TRACE("do something for brightness=%d", brightness);
                g_device_vars.brightness = brightness;
                break;
            default:
                LOG_WARN("unkown property id = %d", p_property->id);
                return TC_IOT_SUCCESS;
        }
        tc_iot_shadow_update_state(&g_tc_iot_shadow_client, buffer, buffer_len,  
                NULL, TC_IOT_CONFIG_COMMAND_TIMEOUT_MS, NULL,
                g_device_property_defs, "reported", 1, p_property->id, msg->data);
        LOG_TRACE("operating device");
        operate_device(&g_device_vars);
    } else {
        LOG_TRACE("unkown event received, event=%ds", msg->event);
    }
    return TC_IOT_SUCCESS;
}

