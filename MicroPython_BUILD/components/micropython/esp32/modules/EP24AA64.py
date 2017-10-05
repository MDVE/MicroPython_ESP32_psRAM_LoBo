import time
from machine import I2C, Pin

USE_I2C_BLOCK_NUMBER = 1
WRITE_PROTECT_ON = 1
WRITE_PROTECT_OFF = 0

class EP24AA64():

    t24C512= 512 * 1024 / 8 #512 Kbits
    t24C256= 256 * 1024 / 8 #256 Kbits
    t24C128= 128 * 1024 / 8 #128 Kbits
    t24C64= 64 * 1024 / 8 #64 Kbits
    capacity=0
    address=0x50
    speed=400000
    sdapin=4
    sclpin=16
    wppin=0
    # i2c automatically created

    def __init__(self, chipType=t24C64, addr=address,  speed=speed,  sda=sdapin,  scl=sclpin,  writeprotect=wppin):
        self.capacity=int(chipType)
        self.address=int(addr)
        self.speed=int(speed)
        self.sdapin=int(sda)
        self.sclpin=int(scl)
        self.wppin=int(writeprotect)
        if (self.wppin > 0):
            self.wp = Pin(self.wppin, Pin.OUT)
            self.wp.value(WRITE_PROTECT_ON)
        try:
            self.i2c=I2C(USE_I2C_BLOCK_NUMBER)
            self.i2c.init(I2C.MASTER, scl=Pin(self.sclpin), sda=Pin(self.sdapin), speed=self.speed)
        except:
            pass

    def reset(self):
        if (self.wppin > 0):
            self.wp = Pin(self.wppin, Pin.OUT)
            self.wp.value(WRITE_PROTECT_ON)
        try:
            self.i2c.deinit()
            self.i2c=I2C(USE_I2C_BLOCK_NUMBER)
            self.i2c.init(I2C.MASTER, scl=Pin(self.sclpin), sda=Pin(self.sdapin), speed=self.speed)
        except:
            pass

    def isOnline(self):
        z = -1
        try:
            z = self.i2c.scan().count(self.address)
        except:
            z = -1
        if (z > 0):
            return 1
        return 0
        
    def writeByteToEEPROM(self, eeaddress, value):
        data = bytearray(1)
        data[0]=value
        self.wp.value(WRITE_PROTECT_OFF)
        self.i2c.writeto_mem(self.address, eeaddress | 0x10000000, data)
        self.wp.value(WRITE_PROTECT_ON)
        #if (self.i2c.scan().count(0x50) <= 0):
        if (self.i2c.ping(0x50) <= 0):
            pass

    def writeDataToEEPROM(self, eeaddress, data):
        # cannot write past 32-byte page boundary
        self.wp.value(WRITE_PROTECT_OFF)
        i = 0
        laddr=int(eeaddress)
        dl = len(data)
        
        # calculate number of bytes to write to first page
        nz = (0xFFC0 & laddr) - laddr
        #print("huddle: {0}".format(nz))
        while (nz <= 0):
            #print("hup: {0}".format(nz))
            nz = nz + 32
        if (nz > dl): # all target bytes to write are on the first page
            #print("yah: {0}".format(nz))
            nz = dl
        #print("Num first bytes: {0}".format(nz))
        while (i < dl):
            z = bytearray(nz)
            for j in range(nz): # (0,nz,1)
                z[j]=data[i]
                i = i + 1
                #if (i >= len(data)):
                #    break
            #print("Writing to {0}: {1}".format(hex(laddr), z))
            self.i2c.writeto_mem(self.address, laddr | 0x10000000, z)
            if (self.i2c.scan().count(self.address) <= 0):
            #if (self.i2c.ping(self.address) <= 0): # wait for ack
                time.sleep_us(10)
            laddr = laddr + nz
            nz = 32 if (dl-i) > 32 else (dl-i) # number of bytes to write
        self.wp.value(WRITE_PROTECT_ON)

    def readByteFromEEPROM(self, eeaddress):
        data = self.i2c.readfrom_mem(self.address,  eeaddress | 0x10000000,  1)
        return data[0]

    def readDataFromEEPROM(self, eeaddress,  length):
        data = self.i2c.readfrom_mem(self.address, eeaddress | 0x10000000, length)
        return data

    def readIntoArrayFromEEPROM(self, eeaddress,  length):
        buf = bytearray(length)
        data = self.i2c.readfrom_mem_into(self.address,  eeaddress | 0x10000000,  buf)
        return buf

    def initialize_eeprom(self):
        i=0
        snout = bytearray(32)
        
        for q in range(32):
            snout[q]=0xFF
            
        while i<self.capacity:
            self.writeDataToEEPROM(i, snout)
            check=self.readByteFromEEPROM(i)
            if (snout[0] == check):
                print(("00000000"+hex(i)[2:])[-8:],"ok")
            else:
                print(("00000000"+hex(i)[2:])[-8:],"failed")
                break
            i+=32
        print("Capacity:",i,"vs",self.capacity,"declared.")
