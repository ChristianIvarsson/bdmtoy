#ifndef __HW_CONFIG_H
#define __HW_CONFIG_H

#include "platform_config.h"
#include "usb_type.h"

void Set_System(void);
void Set_USBClock(void);
void USB_Interrupts_Config(void);
void USB_Cable_Config (FunctionalState NewState);
void Handle_USBAsynchXfer (void);
void Get_SerialNum(void);

/* External variables --------------------------------------------------------*/

#endif
