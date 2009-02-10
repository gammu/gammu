/*
 *  Nokia DKU2 USB driver
 *
 *  Copyright (C) 2004
 *  Author: C Kemp
 *
 *  This program is largely derived from work by the linux-usb group
 *  and associated source files.  Please see the usb/serial files for
 *  individual credits and copyrights.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  20.09.2005 - Matthias Blaesing <matthias.blaesing@rwth-aachen.de>
 *  Added usb_deregister to exit code - to allow remove and reinsert of module
 */


#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <asm/uaccess.h>
#include <linux/usb.h>

#ifdef CONFIG_USB_SERIAL_DEBUG
	static int debug = 1;
#else
	static int debug;
#endif

#include "usb-serial.h"
#include "nokia_dku2.h"

/*
 * Version Information
 */
#define DRIVER_VERSION	"v0.2"
#define DRIVER_AUTHOR	"C Kemp"
#define DRIVER_DESC	"Nokia DKU2 Driver"

/* Function prototypes */
static int nokia_startup(struct usb_serial *serial);
static void nokia_shutdown(struct usb_serial *serial);
static void generic_read_bulk_callback(struct urb *urb);
static void generic_write_bulk_callback(struct urb *urb);

static struct usb_device_id id_table [] = {
	{ USB_DEVICE(NOKIA_VENDOR_ID, NOKIA7600_PRODUCT_ID) },
	{ USB_DEVICE(NOKIA_VENDOR_ID, NOKIA6230_PRODUCT_ID) },
	{ USB_DEVICE(NOKIA_VENDOR_ID, NOKIA6170_PRODUCT_ID) },
	{ USB_DEVICE(NOKIA_VENDOR_ID, NOKIA6670_PRODUCT_ID) },
	{ USB_DEVICE(NOKIA_VENDOR_ID, NOKIA6680_PRODUCT_ID) },
	{ USB_DEVICE(NOKIA_VENDOR_ID, NOKIA6230i_PRODUCT_ID) },
	{ }			/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, id_table);

static struct usb_serial_device_type nokia_device = {
	.owner =		THIS_MODULE,
	.name =			"Nokia 7600/6230(i)/6170/66x0 DKU2 driver",
	.id_table =		id_table,
	.num_interrupt_in =	1,
	.num_bulk_in =		1,
	.num_bulk_out =		1,
	.num_ports =		1,
	.startup =		nokia_startup,
	.shutdown =		nokia_shutdown,
};

/* do some startup allocations not currently performed by usb_serial_probe() */
static int nokia_startup(struct usb_serial *serial)
{
        struct usb_serial_port *port;
        struct usb_endpoint_descriptor *endpoint;
	int buffer_size = 0;

	dbg("%s", __FUNCTION__);

	if (serial->interface->altsetting[0].endpoint[0].bEndpointAddress == NOKIA_AT_PORT) {
		/* the AT port */
		printk("Nokia AT Port:\n");
	} else if (serial->interface->num_altsetting == 2 &&
		   serial->interface->altsetting[1].endpoint[0].bEndpointAddress == NOKIA_FBUS_PORT) {
		printk("Nokia FBUS Port:\n");
		usb_set_interface(serial->dev, 10, 1);
		/* The usbserial driver won't have sorted out the endpoints for us so... */
		endpoint = &serial->interface->altsetting[1].endpoint[0];
		port = &serial->port[0];
		port->read_urb = usb_alloc_urb(0);
		if (!port->read_urb)
			return -1;

		buffer_size = endpoint->wMaxPacketSize;
		port->bulk_in_endpointAddress = endpoint->bEndpointAddress;
		port->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
		if (!port->bulk_in_buffer)
			return -1;

		usb_fill_bulk_urb(port->read_urb, serial->dev,
				  usb_rcvbulkpipe(serial->dev,
						  endpoint->bEndpointAddress),
				  port->bulk_in_buffer, buffer_size,
				  generic_read_bulk_callback, port);

		endpoint = &serial->interface->altsetting[1].endpoint[1];
		port->write_urb = usb_alloc_urb(0);
		if (!port->write_urb)
			return -1;

		buffer_size = endpoint->wMaxPacketSize;
		port->bulk_out_size = buffer_size;
		port->bulk_out_endpointAddress = endpoint->bEndpointAddress;
		port->bulk_out_buffer = kmalloc(buffer_size, GFP_KERNEL);
		if (!port->bulk_out_buffer)
			return -1;

		usb_fill_bulk_urb(port->write_urb, serial->dev,
				  usb_sndbulkpipe(serial->dev,
						  endpoint->bEndpointAddress),
				  port->bulk_out_buffer, buffer_size,
				  generic_write_bulk_callback, port);
		serial->num_bulk_out = 1;
		serial->num_bulk_in = 1;
	} else
		return -1;

	init_waitqueue_head(&serial->port->write_wait);

	return 0;
}

static void generic_read_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = (struct usb_serial_port *)urb->context;
	struct usb_serial *serial = get_usb_serial (port, __FUNCTION__);
	struct tty_struct *tty;
	unsigned char *data = urb->transfer_buffer;
	int i;
	int result;

	dbg("%s - port %d", __FUNCTION__, port->number);

	if (!serial) {
		dbg("%s - bad serial pointer, exiting", __FUNCTION__);
		return;
	}

	if (urb->status) {
		dbg("%s - nonzero read bulk status received: %d", __FUNCTION__, urb->status);
		return;
	}

	usb_serial_debug_data(__FILE__, __FUNCTION__, urb->actual_length, data);

	tty = port->tty;
	if (tty && urb->actual_length) {
		for (i = 0; i < urb->actual_length ; ++i) {
			/* if we insert more than TTY_FLIPBUF_SIZE characters, we drop them. */
			if (tty->flip.count >= TTY_FLIPBUF_SIZE) {
				tty_flip_buffer_push(tty);
			}
			/* this doesn't actually push the data through unless tty->low_latency is set */
			tty_insert_flip_char(tty, data[i], 0);
		}
	  	tty_flip_buffer_push(tty);
	}

	/* Continue trying to always read  */
	usb_fill_bulk_urb(port->read_urb, serial->dev,
			  usb_rcvbulkpipe(serial->dev,
					  port->bulk_in_endpointAddress),
			  port->read_urb->transfer_buffer,
			  port->read_urb->transfer_buffer_length,
			  ((serial->type->read_bulk_callback) ?
			    serial->type->read_bulk_callback :
			    generic_read_bulk_callback), port);
	result = usb_submit_urb(port->read_urb);
	if (result)
		err("%s - failed resubmitting read urb, error %d", __FUNCTION__, result);
}

static void generic_write_bulk_callback(struct urb *urb)
{
	struct usb_serial_port *port = (struct usb_serial_port *)urb->context;
	struct usb_serial *serial = get_usb_serial(port, __FUNCTION__);

	dbg("%s - port %d", __FUNCTION__, port->number);

	port->write_busy = 0;
	wmb();

	if (!serial) {
		err("%s - null serial pointer, exiting", __FUNCTION__);
		return;
	}

	if (urb->status) {
		dbg("%s - nonzero write bulk status received: %d", __FUNCTION__, urb->status);
	}

	queue_task(&port->tqueue, &tq_immediate);
	mark_bh(IMMEDIATE_BH);

	return;
}

static void nokia_shutdown(struct usb_serial *serial)
{
	int i;

	dbg("%s", __FUNCTION__);

	for (i = 0; i < serial->num_ports; ++i) {
		if (serial->port[i].private)
			kfree(serial->port[i].private);
	}
}

static int __init nokia_init(void)
{
	usb_serial_register (&nokia_device);

	info(DRIVER_VERSION " " DRIVER_AUTHOR);
	info(DRIVER_DESC);

	return 0;
}

static void __exit nokia_exit(void)
{
	usb_deregister(&nokia_driver);
	usb_serial_deregister(&nokia_device);
}

module_init(nokia_init);
module_exit(nokia_exit);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");

MODULE_PARM(debug, "i");
MODULE_PARM_DESC(debug, "Debug enabled or not");

