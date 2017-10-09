#ifndef __CONFIGS_LIQUIDFUSION_INCLUDE_BOARD_H
#define __CONFIGS_LIQUIDFUSION_INCLUDE_BOARD_H 1

/************************************************************************************
 * Included Files
 ************************************************************************************/

#include <nuttx/config.h>

#ifndef __ASSEMBLY__
# include <stdint.h>
#endif
#include "stm32_rcc.h"
#include "stm32_sdio.h"
#include "stm32.h"

#include <nuttx/arch.h>

/************************************************************************************
 * Pre-processor Definitions
 ************************************************************************************/

/* Clocking *************************************************************************/

/* HSI - 8 MHz RC factory-trimmed
 * LSI - 40 KHz RC (30-60KHz, uncalibrated)
 * HSE - On-board crystal frequency is 25MHz
 * LSE - 32.768 kHz
 */

#define STM32_BOARD_XTAL        25000000ul

#define STM32_HSI_FREQUENCY     8000000ul
#define STM32_LSI_FREQUENCY     40000
#define STM32_HSE_FREQUENCY     STM32_BOARD_XTAL
#define STM32_LSE_FREQUENCY     32768

/* PLL ouput is 72MHz */

#define STM32_PLL_PREDIV2       RCC_CFGR2_PREDIV2d5   /* 25MHz / 5 => 5MHz */
#define STM32_PLL_PLL2MUL       RCC_CFGR2_PLL2MULx8   /* 5MHz * 8  => 40MHz */
#define STM32_PLL_PREDIV1       RCC_CFGR2_PREDIV1d5   /* 40MHz / 5 => 8MHz */
#define STM32_PLL_PLLMUL        RCC_CFGR_PLLMUL_CLKx9 /* 8MHz * 9  => 72Mhz */
#define STM32_PLL_FREQUENCY     (72000000)

/* SYCLLK and HCLK are the PLL frequency */

#define STM32_SYSCLK_FREQUENCY  STM32_PLL_FREQUENCY
#define STM32_HCLK_FREQUENCY    STM32_PLL_FREQUENCY
#define STM32_BOARD_HCLK        STM32_HCLK_FREQUENCY  /* same as above, to satisfy compiler */

/* APB2 clock (PCLK2) is HCLK (72MHz) */

#define STM32_RCC_CFGR_PPRE2    RCC_CFGR_PPRE2_HCLK
#define STM32_PCLK2_FREQUENCY   STM32_HCLK_FREQUENCY
#define STM32_APB2_CLKIN        (STM32_PCLK2_FREQUENCY)   /* Timers 2-7, 12-14 */

/* APB2 timers 1 and 8 will receive PCLK2. */

#define STM32_APB2_TIM1_CLKIN   (STM32_PCLK2_FREQUENCY)
#define STM32_APB2_TIM8_CLKIN   (STM32_PCLK2_FREQUENCY)

/* APB1 clock (PCLK1) is HCLK/2 (36MHz) */

#define STM32_RCC_CFGR_PPRE1    RCC_CFGR_PPRE1_HCLKd2
#define STM32_PCLK1_FREQUENCY   (STM32_HCLK_FREQUENCY/2)

/* APB1 timers 2-7 will be twice PCLK1 */

#define STM32_APB1_TIM2_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM3_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM4_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM5_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM6_CLKIN   (2*STM32_PCLK1_FREQUENCY)
#define STM32_APB1_TIM7_CLKIN   (2*STM32_PCLK1_FREQUENCY)

/* MCO output driven by PLL3. From above, we already have PLL3 input frequency as:
 *
 *  STM32_PLL_PREDIV2 = 5, 25MHz / 5 => 5MHz
 */

#if defined(CONFIG_STM32_MII_MCO) || defined(CONFIG_STM32_RMII_MCO)
#  define BOARD_CFGR_MCO_SOURCE  RCC_CFGR_PLL3CLK      /* Source: PLL3 */
#  define STM32_PLL_PLL3MUL      RCC_CFGR2_PLL3MULx10  /* MCO 5MHz * 10 = 50MHz */
#endif

/* LED definitions ******************************************************************/
/* If CONFIG_ARCH_LEDS is not defined, then the user can control the LEDs in any
 * way.  The following definitions are used to access individual LEDs.
 */

/* LED index values for use with board_userled() */

#define BOARD_LED1          0
#define BOARD_LED2          1
#define BOARD_LED3          2
#define BOARD_NLEDS         3

/* LED bits for use with board_userled_all() */

#define BOARD_LED1_BIT      (1 << BOARD_LED1)
#define BOARD_LED2_BIT      (1 << BOARD_LED2)
#define BOARD_LED3_BIT      (1 << BOARD_LED3)

/* Button definitions ***************************************************************/
/* The onboard 'reset' button is hardwired to the reset pin.
 * Only the 'user' button is software interactable.
 */

#define BUTTON_KEY1         0
#define NUM_BUTTONS         1

#define BUTTON_USER         BUTTON_KEY1

#define BUTTON_KEY1_BIT     (1 << BUTTON_KEY1)

#define BUTTON_USER_BIT     BUTTON_KEY1_BIT

/* Pin selections ******************************************************************/
/* I2C
 *
 * -- ---- -------------- ----------------------------------------------------------
 * PN NAME SIGNAL         NOTES
 * -- ---- -------------- ----------------------------------------------------------
 * 58 PB6  I2C1_SCL       Requires !CONFIG_STM32_I2C1_REMAP
 * 59 PB7  I2C1_SDA
 */

#if defined(CONFIG_STM32_I2C1) && defined(CONFIG_STM32_I2C1_REMAP)
#  error "CONFIG_STM32_I2C1 must not have CONFIG_STM32_I2C1_REMAP"
#endif

/* DAC
 *
 * -- ---- -------------- ----------------------------------------------------------
 * PN NAME SIGNAL         NOTES
 * -- ---- -------------- ----------------------------------------------------------
 * 20 PA4  DAC_OUT1       Reference signal to linear regulator
 */

/* ADC
 *
 * -- ---- -------------- ----------------------------------------------------------
 * PN NAME SIGNAL         NOTES
 * -- ---- -------------- ----------------------------------------------------------
 * 21 PA5  ADC_IN1        GPIO_ADC12_IN5. Measures current through CC regulator.
 * 22 PA6  ADC_IN2        GPIO_ADC12_IN6. Measures temperature of power transistor.
 */

/* Ethernet
 *
 * -- ---- -------------- ----------------------------------------------------------
 * PN NAME SIGNAL         NOTES
 * -- ---- -------------- ----------------------------------------------------------
 * 24 PC4  MII_RXD0       Ethernet PHY.  Requires CONFIG_STM32_ETH_REMAP
 * 25 PC5  MII_RXD1       Ethernet PHY.  Requires CONFIG_STM32_ETH_REMAP
 * 26 PB0  MII_RXD2       Ethernet PHY
 * 27 PB1  MII_RXD3       Ethernet PHY
 * 29 PB10 MII_RXER       Ethernet PHY
 * 23 PA7  MII_RXDV       Ethernet PHY.  Requires CONFIG_STM32_ETH_REMAP
 * 15 PA1  MII_RXC        Ethernet PHY
 *
 * 11 PC3  MII_TXC        Ethernet PHY
 * 30 PB11 MII_TXEN       Ethernet PHY
 * 33 PB12 MII_TXD0       Ethernet PHY
 * 34 PB13 MII_TXD1       Ethernet PHY
 * 10 PC2  MII_TXD2       Ethernet PHY
 * 61 PB8  MII_TXD3
 *
 * 17 PA3  MII_COL        Ethernet PHY
 * 14 PA0  MII_CRS        Ethernet PHY.  Requires CONFIG_STM32_ETH_REMAP
 *
 *  9 PC1  MII_MDC        Ethernet PHY
 * 16 PA2  MII_MDIO       Ethernet PHY
 *
 *  8 PC0  MII_INT        Ethernet PHY
 */

#ifdef CONFIG_STM32_ETHMAC
#  ifndef CONFIG_STM32_ETH_REMAP
#    error "STM32 Ethernet requires CONFIG_STM32_ETH_REMAP"
#  endif
#  ifndef CONFIG_STM32_MII
#    error "STM32 Ethernet requires CONFIG_STM32_MII"
#  endif
#  ifndef CONFIG_STM32_MII_MCO
#    error "STM32 Ethernet requires CONFIG_STM32_MII_MCO"
#  endif
#endif

/************************************************************************************
 * Public Data
 ************************************************************************************/

#ifndef __ASSEMBLY__

#undef EXTERN
#if defined(__cplusplus)
#define EXTERN extern "C"
extern "C" {
#else
#define EXTERN extern
#endif

/************************************************************************************
 * Public Function Prototypes
 ************************************************************************************/
/************************************************************************************
 * Name: stm32_boardinitialize
 *
 * Description:
 *   All STM32 architectures must provide the following entry point.  This entry point
 *   is called early in the intitialization -- after all memory has been configured
 *   and mapped but before any devices have been initialized.
 *
 ************************************************************************************/

void stm32_boardinitialize(void);

/************************************************************************************
 * Chip ID functions
 *
 * Description:
 *   Non-standard functions to obtain chip ID information.
 *
 ************************************************************************************/

const char *stm32_getchipid(void);
const char *stm32_getchipid_string(void);

#undef EXTERN
#if defined(__cplusplus)
}
#endif

#endif /* __ASSEMBLY__ */
#endif /* __CONFIGS_LIQUIDFUSION_INCLUDE_BOARD_H */
