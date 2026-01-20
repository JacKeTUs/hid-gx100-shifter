// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  HID driver for chinese GX100 shifter
 *
 *  Copyright (c) 2026
 */

#include <linux/device.h>
#include <linux/input.h>
#include <linux/hid.h>
#include <linux/module.h>
#include <linux/usb.h>

#define USB_VENDOR_ID_GX               0x04b0
#define USB_PRODUCT_ID_GX100_HANDBRAKE   0x5750

/*
Original descriptor
0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
0x09, 0x05,        // Usage (Game Pad)
0xA1, 0x02,        // Collection (Logical)
0x05, 0x09,        //   Usage Page (Button)
0x19, 0x01,        //   Usage Minimum (0x01)
0x29, 0x10,        //   Usage Maximum (0x10)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0x01,        //   Logical Maximum (1)
0x95, 0x10,        //   Report Count (16)
0x75, 0x01,        //   Report Size (1)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x05, 0x00,        //   Usage Page (Undefined)
0x09, 0x01,        //   Usage (0x01)
0x15, 0x00,        //   Logical Minimum (0)
0x25, 0xFF,        //   Logical Maximum (-1)
0x95, 0x0E,        //   Report Count (14)
0x75, 0x08,        //   Report Size (8)
0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
0x09, 0x20,        //   Usage (0x20)
0x15, 0x00,        //   Logical Minimum (0)
0x26, 0xFF, 0x00,  //   Logical Maximum (255)
0x95, 0x40,        //   Report Count (64)
0x75, 0x08,        //   Report Size (8)
0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
0xC0,              // End Collection

// 50 bytes

// best guess: USB HID Report Descriptor
*/

static const __u8 *shifter_report_fixup(struct hid_device *hid, __u8 *rdesc, unsigned int *rsize)
{
    if (*rsize == 50 && rdesc[2] == 0x09 && rdesc[3] == 0x05 && rdesc[4] == 0xA1 && rdesc[5] == 0x02) {
        hid_info(hid,
             "fixing up GX100 shifter report descriptor\n");

        rdesc[3] = 0x04; // Usage Joystick
        rdesc[5] = 0x01; // Collection Application
        return rdesc;
    } else {
        hid_info(hid,
             "Descriptor size is %d, 3rd-6th bytes are %02x %02x %02x %02x, "
             "skipping fixup\n", *rsize, rdesc[3], rdesc[4], rdesc[5], rdesc[6]);
    }

    return rdesc;
}

static const struct hid_device_id shifter_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_GX, USB_PRODUCT_ID_GX100_HANDBRAKE) },
    { }
};

MODULE_DEVICE_TABLE(hid, shifter_devices);

static struct hid_driver shifter_driver = {
    .name = "gx100-shifter",
    .id_table = shifter_devices,
    .report_fixup = shifter_report_fixup
};
module_hid_driver(shifter_driver);

MODULE_AUTHOR("R Orth <giantorth@gmail.com>");
MODULE_AUTHOR("Oleg Makarenko <oleg@makarenk.ooo>");
MODULE_DESCRIPTION("HID driver for chinese GX100 shifter");
MODULE_LICENSE("GPL");
