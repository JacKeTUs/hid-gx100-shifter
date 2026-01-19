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

//basic
static const __u8 shifter_rdesc[] = {

    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)

    // ---- Buttons: first 16 bits of report ----
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,
    0x29, 0x10,        //   Button 1..16
    0x15, 0x00,
    0x25, 0x01,
    0x95, 0x10,        //   16 buttons
    0x75, 0x01,
    0x81, 0x02,        //   Input (Data,Var,Abs)

    // ---- Ignore remaining 14 bytes ----
    0x95, 0x0E,        //   14 bytes
    0x75, 0x08,
    0x81, 0x03,        //   Input (Const)

    0xC0
};

static const __u8 *shifter_report_fixup(struct hid_device *hid, __u8 *rdesc, unsigned int *rsize)
{
    if (*rsize == 50 && rdesc[0] == 0x05 && rdesc[1] == 0x01 && rdesc[2] == 0x09 && rdesc[3] == 0x05) {
        hid_info(hid,
             "fixing up GX100 shifter report descriptor\n");

        *rsize = sizeof(shifter_rdesc);
        return shifter_rdesc;
    } else {
        hid_info(hid,
             "Descriptor size is %d, rdesc[7] is %d, rdesc[81] is %d, rdesc[83] is %d"
             "skipping fixup\n", *rsize, rdesc[7], rdesc[81], rdesc[83]);
    }

    return rdesc;
}

static const struct hid_device_id shifter_devices[] = {
    { HID_USB_DEVICE(0x04b0, 0x5750) },
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
MODULE_DESCRIPTION("HID driver for chinese GX100 shifter");
MODULE_LICENSE("GPL");
