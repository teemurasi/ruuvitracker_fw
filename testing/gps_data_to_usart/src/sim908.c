#include "sim908.h"
// ChibiOS specific includes
#include "ch.h"
#include "hal.h"

/*static struct SerialConfig serial_cfg_3 = {
  0, //txend1,
  0, //txend2,
  rxend,
  0, //rxchar,
  0, //rxerr,
  38400,
  0,
  USART_CR2_LINEN,
  0
};*/

void initSIM908(void)
{
    palSetPad(GPIOC, GPIOC_PWRKEY); // Initial state
    

}

void SIM908_pwr_on(void)
{
    // Use 1sec trigger on PWRKEY pin to start the SIM908 module
    palClearPad(GPIOC, GPIOC_PWRKEY);
    chThdSleepMilliseconds(1000);
    palSetPad(GPIOC, GPIOC_PWRKEY);
}

