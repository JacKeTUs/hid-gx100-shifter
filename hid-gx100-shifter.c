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

static bool sysfs_strtoint(int *out, const char* buf, size_t count)
{
    char tmp[32];
    if (count >= sizeof(tmp) || !out || !buf)
        return false;   
    memcpy(tmp, buf, count);
    tmp[count] = '\0';
    return kstrtoint(tmp, 10, out) == 0;
}

static char current_mode = 'H';
static short int calibration_h_started = 0;
static short int calibration_seq_started = 0;


static void set_calibration(int is_seq, int is_started)
{
    if (is_seq)
        calibration_seq_started = is_started;
    else
        calibration_h_started = is_started;
    // here we should send report to the device to start/end calibration
}

static void set_mode(char mode) {
    current_mode = mode;
    // here we should send report to the device to switch the mode
}


static ssize_t mode_store(struct device *dev, struct device_attribute *attr,
                          const char *buf, size_t count)
{
    if (count < 1)
        return -EINVAL;

    if(buf[0] == 'H' || buf[0] == 'S')
        set_mode(buf[0]);
    else
        return -EINVAL;

    return count;
}

static ssize_t mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%c\n", current_mode);
}

static DEVICE_ATTR_RW(mode);

static ssize_t calibration_h_store(struct device *dev, struct device_attribute *attr,
                                   const char *buf, size_t count)
{
    int value;

    if (!sysfs_strtoint(&value, buf, count))
        return -EINVAL;

    if (value == 0 || value == 1)
        set_calibration(0, value);
    else
        return -EINVAL;

    return count;
}

static ssize_t calibration_h_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%hd\n", calibration_h_started);
}

static DEVICE_ATTR_RW(calibration_h);

static ssize_t calibration_seq_store(struct device *dev, struct device_attribute *attr,
                                     const char *buf, size_t count)
{
    int value;

    if (!sysfs_strtoint(&value, buf, count))
        return -EINVAL;

    if (value == 0 || value == 1)
        set_calibration(1, value);
    else
        return -EINVAL;

    return count;
}

static ssize_t calibration_seq_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%hd\n", calibration_seq_started);
}

static DEVICE_ATTR_RW(calibration_seq);

static struct attribute *shifter_attrs[] = {
    &dev_attr_mode.attr,
    &dev_attr_calibration_h.attr,
    &dev_attr_calibration_seq.attr,
    NULL
};

static const struct attribute_group shifter_attr_group = {
    .attrs = shifter_attrs
};

static int shifter_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
    int ret;

    ret = hid_parse(hdev);
    if (ret) {
        hid_err(hdev, "parse failed\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        hid_err(hdev, "hw start failed\n");
        return ret;
    }

    ret = sysfs_create_group(&hdev->dev.kobj, &shifter_attr_group);
    if (ret) {
        hid_err(hdev, "failed to create sysfs attributes\n");
        hid_hw_stop(hdev);
        return ret;
    }

    return 0;
}

static void shifter_remove(struct hid_device *hdev)
{
    sysfs_remove_group(&hdev->dev.kobj, &shifter_attr_group);
    hid_hw_stop(hdev);
}

static const struct hid_device_id shifter_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_GX, USB_PRODUCT_ID_GX100_HANDBRAKE) },
    { }
};

MODULE_DEVICE_TABLE(hid, shifter_devices);

static struct hid_driver shifter_driver = {
    .name = "gx100-shifter",
    .id_table = shifter_devices,
    .report_fixup = shifter_report_fixup,
    .probe = shifter_probe,
    .remove = shifter_remove
};
module_hid_driver(shifter_driver);

MODULE_AUTHOR("R Orth <giantorth@gmail.com>");
MODULE_AUTHOR("Oleg Makarenko <oleg@makarenk.ooo>");
MODULE_DESCRIPTION("HID driver for chinese GX100 shifter");
MODULE_LICENSE("GPL");
