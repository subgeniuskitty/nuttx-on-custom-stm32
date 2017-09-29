#include <stdint.h>
#include <stdbool.h>

/************************************************************************************
 * Name: board_userled_initialize
 *
 * Description:
 *   Initializes LED GPIO pins.
 *
 ************************************************************************************/

void board_userled_initialize(void);

/************************************************************************************
 * Name: board_userled
 *
 * Description:
 *   Set LED state
 *
 ************************************************************************************/

void board_userled(int led, bool ledon);
