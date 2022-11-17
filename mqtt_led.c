#include "lwip/dns.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/apps/mqtt.h"

#define HIVEMQ_PORT (1883)
#define HIVEMQ_HOST "public.mqtthq.com"

struct mqtt_connect_client_info_t hivemq_client_info = {
        .client_id = "picow",
        .client_user = NULL,
        .client_pass = NULL,
        .keep_alive = 100,
        .will_msg = NULL,
        .will_topic =NULL
};
volatile int blink_period = 20;

ip_addr_t hiveIP;
static mqtt_client_t *mqtt_client;
void mqtt_request_callback(void *arg, err_t err) {
    if(err != ERR_OK) blink_period = 0;
}

void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    if(status == MQTT_CONNECT_ACCEPTED || status == MQTT_CONNECT_DISCONNECTED) {
        err_t statusSub = mqtt_sub_unsub(client, "hackatum2022-jetbrains", 1, mqtt_request_callback, LWIP_CONST_CAST(void*, mqtt_client),
                                         1);
        if(statusSub == ERR_OK)
        {
            blink_period = 100;
        } else {
            blink_period = 10;
        }
    } else {
        blink_period = 10;
    }
}

void hivemq_found_callback(const char *name, const ip_addr_t *ipaddr, void *callback_arg) {
    if (ipaddr == NULL) {
        blink_period = 10;
    } else {
    blink_period = 50;
    memcpy(&hiveIP,ipaddr, sizeof (ip4_addr_t));
    mqtt_client_connect(mqtt_client,
                        &hiveIP, HIVEMQ_PORT,
                        mqtt_connection_callback, LWIP_CONST_CAST(void*, &hivemq_client_info),
                        &hivemq_client_info);
    }
}


void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    if (data == NULL) {
        blink_period = 0;
    } else {
        blink_period = 200 * (*data - '0');
    }
}


void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len) {
    if (tot_len == 0) {
        blink_period = 0;
    }
}


int main() {
    stdio_init_all();
    if (cyw43_arch_init()) {
        printf("WiFi init failed");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    int result = cyw43_arch_wifi_connect_blocking("jetbrains-hackatum", "rpipicow", CYW43_AUTH_WPA2_MIXED_PSK);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, result ? 0 : 1);
    cyw43_arch_lwip_begin();

    dns_gethostbyname(HIVEMQ_HOST, &hiveIP, &hivemq_found_callback, NULL);
    mqtt_client = mqtt_client_new();

    mqtt_set_inpub_callback(mqtt_client,
                            mqtt_incoming_publish_cb,
                            mqtt_incoming_data_cb,
                            LWIP_CONST_CAST(void*, &hivemq_client_info));

    while (true) {
        if (blink_period == 0) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        } else {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(blink_period);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(blink_period);
        }
    }
}