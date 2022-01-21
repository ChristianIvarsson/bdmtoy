/**
  ******************************************************************************
  * @file    usb_desc.c
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   Descriptors for Virtual Com Port Demo
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"

/* USB Standard Device Descriptor */
const uint8_t usb_DeviceDescriptor[] =
  {
    0x12,   // bLength
    USB_DEVICE_DESCRIPTOR_TYPE,     // bDescriptorType
    0x00,
    0x02,   // bcdUSB = 2.00
    0x02,   // bDeviceClass: CDC
    0x02,   // bDeviceSubClass
    0x02,   // bDeviceProtocol
    0x40,   // bMaxPacketSize0
    0xFF, 0xFF, // idVendor  =
    0x07, 0x01, // idProduct =
    0x00,
    0x02,   // bcdDevice = 2.00
       1,    // Index of string descriptor describing manufacturer
       2,    // Index of string descriptor describing product
       3,    // Index of string descriptor describing the device's serial number
    0x01    // bNumConfigurations
  };

const uint8_t usb_ConfigDescriptor[] =
  {
    // Configuration Descriptor
    0x09,   // bLength: Configuration Descriptor size
    USB_CONFIGURATION_DESCRIPTOR_TYPE,      // bDescriptorType: Configuration
    USB_SIZ_CONFIG_DESC,       // wTotalLength:no of returned bytes
    0x00,
    0x02,   // bNumInterfaces: 2 interface
    0x01,   // bConfigurationValue: Configuration value
    0x00,   // iConfiguration: Index of string descriptor describing the configuration
    0xC0,   // bmAttributes: self powered
    0x32,   // MaxPower 0 mA

    // Interface Descriptor
    0x09,   // bLength: Interface Descriptor size
    USB_INTERFACE_DESCRIPTOR_TYPE,  // bDescriptorType: Interface

    // Interface descriptor type
    0x00,   // bInterfaceNumber: Number of Interface
    0x00,   // bAlternateSetting: Alternate setting
    0x01,   // bNumEndpoints: One endpoints used
    0x02,   // bInterfaceClass: Communication Interface Class
    0x02,   // bInterfaceSubClass: Abstract Control Model
    0x01,   // bInterfaceProtocol: Common AT commands
    0x00,   // iInterface:

    // Header Functional Descriptor
    0x05,   // bLength: Endpoint Descriptor size
    0x24,   // bDescriptorType: CS_INTERFACE
    0x00,   // bDescriptorSubtype: Header Func Desc
    0x10,   // bcdCDC: spec release number
    0x01,

    // Call Management Functional Descriptor
    0x05,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x01,   // bDescriptorSubtype: Call Management Func Desc
    0x00,   // bmCapabilities: D0+D1
    0x01,   // bDataInterface: 1

    // ACM Functional Descriptor
    0x04,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x02,   // bDescriptorSubtype: Abstract Control Management desc
    0x02,   // bmCapabilities

    // Union Functional Descriptor
    0x05,   // bFunctionLength
    0x24,   // bDescriptorType: CS_INTERFACE
    0x06,   // bDescriptorSubtype: Union func desc
    0x00,   // bMasterInterface: Communication class interface
    0x01,   // bSlaveInterface0: Data Class Interface

    // Endpoint 2 Descriptor
    0x07,   // bLength: Endpoint Descriptor size
    USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType: Endpoint
    0x82,   // bEndpointAddress: (IN2)
    0x03,   // bmAttributes: Interrupt
    USB_INT_SIZE,      // wMaxPacketSize:
    0x00,
    0xFF,   // bInterval:

    // Data class interface descriptor
    0x09,   // bLength: Endpoint Descriptor size
    USB_INTERFACE_DESCRIPTOR_TYPE,  // bDescriptorType:
    0x01,   // bInterfaceNumber: Number of Interface
    0x00,   // bAlternateSetting: Alternate setting
    0x02,   // bNumEndpoints: Two endpoints used
    0x0A,   // bInterfaceClass: CDC
    0x00,   // bInterfaceSubClass:
    0x00,   // bInterfaceProtocol:
    0x00,   // iInterface:

    // Endpoint 3 Descriptor
    0x07,   // bLength: Endpoint Descriptor size
    USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType: Endpoint
    0x03,   // bEndpointAddress: (OUT3)
    0x02,   // bmAttributes: Bulk
    USB_DATA_SIZE,             // wMaxPacketSize:
    0x00,
    0x00,   // bInterval: ignore for Bulk transfer

    // Endpoint 1 Descriptor
    0x07,   // bLength: Endpoint Descriptor size
    USB_ENDPOINT_DESCRIPTOR_TYPE,   // bDescriptorType: Endpoint
    0x81,   // bEndpointAddress: (IN1)
    0x02,   // bmAttributes: Bulk
    USB_DATA_SIZE,             // wMaxPacketSize:
    0x00,
    0x00    // bInterval
  };

// USB String Descriptors
const uint8_t usb_StringLangID[USB_SIZ_STRING_LANGID] =
  {
    USB_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09,
    0x04 // LangID = 0x0409: U.S. English
  };

const uint8_t usb_StringVendor[USB_SIZ_STRING_VENDOR] =
  {
    USB_SIZ_STRING_VENDOR,     // Size of Vendor string
    USB_STRING_DESCRIPTOR_TYPE,             // bDescriptorType
    // Manufacturer: "STMicroelectronics"
    'T', 0, 'X', 0, 'S', 0, 'U', 0, 'I', 0, 'T', 0, 'E', 0, '.', 0,
    'O', 0, 'R', 0, 'G', 0,
  };

// 50 (string + 2)
const uint8_t usb_StringProduct[USB_SIZ_STRING_PRODUCT] =
  {
    USB_SIZ_STRING_PRODUCT,          /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
    /* Product name: "STM32 Virtual COM Port" */
    'N', 0, 'a', 0, 'm', 0, 'e', 0, '_', 0, 'M', 0, 'e', 0, '!', 0,
    '!', 0,
  };

uint8_t usb_StringSerial[USB_SIZ_STRING_SERIAL] =
  {
    USB_SIZ_STRING_SERIAL,           /* bLength */
    USB_STRING_DESCRIPTOR_TYPE,                   /* bDescriptorType */
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0
  };

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
