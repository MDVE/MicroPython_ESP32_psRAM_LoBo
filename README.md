# MicroPython for ESP32

# with support for 4MB of psRAM

---

**This repository can be used to build MicroPython for modules with psRAM as well as for regular ESP32 modules without psRAM.**

**As of Sep 18, 2017 full support for psRAM is included into esp-idf and xtensa toolchain**

---

MicroPython works great on ESP32, but the most serious issue is still (as on most other MicroPython boards) limited amount of free memory.

ESP32 can use external **SPI RAM (psRAM)** to expand available RAM up to 16MB. 
Currently, there are several modules & development boards which incorporates **4MB** of psRAM:

**ESP-WROVER-KIT boards** from Espressif or [AnalogLamb](https://www.analoglamb.com/product/esp-wrover-kit-esp32-wrover-module/).

**ESP-WROVER** from Espressif or [AnalogLamb](https://www.analoglamb.com/product/esp32-wrover/).

**ALB32-WROVER** from [AnalogLamb](https://www.analoglamb.com/product/alb32-wrover-esp32-module-with-64mb-flash-and-32mb-psram/).

**S01** and **L01** OEM modules from [Pycom](https://www.pycom.io/webshop).

---

Some basic [documentation](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo/blob/master/MicroPython_BUILD/components/micropython/docs/README.md) specific to this **MicroPython** port is available. It will soon be updated to include the documentation for all added/changed modules.

Some examples can be found in [modules_examples](https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo/tree/master/MicroPython_BUILD/components/micropython/esp32/modules_examples) directory.

---

This repository contains all the tools and sources necessary to **build working MicroPython firmware** which can fully use the advantages of **4MB** (or more) of **psRAM**

It is **huge difference** between MicroPython running with **less than 100KB** of free memory and running with **4MB** of free memory.

---

## **The MicroPython firmware is built as esp-idf component**

This means the regular esp-idf **menuconfig** system can be used for configuration. Besides the ESP32 configuration itself, many MicroPython options can also be configured via **menuconfig**.

This way many features not available in standard ESP32 MicroPython are enabled, like unicore/dualcore, all Flash speed/mode options etc. No manual *sdkconfig.h* editing and tweaking is necessary.

---

### Features

* MicroPython build is based on latest build (1.9.2) from [main Micropython repository](https://github.com/micropython/micropython)
* ESP32 build is based on [MicroPython's ESP32 build](https://github.com/micropython/micropython-esp32/tree/esp32/esp32) with added changes needed to build on ESP32 with psRAM
* Default configuration has **2MB** of MicroPython heap, **20KB** of MicroPython stack, **~200KB** of free DRAM heap for C modules and functions
* MicroPython can be built in **unicore** (FreeRTOS & MicroPython task running only on the first ESP32 core, or **dualcore** configuration (MicroPython task running on ESP32 **App** core)
* ESP32 Flash can be configured in any mode, **QIO**, **QOUT**, **DIO**, **DOUT**
* **BUILD.sh** script is provided to make **building** MicroPython firmware as **easy** as possible
* Internal filesystem is built with esp-idf **wear leveling** driver, so there is less danger of damaging the flash with frequent writes. File system parameters (start address, size, ...) can be set via **menuconfig**.
* **SPIFFS** filesystem is supported and can be used instead of FatFS in SPI Flash. Configurable via **menuconfig**
* **sdcard** support is included which uses esp-idf **sdmmc** driver and can work in **SD mode** (*1-bit* and *4-bit*) or in **SPI mode** (sd card can be connected to any pins). For imformation on how to connect sdcard see the documentation.
* Files **timestamp** is correctly set to system time both on internal fat filesysten and on sdcard
* **Native ESP32 VFS** support for spi Flash & sdcard filesystems.
* **RTC Class** is added to machine module, including methods for synchronization of system time to **ntp** server, **deepsleep**, **wakeup** from deepsleep **on external pin** level, ...
* **Time zone** can be configured via **menuconfig** and is used when syncronizing time from NTP server
* Built-in **ymodem module** for fast transfer of text/binary files to/from host
* Some additional frozen modules are added, like **pye** editor, **urequests**, **functools**, **logging**, ...
* **Btree** module included, can be Enabled/Disabled via **menuconfig**
* **_threads** module greatly improved, inter-thread **notifications** and **messaging** included
* **Neopixel** module using ESP32 **RMT** peripheral with many new features
* **i2c** module uses ESP32 hardware i2c driver
* **spi** module uses ESP32 hardware spi driver
* **curl** module added, many client protocols including FTP and eMAIL
* **ssh** module added with sftp support
* **display** module added with full support for spi TFT displays
* **DHT** module implemented using ESP32 RMT peripheral
* **mqtt** module added, implemented in C, runs in separate task
* **telnet** module added, connect to **REPL via WiFi** using telnet protocol
* **ftp** server module added, runs as separate ESP32 task
* **Eclipse** project files included. To include it into Eclipse goto File->Import->Existing Projects into Workspace->Select root directory->[select *MicroPython_BUILD* directory]->Finish. **Rebuild index**.

---


### How to Build

---

Clone this repository, as it uses some submodules, use --recursive option

```
git clone --recursive https://github.com/loboris/MicroPython_ESP32_psRAM_LoBo.git
```

*Xtensa toolchains and esp-idf are provided as tar archives. They will be automatically unpacked on* **first run** *of* **BUILD.sh** *script*

**Goto MicroPython_BUILD directory**

---

To change some ESP32 & Micropython options or to create initial **sdkconfig** run:
```
./BUILD.sh menuconfig
```

To build the MicroPython firmware, run:
```
./BUILD.sh
```
You can use -jn option to make the build process faster. If using too high **n** the build may fail, if that happens, run build again or run without the -j option.

If no errors are detected, you can now flash the MicroPython firmware to your board. Run:
```
./BUILD.sh flash
```
The board stays in bootloader mode. Run your terminal emulator and reset the board.

You can also run *./BUILD.sh monitor* to use esp-idf's terminal program, it will reset the board automatically.

*After changing* **sdkconfig.h** (via menuconfig) *always run* **./BUILD.sh clean** *before new build*

---


### BUILD.sh

Included *BUILD.sh* script makes **building** MicroPython firmware **easy**.

Usage:

* **./BUILD.sh**               - run the build, create MicroPython firmware
* **./BUILD.sh -jn**           - run the build on multicore system, much faster build. Replace **n** with the number of cores on your system
* **./BUILD.sh menuconfig**    - run menuconfig to configure ESP32/MicroPython
* **./BUILD.sh clean**         - clean the build
* **./BUILD.sh flash**         - flash MicroPython firmware to ESP32
* **./BUILD.sh erase**         - erase the whole ESP32 Flash
* **./BUILD.sh monitor**       - run esp-idf terminal program
* **./BUILD.sh makefs**        - create SPIFFS file system image which can be flashed to ESP32
* **./BUILD.sh flashfs**       - flash SPIFFS file system image to ESP32, if not created, create it first
* **./BUILD.sh copyfs**        - flash the default SPIFFS file system image to ESP32

As default the build process runs silently, without showing compiler output. You can change that by adding **verbose** as the last parameter to *BUILD.sh*.

**To build with psRAM support:**

In menuconfig select **→ Component config → ESP32-specific → Support for external, SPI-connected RAM**

In menuconfig select **→ Component config → ESP32-specific → SPI RAM config → Make RAM allocatable using heap_caps_malloc**

After the successful build the firmware files will be placed into **firmware** directory. **flash.sh** script will also be created.

---


#### Using **SPIFFS** filesystem

**SPIFFS** filesystem can be used on internal spi Flash instead of **FatFS**.

If you want to use it configure it via **menuconfig**  *→ MicroPython → File systems → Use SPIFFS*

**Prepared** image file can be flashed to ESP32, if not flashed, filesystem will be formated after first boot.


SFPIFFS **image** can be **prepared** on host and flashed to ESP32:

Copy the files to be included on spiffs into **components/spiffs_image/image/** directory. Subdirectories can also be added.

Execute:

`./BUILD.sh makefs`

to create **spiffs image** in **build** directory **without flashing** to ESP32

Execute:

`./BUILD.sh flashfs`

to create **spiffs image** in **build** directory and **flash** it to ESP32

Execute:

`./BUILD.sh copyfs`

to **flash default spiffs image** *components/spiffs_image/spiffs_image.img* to ESP32

---


### Some examples

Using new machine methods and RTC:
```
import machine

rtc = machine.RTC()

rtc.init((2017, 6, 12, 14, 35, 20))

rtc.now()

rtc.ntp_sync(server="<ntp_server>" [,update_period=])
  <ntp_server> can be empty string, then the default server is used ("pool.ntp.org")

rtc.synced()
  returns True if time synchronized to NTP server

rtc.wake_on_ext0(Pin, level)
rtc.wake_on_ext1(Pin, level)
  wake up from deepsleep on pin level

machine.deepsleep(time_ms)
machine.wake_reason()
  returns tuple with reset & wakeup reasons
machine.wake_description()
  returns tuple with strings describing reset & wakeup reasons


```

Using sdcard module:
```
import uos

uos.mountsd()
uos.listdir('/sd')
```

Working directory can be changed to root of the sd card automatically on mount:
```
>>> import uos
>>> uos.mountsd(True)
---------------------
 Mode:  SD (4bit)
 Name: NCard
 Type: SDHC/SDXC
Speed: default speed (25 MHz)
 Size: 15079 MB
  CSD: ver=1, sector_size=512, capacity=30881792 read_bl_len=9
  SCR: sd_spec=2, bus_width=5

>>> uos.listdir()
['overlays', 'bcm2708-rpi-0-w.dtb', ......
>>>

```

---

Tested on **ESP-WROVER-KIT v3**
![Tested on](https://raw.githubusercontent.com/loboris/MicroPython_ESP32_psRAM_LoBo/master/Documents/ESP-WROVER-KIT_v3_small.jpg)

---

### Example terminal session


```

rst:0x1 (POWERON_RESET),boot:0x3e (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0010,len:4
load:0x3fff0014,len:5656
load:0x40078000,len:0
ho 12 tail 0 room 4
load:0x40078000,len:13220
entry 0x40078fe4
W (36) rtc_clk: Possibly invalid CONFIG_ESP32_XTAL_FREQ setting (40MHz). Detected 40 MHz.
I (59) boot: ESP-IDF ESP32_LoBo_v1.9.1-13-gfecf988-dirty 2nd stage bootloader
I (60) boot: compile time 21:07:29
I (108) boot: Enabling RNG early entropy source...
I (108) boot: SPI Speed      : 40MHz
I (108) boot: SPI Mode       : DIO
I (115) boot: SPI Flash Size : 4MB
I (128) boot: Partition Table:
I (139) boot: ## Label            Usage          Type ST Offset   Length
I (162) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (185) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (209) boot:  2 MicroPython      factory app      00 00 00010000 00270000
I (232) boot:  3 internalfs       Unknown data     01 81 00280000 00140000
I (255) boot: End of partition table
I (268) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f400020 size=0x48a74 (297588) map
I (613) esp_image: segment 1: paddr=0x00058a9c vaddr=0x3ffb0000 size=0x07574 ( 30068) load
I (650) esp_image: segment 2: paddr=0x00060018 vaddr=0x400d0018 size=0xc83f4 (820212) map
0x400d0018: _stext at ??:?

I (1525) esp_image: segment 3: paddr=0x00128414 vaddr=0x3ffb7574 size=0x052d0 ( 21200) load
I (1551) esp_image: segment 4: paddr=0x0012d6ec vaddr=0x40080000 size=0x00400 (  1024) load
0x40080000: _iram_start at /home/LoBo2_Razno/ESP32/MicroPython/MicroPython_ESP32_psRAM_LoBo/Tools/esp-idf/components/freertos/./xtensa_vectors.S:1675

I (1553) esp_image: segment 5: paddr=0x0012daf4 vaddr=0x40080400 size=0x1a744 (108356) load
I (1711) esp_image: segment 6: paddr=0x00148240 vaddr=0x400c0000 size=0x0006c (   108) load
I (1712) esp_image: segment 7: paddr=0x001482b4 vaddr=0x50000000 size=0x00400 (  1024) load
I (1794) boot: Loaded app from partition at offset 0x10000
I (1794) boot: Disabling RNG early entropy source...
I (1800) spiram: SPI RAM mode: flash 40m sram 40m
I (1812) spiram: PSRAM initialized, cache is in low/high (2-core) mode.
I (1834) cpu_start: Pro cpu up.
I (1846) cpu_start: Starting app cpu, entry point is 0x400814e4
0x400814e4: call_start_cpu1 at /home/LoBo2_Razno/ESP32/MicroPython/MicroPython_ESP32_psRAM_LoBo/Tools/esp-idf/components/esp32/./cpu_start.c:219

I (0) cpu_start: App cpu up.
I (4612) spiram: SPI SRAM memory test OK
I (4614) heap_init: Initializing. RAM available for dynamic allocation:
I (4615) heap_init: At 3FFAE2A0 len 00001D60 (7 KiB): DRAM
I (4633) heap_init: At 3FFC30C0 len 0001CF40 (115 KiB): DRAM
I (4653) heap_init: At 3FFE0440 len 00003BC0 (14 KiB): D/IRAM
I (4672) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM
I (4692) heap_init: At 4009AB44 len 000054BC (21 KiB): IRAM
I (4712) cpu_start: Pro cpu start user code
I (4777) cpu_start: Starting scheduler on PRO CPU.
I (2920) cpu_start: Starting scheduler on APP CPU.

FreeRTOS running on BOTH CORES, MicroPython task started on App Core.

uPY stack size = 19456 bytes
uPY  heap size = 2097152 bytes (in SPIRAM using heap_caps_malloc)

Reset reason: Power on reset Wakeup: Power on wake
I (3130) phy: phy_version: 359.0, e79c19d, Aug 31 2017, 17:06:07, 0, 0

Starting WiFi ...
WiFi started
Synchronize time from NTP server ...
Time set

MicroPython ESP32_LoBo_v2.0.2 - 2017-09-19 on ESP32 board with ESP32
Type "help()" for more information.
>>> 
>>> import micropython, machine
>>> micropython.mem_info()
stack: 736 out of 19456
GC: total: 2049088, used: 6848, free: 2042240
 No. of 1-blocks: 37, 2-blocks: 9, max blk sz: 329, max free sz: 127565
>>> machine.heap_info()
Free heap outside of MicroPython heap:
 total=2232108, SPISRAM=2097108, DRAM=135000
>>> 
>>> a = ['esp32'] * 200000
>>> 
>>> a[123456]
'esp32'
>>> 
>>> micropython.mem_info()
stack: 736 out of 19456
GC: total: 2049088, used: 807104, free: 1241984
 No. of 1-blocks: 44, 2-blocks: 13, max blk sz: 50000, max free sz: 77565
>>> 

```
