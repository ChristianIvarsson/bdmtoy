#ifndef __COMMON_H__
#define __COMMON_H__
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "stm32f10x_gpio.h"
#include "SupportFuncs.h"
#include "../shared/enums.h"
#include "../shared/cmddesc.h"

// Be warned:
// Assembler functions have their own definitions and some of them are taking liberties due to how the pins are currently configured
// Common for them to not mask off bit 7/15 (TDI) if the state is predetermined

// GPIO port B
#define privPORT    1

// Pins
#define privJCM     9 // JCOMP. Used to enable JTAG compatibility, active high
#define privTMS    10 // TMS
#define privRST    11 // Reset
#define privRDY    12 // Ready / Freeze
#define privCLK    13 // Clock / BKPT
#define privTDO    14 // TDO / DSO / DSDO (Receive bits from target)
#define privTDI    15 // TDI / DSI / DSDI (Sent bits to target)

// Macros for direct pin access
#define JCM_HI  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BSRR = (1 << privJCM))
#define JCM_LO  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BRR  = (1 << privJCM))

#define RST_HI  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BSRR = (1 << privRST))
#define RST_LO  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BRR  = (1 << privRST))

#define TMS_HI  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BSRR = (1 << privTMS))
#define TMS_LO  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BRR  = (1 << privTMS))

#define CLK_HI  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BSRR = (1 << privCLK))
#define CLK_LO  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BRR  = (1 << privCLK))

#define TDI_HI  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BSRR = (1 << privTDI))
#define TDI_LO  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->BRR  = (1 << privTDI))

#define TDO_RD  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->IDR & (1 << privTDO) ? 1 : 0)
#define TDO_RDs (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->IDR & (1 << privTDO)) // Same as above except that it can only be used for state detection (slightly faster)
#define RDY_RD  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->IDR & (1 << privRDY))
#define RST_RD  (((GPIO_TypeDef *) (GPIOA_BASE + (privPORT*0x400)))->IDR & (1 << privRST))

// Usb disconnect. PC12
// It's only used on the big board I use to debug the code. 
#define USB_DIS_HI  (((GPIO_TypeDef *) (GPIOA_BASE + (3*0x400)))->BSRR = (1 << 12))
#define USB_DIS_LO  (((GPIO_TypeDef *) (GPIOA_BASE + (3*0x400)))->BRR  = (1 << 12))

// Macros for HAL access
#define P_JCMP     privPORT, privJCM
#define P_TMS      privPORT, privTMS
#define P_Trst     privPORT, privRST
#define P_RDY      privPORT, privRDY
#define P_CLK      privPORT, privCLK
#define P_TDO      privPORT, privTDO
#define P_TDI      privPORT, privTDI

// Used by NEXUS compatible devices
// #define P_rstcfg   0,   8 // Pulled low during reset to force MCU to sample bootcfg pins
// #define P_bootcfg0 0,  0
// #define P_bootcfg1 0,  0

extern void     InitSPI(const spi_cfg_t *cfg);
extern uint16_t usb_sendData(const void *buffer);

#endif
