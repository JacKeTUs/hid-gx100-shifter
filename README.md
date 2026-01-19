# Driver for GX-100 shifters

Fixes HID report to add linux support.  Currently doesn't support any mode setting. Recommend disabling seq mode to avoid multi-button press inputs for now.

## Installation

### Manual

```
make install
```

### DKMS
DKMS will install module into system, and will update it every time you update your kernel. Module will persist after reboots. It's the preferrable way to install it on the most distros.

1. Install `dkms` package from your distro package manager
2. Clone repository to `/usr/src/gx100-shifter`
3. Install the module:
`sudo dkms install /usr/src/gx100-shifter`
4. Update initramfs:
`sudo update-initramfs -u`
5. Reboot

To remove module:
`sudo dkms remove hid-gx100-shifter/<version> --all`
