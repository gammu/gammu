/**
 * libusb helper functions
 *
 * Part of Gammu project
 *
 * Copyright (C) 2009 Michal Čihař
 *
 * Licensed under GNU GPL version 2 or later
 */

#include <libusb.h>
#include "usb.h"

#include "../../gsmstate.h"
#include "../../gsmcomon.h"

/* Nokia is the vendor we are interested in */
#define NOKIA_VENDOR_ID 0x0421

/* CDC class and subclass types */
#define USB_CDC_CLASS           0x02
#define USB_CDC_FBUS_SUBCLASS       0xfe


GSM_Error GSM_USB_Probe(GSM_StateMachine *s, GSM_USB_Match_Function matcher)
{
	libusb_device **devs;
	libusb_device *dev;
	GSM_Device_USBData *d = &s->Device.Data.USB;
	ssize_t cnt;
	int rc;
	int i = 0;
	struct libusb_device_descriptor desc;
	GSM_Error error;

	cnt = libusb_get_device_list(d->context, &devs);
	if (cnt < 0) {
		smprintf(s, "Failed to list USB devices (%d)!\n", (int)cnt);
		return ERR_UNKNOWN;
	}

	while ((dev = devs[i++]) != NULL) {
		rc = libusb_get_device_descriptor(dev, &desc);
		if (rc < 0) {
			smprintf(s, "Failed to get device descriptor (%d)!\n", rc);
			continue;
		}

		smprintf(s, "Checking %04x:%04x (bus %d, device %d)\n",
			desc.idVendor, desc.idProduct,
			libusb_get_bus_number(dev), libusb_get_device_address(dev));

		/* TODO: add optional matching by ids based on device name from configuration */

		if (matcher(s, dev, &desc)) {
			break;
		}
	}

	if (dev == NULL) {
		error = ERR_DEVICENOTEXIST;
		goto done;
	}

	smprintf(s, "Trying to open device, config=%d, iface=%d, alt=%d\n",
		d->configuration, d->iface, d->altsetting);

	rc = libusb_open(dev, &d->handle);
	if (rc != 0) {
		d->handle = NULL;
		error = ERR_DEVICEOPENERROR;
		goto done;
	}

	rc = libusb_set_configuration(d->handle, d->configuration);
	if (rc != 0) {
		smprintf(s, "Failed to set device configuration %d (%d)!\n", d->configuration, rc);
		libusb_close(d->handle);
		d->handle = NULL;
		error = ERR_DEVICEOPENERROR;
		goto done;
	}

	rc = libusb_claim_interface(d->handle, d->iface);
	if (rc != 0) {
		smprintf(s, "Failed to set claim interface %d (%d)!\n", d->iface, rc);
		libusb_close(d->handle);
		d->handle = NULL;
		error = ERR_DEVICEOPENERROR;
		goto done;
	}

	rc = libusb_set_interface_alt_setting(d->handle, d->iface, d->altsetting);
	if (rc != 0) {
		smprintf(s, "Failed to set alt setting %d (%d)!\n", d->altsetting, rc);
		libusb_close(d->handle);
		d->handle = NULL;
		error = ERR_DEVICEOPENERROR;
		goto done;
	}

	error = ERR_NONE;

done:

	return error;
}

GSM_Error GSM_USB_Init(GSM_StateMachine *s)
{
	GSM_Device_USBData *d = &s->Device.Data.USB;
	int rc;

	d->handle = NULL;

	rc = libusb_init(&d->context);
	if (rc != 0) {
		d->context = NULL;
		smprintf(s, "Failed to init libusb (%d)!\n", rc);
		return ERR_UNKNOWN;
	}

	return ERR_NONE;
}

GSM_Error GSM_USB_Terminate(GSM_StateMachine *s)
{
	GSM_Device_USBData *d = &s->Device.Data.USB;

	if (d->handle != NULL) {
		libusb_close(d->handle);
	}

	libusb_exit(d->context);

	d->handle = NULL;
	d->context = NULL;

	return ERR_NONE;
}

int GSM_USB_Read(GSM_StateMachine *s, void *buf, size_t nbytes)
{
	return 0;
}

int GSM_USB_Write(GSM_StateMachine *s, const void *buf, size_t nbytes)
{
	return 0;
}

bool FBUSUSB_Match(GSM_StateMachine *s, libusb_device *dev, struct libusb_device_descriptor *desc)
{
	int c, i, a;
	int rc;
	struct libusb_config_descriptor *config;
	GSM_Device_USBData *d = &s->Device.Data.USB;

	/* We care only about Nokia */
	if (desc->idVendor != NOKIA_VENDOR_ID) return false;

	/* Find configuration we want */
	for (c = 0; c < desc->bNumConfigurations; c++) {
		rc = libusb_get_config_descriptor(dev, c, &config);
		if (rc != 0) return false;
		/* Find interface we want */
		for (i = 0; i < config->bNumInterfaces; i++) {
			for (a = 0; a < config->interface[i].num_altsetting; a++) {
				/* We want only CDC FBUS settings */
				if (config->interface[i].altsetting[a].bInterfaceClass == USB_CDC_CLASS
						&& config->interface[i].altsetting[a].bInterfaceSubClass == USB_CDC_FBUS_SUBCLASS
					) {
					/* We have it */
					goto found;
				}
			}
		}
		libusb_free_config_descriptor(config);
	}
	return false;
found:
	/* Remember configuration which is interesting */
	d->configuration = config->bConfigurationValue;
	d->iface = config->interface[i].altsetting[a].bInterfaceNumber;
	d->altsetting = config->interface[i].altsetting[a].bAlternateSetting;

	/* FIXME: Find out descriptors */

	/* Free config descriptor */
	libusb_free_config_descriptor(config);
	return true;
}

GSM_Error FBUSUSB_Open(GSM_StateMachine *s)
{
	GSM_Error error;

	error = GSM_USB_Init(s);
	if (error != ERR_NONE) return error;

	error = GSM_USB_Probe(s, FBUSUSB_Match);
	if (error != ERR_NONE) return error;

	return ERR_NONE;
}


GSM_Device_Functions FBUSUSBDevice = {
    	FBUSUSB_Open,
    	GSM_USB_Terminate,
	NONEFUNCTION,
	NONEFUNCTION,
	NONEFUNCTION,
    	GSM_USB_Read,
    	GSM_USB_Write
};

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
