#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

// Unique Devices IDs register set
#if defined (STM32F37X) || defined(STM32F303xC) || defined(STM32F303xE)

#define         ID1          (0x1FFFF7AC)
#define         ID2          (0x1FFFF7B0)
#define         ID3          (0x1FFFF7B4)

#else /*STM32F1x*/

#define         ID1          (0x1FFFF7E8)
#define         ID2          (0x1FFFF7EC)
#define         ID3          (0x1FFFF7F0)

#endif

#define USB_DISCONNECT                      GPIOD
#define USB_DISCONNECT_PIN                  GPIO_Pin_9
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD
#define RCC_APB2Periph_ALLGPIO            ( RCC_APB2Periph_GPIOA \
                                          | RCC_APB2Periph_GPIOB \
                                          | RCC_APB2Periph_GPIOC \
                                          | RCC_APB2Periph_GPIOD \
                                          | RCC_APB2Periph_GPIOE )
#endif /* __PLATFORM_CONFIG_H */
