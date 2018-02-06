#include "tc_iot_device_config.h"
#include "tc_iot_demo_light.h"
#include "tc_iot_export.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static tc_iot_demo_light g_light_status = {
    false, "colorful light", 0xFFFFFF, 100.00,
};

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

extern void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv);
int run_shadow(tc_iot_shadow_config * p_client_config);

static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}

static void operate_light(tc_iot_demo_light * light) {
    if (light->light_switch) {
        tc_iot_hal_printf(
                ANSI_COLOR_RED 
                "light status:[name=%s]|[switch:off]|[color rgb:%d]|[brightness:%f]" 
                ANSI_COLOR_RESET, 
                light->name,
                light->color,
                light->brightness
                );
    } else {
        tc_iot_hal_printf(
                ANSI_COLOR_RED 
                "light status:[name=%s]|[switch:on]|[color rgb:%d]|[brightness:%f]" 
                ANSI_COLOR_RESET, 
                light->name,
                light->color,
                light->brightness
                );
    }
}

void _on_message_received(tc_iot_message_data* md) {
    jsmntok_t  json_token[60];
    char json_buf[512];
    char field_buf[TC_IOT_LIGHT_NAME_LEN+1];
    int field_index = 0;
    const char * reported_start = NULL;
    int reported_len = 0;
    const char * desired_start;
    int desired_len = 0;
    int ret = 0;
    int i = 0;

    memset(field_buf, 0, sizeof(field_buf));
    memset(json_buf, 0, sizeof(json_buf));

    tc_iot_mqtt_message* message = md->message;
    tc_iot_hal_printf("[s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);

    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        return ;
    }

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "method", json_buf, sizeof(json_buf));
    if (field_index <= 0 ) {
        LOG_ERROR("field method not found in JSON: %.*s", (int)message->payloadlen, (char*)message->payload);
        return ;
    }

    if (strncmp("control", json_buf, strlen(json_buf)) == 0) {

        LOG_TRACE("Control data receved.");
        field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "payload.state.reported", json_buf, sizeof(json_buf));
        if (field_index <= 0 ) {
            LOG_TRACE("payload.state.reported not found");
        } else {
            reported_start = message->payload + json_token[field_index].start;
            reported_len = json_token[field_index].end - json_token[field_index].start;
            LOG_TRACE("payload.state.reported found:%.*s", reported_len, reported_start);
        }

        field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "payload.state.desired", json_buf, sizeof(json_buf));
        if (field_index <= 0 ) {
            LOG_TRACE("payload.state.desired not found");
        } else {
            desired_start = message->payload + json_token[field_index].start;
            desired_len = json_token[field_index].end - json_token[field_index].start;
            LOG_TRACE("payload.state.desired found:%.*s", desired_len, desired_start);

            ret = tc_iot_json_parse(desired_start,desired_len, json_token, TC_IOT_ARRAY_LENGTH(json_token));
            if (ret <= 0) {
                return ;
            }
            for (i = 0; i < ret; i++) {
                tc_iot_json_print_node("desired:", json_buf, json_token, i);
            }
        }
        LOG_TRACE("Control data processed.");

    } else if (strncmp("reply", json_buf, strlen(json_buf)) == 0) {
        LOG_TRACE("Reply pack recevied.");
    }

}

tc_iot_shadow_config g_client_config = {
    {
        {
            // device info
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
        },
        TC_IOT_CONFIG_SERVER_HOST,
        TC_IOT_CONFIG_SERVER_PORT,
        TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
        TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
        TC_IOT_CONFIG_CLEAN_SESSION,
        TC_IOT_CONFIG_USE_TLS,
        TC_IOT_CONFIG_AUTO_RECONNECT,
        TC_IOT_CONFIG_ROOT_CA,
        TC_IOT_CONFIG_CLIENT_CRT,
        TC_IOT_CONFIG_CLIENT_KEY,
        NULL,
        NULL,
    },
    TC_IOT_SUB_TOPIC_DEF,
    TC_IOT_PUB_TOPIC_DEF,
    _on_message_received,
};

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;
    bool token_defined;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    long nonce = tc_iot_hal_random();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    p_client_config = &(g_client_config.mqtt_client_config);
    parse_command(p_client_config, argc, argv);
    snprintf(g_client_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_client_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    token_defined = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!token_defined) {
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token(
                TC_IOT_CONFIG_AUTH_API_URL, TC_IOT_CONFIG_ROOT_CA,
                timestamp, nonce,
                &p_client_config->device_info);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    run_shadow(&g_client_config);

    return 0;
}

int run_shadow(tc_iot_shadow_config * p_client_config) {
    int timeout;
    int ret = 0;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char reported[256];
    char desired[256];
    tc_iot_shadow_client client;
    tc_iot_shadow_client* p_shadow_client = &client;

    tc_iot_hal_printf("constructing mqtt shadow client.\n");
    ret = tc_iot_shadow_construct(p_shadow_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("construct shadow client failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    tc_iot_hal_printf("construct mqtt shadow client success.\n");
    timeout = TC_IOT_CONFIG_COMMAND_TIMEOUT_MS;
    tc_iot_hal_printf("yield waiting for server push.\n");
    tc_iot_shadow_yield(p_shadow_client, timeout);
    tc_iot_hal_printf("yield waiting for server finished.\n");

    tc_iot_hal_printf("[c->s] shadow_get\n");
    tc_iot_shadow_get(p_shadow_client);
    tc_iot_shadow_yield(p_shadow_client, timeout);
    operate_light(&g_light_status);

    snprintf(reported, sizeof(reported), 
            "{\"name\":\"%s\",\"color\":%d,\"brightness\":%f,\"light_switch\":%s}",
            tc_iot_json_inline_escape(buffer, buffer_len, g_light_status.name),
            g_light_status.color,g_light_status.brightness,
            g_light_status.light_switch?TC_IOT_JSON_TRUE:TC_IOT_JSON_FALSE);
    ret =  tc_iot_shadow_doc_pack_for_update(buffer, buffer_len, p_shadow_client, reported, NULL);
    tc_iot_hal_printf("[c->s] shadow_update_reported\n%s\n", buffer);
    tc_iot_shadow_update(p_shadow_client, buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    while (!stop) {
        tc_iot_shadow_yield(p_shadow_client, 500);
    }

    tc_iot_hal_printf("Stopping\n");
    tc_iot_shadow_destroy(p_shadow_client);
    tc_iot_hal_printf("Exit success.\n");
    return 0;
}
