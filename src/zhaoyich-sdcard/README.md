# SDCard Driver and Dependancies
These files are copied from my EECS 373 project. All files are written for FreeRTOS. The files are described as follows:

## sdcard.[ch]
This is an an SDCard driver that supports regular (<4GiB) sdcards at high speed (1Mbps) in SPI mode.
Larger SDCard formats (SDHC etc) should be easy to support with necessary additions that
read the SDCard internal registers. Additionally, only SDCard read operations are implemented.
Write operations need extra code and testing.

The main difference between regular and SDHC cards is the meaning of the seek command,
command 17.  The smaller regular cards take an absolute address (byte address), while the bigger
SDHC cards take a sector number. This effectively makes SDHC cards able to address 512 times the
capacity, but also makes them not backward compatible.

## crc.[ch]
CRC routines for use in the SDCard driver.  Taken from online sources as indicated in the file.

## spi1_os.[ch]
A FreeRTOS task-blocking (but not busy waiting) SPI driver for Actel SmartFusion.  Should be adapted to be used
on the new chipset.

## os.[ch]
Some supporting code to prevent deadlocks when certain routines are called before FreeRTOS
scheduler starts

## diskio.c
FatFs low-level access routines. This file makes FatFs access the FAT filesystem stored on the SDCard.
Again, only the read routines are implemented.  Enabling writing in the FatFs ffconf.h would also add
a significant amount of code to the program.


## Remarks
I authorize the use of any code written by me in this folder to be used in the project of EECS 473 team Rocket.

Code taken from other sources (FatFs, FreeRTOS, crc) should be used in accordance with their respective licenses
and the course guidelines.

Yichen Zhao. 09/16/2014