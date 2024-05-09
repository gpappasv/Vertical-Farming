#ifndef WIFI_APIS_H
#define WIFI_APIS_H

// --- includes ----------------------------------------------------------------
#include "wifi_config.h"

// --- defines -----------------------------------------------------------------

// --- functions declarations --------------------------------------------------
int wifi_apis_connect(void);
int wifi_apis_disconnect(void);

void wifi_apis_wifi_init(void);

#endif // WIFI_APIS_H
