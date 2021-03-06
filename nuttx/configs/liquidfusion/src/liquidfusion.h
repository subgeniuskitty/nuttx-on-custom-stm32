#ifndef __CONFIGS_LIQUIDFUSIONL_SRC_LIQUIDFUSION_H
#define __CONFIGS_LIQUIDFUSIONL_SRC_LIQUIDFUSION_H

/****************************************************************************************************
 * Included Files
 ****************************************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>
#include <stdint.h>

/****************************************************************************************************
 * Pre-processor Definitions
 ****************************************************************************************************/
/* Liquid Fusion GPIO Configuration **********************************************************************/

/* LEDs
 *
 * -- ---- -------------- -------------------------------------------------------------------
 * PN NAME SIGNAL         NOTES
 * -- ---- -------------- -------------------------------------------------------------------
 */

#define GPIO_LED1       (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                         GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN6)
#define GPIO_LED2       (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                         GPIO_OUTPUT_CLEAR|GPIO_PORTC|GPIO_PIN7)
#define GPIO_LED3       (GPIO_OUTPUT|GPIO_CNF_OUTPP|GPIO_MODE_50MHz|\
                         GPIO_OUTPUT_CLEAR|GPIO_PORTB|GPIO_PIN15)

/****************************************************************************************************
 * Public Types
 ****************************************************************************************************/

/****************************************************************************************************
 * Public data
 ****************************************************************************************************/

#ifndef __ASSEMBLY__

/****************************************************************************************************
 * Public Functions
 ****************************************************************************************************/

/****************************************************************************
 * Name: board_userled_initialize
 ****************************************************************************/

void board_userled_initialize(void);

/****************************************************************************
 * Name: board_userled
 ****************************************************************************/

void board_userled(int led, bool ledon);

/****************************************************************************
 * Name: board_userled_all
 ****************************************************************************/

void board_userled_all(uint8_t ledset);

#endif /* __ASSEMBLY__ */
#endif /* __CONFIGS_LIQUIDFUSIONL_SRC_LIQUIDFUSION_H */
