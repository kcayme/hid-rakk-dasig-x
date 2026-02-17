// SPDX-License-Identifier: GPL-2.0
/*
 * HID driver for Rakk Dasig X
 *
 * The Rakk Dasig X gaming mouse has a faulty HID report descriptor that
 * declares USAGE_MAXIMUM = 3 (buttons 1-3) while actually sending 5 button
 * bits (REPORT_COUNT = 5). This causes the kernel to ignore side buttons
 * (buttons 4 and 5). This driver fixes the descriptor so all 5 buttons
 * are properly recognized.
 *
 * Based on hid-rakk-bulus by Carl Eric Doromal.
 */

#include <linux/hid.h>
#include <linux/module.h>

#define USB_VENDOR_ID_RAKK 0x248A
#define USB_DEVICE_ID_RAKK_DASIG_X 0xFB01
#define USB_DEVICE_ID_RAKK_DASIG_X_DONGLE 0xFA02
#define USB_DEVICE_ID_RAKK_DASIG_X_BLUETOOTH 0x8266

/*
 * The faulty byte is at offset 17 in the report descriptor.
 * Bytes 16-17 are: 0x29 0x03 (USAGE_MAXIMUM = 3)
 * The fix changes byte 17 to 0x05 (USAGE_MAXIMUM = 5).
 *
 * Original descriptor bytes 0-17:
 *   05 01 09 02 a1 01 85 01 09 01 a1 00 05 09 19 01 29 03
 *                                                        ^^
 *   Should be 0x05 to declare 5 buttons instead of 3.
 */
#define RAKK_DASIG_X_RDESC_ORIG_SIZE 193
#define RAKK_DASIG_X_DONGLE_RDESC_ORIG_SIZE 150
#define RAKK_DASIG_X_BLUETOOTH_RDESC_ORIG_SIZE 89
#define RAKK_DASIG_X_USAGE_MAX_OFFSET 17
#define RAKK_DASIG_X_USAGE_MAX_ORIG_VALUE 0x03
#define RAKK_DASIG_X_USAGE_MAX_FIXED_VALUE 0x05

/**
 * intercept_dasig_x_report - intercept and fix the HID report descriptor
 * @hdev: HID device instance
 * @rdesc: raw report descriptor buffer (mutable)
 * @rsize: pointer to the descriptor size in bytes
 *
 * Called by the HID subsystem before parsing the report descriptor. If the
 * descriptor matches a known faulty Rakk Dasig X layout (193 bytes via USB,
 * 150 bytes via dongle, or 89 bytes via Bluetooth, all with USAGE_MAXIMUM = 3
 * at offset 17), patch
 * it in-place to USAGE_MAXIMUM = 5 so the kernel registers all five mouse
 * buttons (left, right, middle, side-back, side-forward).
 *
 * Return: pointer to the (possibly patched) report descriptor.
 */
static const __u8 *intercept_dasig_x_report(struct hid_device *hdev,
                                            __u8 *rdesc, unsigned int *rsize) {
  if ((*rsize == RAKK_DASIG_X_RDESC_ORIG_SIZE ||
       *rsize == RAKK_DASIG_X_DONGLE_RDESC_ORIG_SIZE ||
       *rsize == RAKK_DASIG_X_BLUETOOTH_RDESC_ORIG_SIZE) &&
      rdesc[RAKK_DASIG_X_USAGE_MAX_OFFSET] ==
          RAKK_DASIG_X_USAGE_MAX_ORIG_VALUE) {

    hid_info(
        hdev,
        "Intercepting Rakk Dasig X report descriptor (buttons 3 -> 5)...\n");

    rdesc[RAKK_DASIG_X_USAGE_MAX_OFFSET] = RAKK_DASIG_X_USAGE_MAX_FIXED_VALUE;
  }

  return rdesc;
}

/**
 * rakk_devices - HID device ID table for the Rakk Dasig X
 *
 * Lists the vendor/product IDs this driver should bind to.
 * The Rakk Dasig X uses vendor 0x248A (Telink) with product 0xFB01
 * for direct USB, product 0xFA02 for the wireless USB dongle, and
 * product 0x8266 for Bluetooth.
 */
static const struct hid_device_id rakk_devices[] = {
    {HID_USB_DEVICE(USB_VENDOR_ID_RAKK, USB_DEVICE_ID_RAKK_DASIG_X)},
    {HID_USB_DEVICE(USB_VENDOR_ID_RAKK, USB_DEVICE_ID_RAKK_DASIG_X_DONGLE)},
    {HID_BLUETOOTH_DEVICE(USB_VENDOR_ID_RAKK,
                          USB_DEVICE_ID_RAKK_DASIG_X_BLUETOOTH)},
    {}};

/**
 * dasig_x_driver - HID driver struct for the Rakk Dasig X
 *
 * Registers the report_fixup callback with the HID subsystem so the
 * faulty report descriptor is patched before the kernel parses it.
 * No other HID callbacks are needed since the generic HID layer handles
 * everything else once the descriptor is corrected.
 */
static struct hid_driver dasig_x_driver = {
    .name = "rakk-dasig-x",
    .id_table = rakk_devices,
    .report_fixup = intercept_dasig_x_report,
};

module_hid_driver(dasig_x_driver);

MODULE_DEVICE_TABLE(hid, rakk_devices);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Karl Cayme");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION(
    "HID driver for Rakk Dasig X mouse - add support for side buttons");
