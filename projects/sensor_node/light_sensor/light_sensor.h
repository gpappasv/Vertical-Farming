#ifndef BH1750_DRIVER_INCLUDED_H
#define BH1750_DRIVER_INCLUDED_H

// --- includes ----------------------------------------------------------------
#include <zephyr/types.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/i2c.h>

// --- function prototypes -----------------------------------------------------
const struct device *get_bh1750_device(void);

// --- defines -----------------------------------------------------------------
#define DT_DRV_COMPAT rohm_bh1750
#define BH1750_I2C_ADDRESS              DT_INST_REG_ADDR(0)

/* Commands */
#define BH1750_CMD_PWN_DN                           0x0
#define BH1750_CMD_PWR_ON                           0x1
#define BH1750_CMD_RESET                            0x3

/* Measurement Modes */
#define BH1750_MODE_CONTINUOUS                      0x10
#define BH1750_MODE_SINGLE                          0x20

/* Measurement Resolutions */
#define BH1750_HIGH_RES                             0x0
#define BH1750_HiGH_RES_TWO                         0x1
#define BH1750_LOW_RES                              0x2                   

#endif 

