# HP ProBook compatibility

Mintosh targets the HP ProBook family across Intel and AMD x86-64 generations. Linux supplies the device drivers; this layer enables a broad driver and firmware baseline and reports machine identity.

## Compatibility baseline

- ACPI battery, thermal, fan and power-management interfaces
- HP WMI, HP laptop platform and accelerometer support
- Intel and AMD integrated graphics, plus Nouveau for supported NVIDIA devices
- Intel, Realtek, Qualcomm/Atheros, Broadcom and MediaTek wireless firmware
- Bluetooth host drivers and vendor firmware
- Intel HDA/SOF and common HDA codec support
- I2C-HID, ELAN, Synaptics, PS/2 and multitouch input
- NVMe, AHCI/SATA, USB storage, USB 3.x and Intel VMD
- USB webcams, USB-C/UCSI and Thunderbolt where hardware supports them

The image enables Debian Trixie's non-free-firmware component and installs selected firmware packages. Matching Linux drivers load firmware when corresponding hardware is present.

## Diagnostics

Run `hp-compat` or `hp-compat --json` for DMI vendor/model/board/BIOS identifiers and HP WMI module status. Use `ioplus --json`, `lspci -nnk`, `lsusb`, `rfkill list` and `journalctl -k` to diagnose individual devices.

## Limits

This is a broad driver and firmware baseline, not certification of every ProBook revision or component SKU. Fingerprint readers, WWAN modems, discrete GPUs, smart-card readers, BIOS hotkeys and suspend/resume vary by model and require physical validation. Do not apply model-specific ACPI quirks or firmware overrides without an exact DMI match and regression test. Unknown HP models retain generic Linux behavior.
