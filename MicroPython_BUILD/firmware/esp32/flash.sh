#!/bin/bash
python /home/LoBo2_Razno/ESP32/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port /dev/ttyUSB1 --baud 921600 --before default_reset --after no_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect 0x1000 bootloader/bootloader.bin 0x10000 MicroPython.bin 0x8000 partitions_singleapp.bin
