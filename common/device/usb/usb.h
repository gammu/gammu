/**
 * libusb helper functions
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */
#ifndef __device__usb__usb_h__
#define __device__usb__usb_h__

#include <gammu-types.h>
#include <gammu-statemachine.h>

/* Hack to avoid need for including libusb.h in this header, these are just pointers anyway */
#ifndef __LIBUSB_H__
#define libusb_context void
#define libusb_device_handle void
#define struct_libusb_device_descriptor void
#define libusb_device void
#else
#define struct_libusb_device_descriptor struct libusb_device_descriptor
#endif

typedef struct {
    libusb_context *context;
    libusb_device_handle *handle;
    int configuration;
    int control_iface;
    int control_altsetting;
    int data_iface;
    int data_altsetting;
    int data_idlesetting;
    unsigned char ep_read;
    unsigned char ep_write;
} GSM_Device_USBData;

typedef bool (*GSM_USB_Match_Function)(GSM_StateMachine *s, libusb_device *dev, struct_libusb_device_descriptor *desc);
#endif
