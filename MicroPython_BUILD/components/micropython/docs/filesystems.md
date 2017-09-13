# File systems used in this MicroPython implementation

This implementation of MicroPython uses native **esp-idf** support for file systems and esp-idf VFS.

Two file systems are implemented: **internal** (Flash file system) and **external** (SDCard).

### Internal file system

The type and size of the Flash file system can be configured before the build using **menuconfig** (*→ MicroPython → File systems*).

* The file system can be formated as **spiffs** or **FatFS**. FatFS uses esp-idf **wear leveling**
* Start address in Flash, file system size, maximum number of opened files can be configured
* If using FatFS, *Code Page*, *Long file name support* and *max long file name length* can be configured in **→ Component config → FAT Filesystem support** via **menuconfig**. This settings are also valid for SDCard file system.

Internal file system is mounted automatically at boot in **/flash** directory and cannot be unmounted.
If the internal file system is not formated, it will be formated automatically on first boot and *boot.py* file will be created.

### External file system

SD card can be configured via **menuconfig** (*→ MicroPython → SD Card configuration*).

SD card can be connected in **SD** mode (*1-line* or *4-line*) or in **SPI** mode.

In SD mode fixed pins must be used, in SPI mode the pins used for MOSI, MISO, SCK and CS can be selected via **menuconfig**

**Using SDCard with sdmmc driver connection:**

| ESP32 pin     | SD name | SD pin | uSD pin | SPI pin | Notes                                                                |
|       -       |     -   |   -    |    -    |    -    |                                   -                                  |
| GPIO14 (MTMS) | CLK     |  5     |  5      | SCK     | 10k pullup in SD mode                                                |
| GPIO15 (MTDO) | CMD     |  2     |  3      | MOSI    | 10k pullup, both in SD and SPI modes                                 |
| GPIO2         | D0      |  7     |  7      | MISO    | 10k pullup in SD mode, pull low to go into download mode             |
| GPIO4         | D1      |  8     |  8      | N/C     | not used in 1-line SD mode; 10k pullup in 4-line SD mode             |
| GPIO12 (MTDI) | D2      |  9     |  1      | N/C     | not used in 1-line SD mode; 10k pullup in 4-line SD mode             |
| GPIO13 (MTCK) | D3      |  1     |  2      | CS      | not used in 1-line SD mode, but card's D3 pin must have a 10k pullup |
| N/C           | CD      |        |         |         | optional, not used                                                   |
| N/C           | WP      |        |         |         | optional, not used                                                   |
| VDD     3.3V  | VSS     |  4     |  4      |         |                                                                      |
| GND     GND   | GND     |  3&6   |  6      |         |                                                                      |


SDcard pinout / uSDcard pinout (Contacts view)
```
 _________________             1 2 3 4 5 6 7 8
|                 |            _______________
|                 |           |# # # # # # # #|
|                 |           |               |
|                 |           |               |
|                 |           /               |
|                 |          /                |
|                 |         |_                |
|                 |           |               |
|                #|          /                |
|# # # # # # # # /          |                 |
|_______________/           |                 |
 8 7 6 5 4 3 2 1 9          |_________________|
```

SD card **must be formated** before it can be used in MicroPython. At the moment there is no option to format SD card from MicroPython.

External file system can be **mounted**/**unmounted** using the following functions from **uos** module:

| Method  | Notes |
| - | - |
| uos.mountsd([chdir]) | Initialize SD card and mount file system on **/sd** directory. If optional argument *chdir* is set to **True**, directory is changed to */sd* after successful mount |
| uos.umountsd() | Unmount SD card. Directory is changed to */flash* after successful unmount |
