#ifndef INCLUDE_SOIL_MOISTURE_H
#define INCLUDE_SOIL_MOISTURE_H

// --- includes ----------------------------------------------------------------
#include <stdint.h>

// --- functions declarations --------------------------------------------------
int16_t adc_get_soil_moisture(void);
int16_t adc_measure_soil_moisture(void);
int16_t init_soil_adc_channel(void);
int16_t soil_moisture_dry_calibrate(void);
int16_t soil_moisture_wet_calibrate(void);
void init_soil_moisture_dry_wet(void);

#endif /* INCLUDE_SOIL_MOISTURE_H */