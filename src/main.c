#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "server.h"
#include "flash_store.h"

int main() {
    stdio_init_all();

    if (cyw43_arch_init()) {
        return -1;
    }

    cyw43_arch_enable_sta_mode();

    char ssid[256], password[256];
    if(wifi_details_load(ssid, password) == WIFI_DETAILS_UNSET) {
        cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000);
        wifi_details_save(WIFI_SSID, WIFI_PASSWORD);
    } else {
        cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000);
    } 

    TCP_SERVER_T *state = tcp_server_init();

    if (!state) { return -1; }
    if (!tcp_server_open(state)) { return -1; }
    while(!state->complete) {
        sleep_ms(1000);
    }

    free(state);

    cyw43_arch_deinit();
}