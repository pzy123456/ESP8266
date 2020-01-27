
#include "gpio.h"
#include "relay.h"
//#include "esp_common.h"
/******************************************************************************
 * FunctionName : Relay_Control
 * 控制继电器的开关
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void Relay_Control(uint16 mask)
{
		GPIO_OUTPUT(GPIO_Pin_5, mask);
}

