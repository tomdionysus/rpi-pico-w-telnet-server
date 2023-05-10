#ifndef FLASH_STORE
#define FLASH_STORE

#define WIFI_DETAILS_OK 0
#define WIFI_DETAILS_UNSET -1

int wifi_details_available();
int wifi_details_load(char *ssid, char *pwd);
int wifi_details_clear();
int wifi_details_save(char *ssid, char *pwd);

#endif