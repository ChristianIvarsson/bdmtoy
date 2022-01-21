/**
  ******************************************************************************
  * @file    usb_desc.h
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   Descriptor Header for Virtual COM Port Device
  ****************************************************************************** */

#ifndef __USB_DESC_H
#define __USB_DESC_H

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define USB_DATA_SIZE              64
#define USB_INT_SIZE               8

#define USB_SIZ_DEVICE_DESC        18
#define USB_SIZ_CONFIG_DESC        67
#define USB_SIZ_STRING_LANGID      4
#define USB_SIZ_STRING_VENDOR      24 // 38 (string + 2)
#define USB_SIZ_STRING_PRODUCT     20 // 50
#define USB_SIZ_STRING_SERIAL      26

#define STANDARD_ENDPOINT_DESC_SIZE             0x09

/* Exported functions ------------------------------------------------------- */
extern const uint8_t usb_DeviceDescriptor[USB_SIZ_DEVICE_DESC];
extern const uint8_t usb_ConfigDescriptor[USB_SIZ_CONFIG_DESC];

extern const uint8_t usb_StringLangID[USB_SIZ_STRING_LANGID];
extern const uint8_t usb_StringVendor[USB_SIZ_STRING_VENDOR];
extern const uint8_t usb_StringProduct[USB_SIZ_STRING_PRODUCT];
extern uint8_t usb_StringSerial[USB_SIZ_STRING_SERIAL];

#endif /* __USB_DESC_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
