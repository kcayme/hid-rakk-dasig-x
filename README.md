# HID Driver for Rakk Dasig X

Linux kernel module that fixes side button support for the Rakk Dasig X gaming mouse.

## The Problem

The Rakk Dasig X has a faulty HID report descriptor across all three connection modes (USB, wireless dongle, Bluetooth). The mouse hardware sends 5 button bits (left, right, middle, side-back, side-forward), but the descriptor only declares 3 buttons:

```
USAGE_MINIMUM  = 1  (Button 1)
USAGE_MAXIMUM  = 3  (Button 3)  <-- should be 5
REPORT_COUNT   = 5  (5 bits actually sent)
```

The kernel trusts the descriptor and ignores bits 4 and 5, so the side buttons produce no input events. This is the same class of bug found in the [Rakk Bulus](https://github.com/cedoromal/hid-rakk-bulus).

## How the Fix Works

The driver registers a `report_fixup` callback with the HID subsystem. When the kernel loads the mouse, it calls this function *before* parsing the descriptor. The driver patches a single byte at offset 17, changing `USAGE_MAXIMUM` from `0x03` to `0x05`, which tells the kernel to recognize all 5 buttons.

Unlike the Rakk Bulus driver (which replaces the entire descriptor), this driver patches in-place, preserving all other HID collections the mouse exposes (consumer control, system control, keyboard).

## Supported Connection Modes

| Mode | Product ID | Device Name | Descriptor Size |
|---|---|---|---|
| USB (direct) | `0xFB01` | Telink Wireless Gaming Mouse | 193 bytes |
| Wireless dongle | `0xFA02` | Telink Wireless Receiver | 150 bytes |
| Bluetooth | `0x8266` | RAKK Dasig X Mouse | 89 bytes |

All three have the same bug at the same byte offset (17) and are fixed by the same single-byte patch.

## Prerequisites

- Linux kernel >= 6.12 (for the `const` return type on `report_fixup`; for older kernels, change `const __u8 *` to `__u8 *` in the source)
- Linux kernel headers for your running kernel
- `make`, `gcc`
- `zstd` (for module compression)

| Distro | Install command |
|---|---|
| Fedora / Nobara / Bazzite | `sudo dnf install kernel-devel make gcc zstd` |
| Ubuntu / Mint | `sudo apt install linux-headers-$(uname -r) make gcc zstd` |
| Arch / CachyOS | `sudo pacman -S linux-headers make gcc zstd` |

### Secure Boot

If your system has Secure Boot enabled (common on Ubuntu), unsigned kernel modules will be rejected. You must either:

- **Sign the module** with a MOK (Machine Owner Key) — see your distro's documentation for enrolling a key with `mokutil`
- **Disable Secure Boot** in your BIOS/UEFI settings

## Building

```bash
make
```

This produces `hid-rakk-dasig-x.ko` and its compressed form `hid-rakk-dasig-x.ko.zst`.

## Running Tests

```bash
make test
```

This builds and runs the userspace unit tests in `tests/`, which verify the fixup logic against all three descriptor variants and various edge cases.

## Testing (Temporary Load)

Before installing permanently, test that the driver works:

### 1. Load the module

```bash
sudo insmod hid-rakk-dasig-x.ko
```

### 2. Reconnect the mouse

- **USB / Dongle:** Unplug and replug the cable or dongle.
- **Bluetooth:** Disconnect and reconnect from your Bluetooth settings (or use `bluetoothctl disconnect` then `bluetoothctl connect`).

The descriptor is only read when the device connects, so the mouse must be reconnected for the fix to take effect.

### 3. Check kernel logs

```bash
sudo dmesg | grep -i "rakk\|dasig\|intercept"
```

You should see:

```
Intercepting Rakk Dasig X report descriptor (buttons 3 -> 5)...
```

### 4. Verify button capabilities

```bash
cat /proc/bus/input/devices | grep -B 2 -A 6 "Telink\|RAKK Dasig"
```

Look at the `B: KEY=` line:

- **Before fix:** `KEY=70000 0 0 0 0` (3 buttons: left, right, middle)
- **After fix:** `KEY=1f0000 0 0 0 0` (5 buttons: left, right, middle, side, extra)

### 5. Test side button events

Install `evtest` if you don't have it:

```bash
sudo dnf install evtest
```

Run evtest and select the mouse entry:

```bash
sudo evtest
```

Select the mouse entry (e.g. "Telink Wireless Gaming Mouse", "Telink Wireless Receiver Mouse", or "RAKK Dasig X Mouse" depending on connection mode), then click the side buttons. You should see `BTN_SIDE` and `BTN_EXTRA` events.

### 6. Unload when done testing

```bash
sudo rmmod hid_rakk_dasig_x
```

## Installing (Permanent)

Once testing confirms the side buttons work, choose one of the two methods:

### Option A: DKMS (Recommended)

DKMS automatically rebuilds the module whenever a new kernel is installed. No manual intervention needed after setup.

Install DKMS if you don't have it:

| Distro | Install command |
|---|---|
| Fedora / Nobara / Bazzite | `sudo dnf install dkms` |
| Ubuntu / Mint | `sudo apt install dkms` |
| Arch / CachyOS | `sudo pacman -S dkms` |

Then install the module:

```bash
sudo make dkms-install
```

After installing, reconnect the mouse (or reboot) for the driver to take effect.

To uninstall:

```bash
sudo rmmod hid_rakk_dasig_x
sudo make dkms-uninstall
```

### Option B: Manual install

```bash
sudo make install
```

This copies the compressed module to `/lib/modules/$(uname -r)/kernel/drivers/hid/` and runs `depmod -a` so the module auto-loads whenever the mouse is connected.

After installing, reconnect the mouse (or reboot) for the driver to take effect.

To uninstall:

```bash
sudo rmmod hid_rakk_dasig_x
sudo make uninstall
```

**Note:** With manual install, you must rebuild and reinstall after every kernel update:

```bash
make clean
make
sudo make install
```

## Compatibility

Tested on:

- **Fedora Nobara** with kernel **6.18.9-201.nobara.fc43.x86_64**
- Rakk Dasig X connected via **USB direct** (`248A:FB01`)
- Rakk Dasig X connected via **wireless USB dongle** (`248A:FA02`)
- Rakk Dasig X connected via **Bluetooth** (`248A:8266`)

The vendor ID `0x248A` is shared by multiple Telink-based devices. This driver only activates for the three product IDs listed above.

## Technical Details

| Field | Value |
|---|---|
| Vendor ID | `0x248A` (Telink) |
| Product IDs | `0xFB01` (USB), `0xFA02` (dongle), `0x8266` (Bluetooth) |
| Descriptor sizes | 193 / 150 / 89 bytes |
| Patched byte offset | 17 |
| Patched byte value | `0x03` → `0x05` |
| Kernel API | `hid_driver.report_fixup` |

### File Structure

```
├── Makefile                          # Build, test, install, uninstall, dkms targets
├── dkms.conf                         # DKMS configuration for auto-rebuild on kernel updates
├── README.md                         # This file
├── src/
│   └── hid-rakk-dasig-x.c           # Driver source
└── tests/
    └── test_hid_rakk_dasig_x.c      # Userspace unit tests
```

## References

- [RAKK Dasig X](https://rakk.ph/product/rakk-dasig-x-ambidextrous-hotswap-trimode-paw3325-huano-80m-rgb-gaming-mouse-black/) — Official product page
- [hid-rakk-bulus](https://github.com/cedoromal/hid-rakk-bulus) — Reference HID driver for the Rakk Bulus (same class of bug)
- [Linux HID report_fixup API](https://www.kernel.org/doc/html/latest/hid/hid-transport.html) — Kernel documentation on HID report descriptor fixups
- [HID Usage Tables (USB-IF)](https://usb.org/document-library/hid-usage-tables-16) — USB HID usage table specification

## Credits

Based on [hid-rakk-bulus](https://github.com/cedoromal/hid-rakk-bulus).

## License

GPL-2.0
