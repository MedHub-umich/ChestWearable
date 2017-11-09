# BloodPressure
# Setup Instructions
What you need to download:
1. GNU Arm Embedded Toolchain: https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads
  Update the Makefile.posix file with the location and version of this toolchain. This file is located in sdk/components/toolchain/gcc
  Here is an example of what a correct Makefile.posix would look like:
  GNU_INSTALL_ROOT := /home/kishbrao/Documents/gcc-arm-none-eabi-6-2017-q2-update/bin/
  GNU_VERSION := 6.3.1
  GNU_PREFIX := arm-none-eabi
 
2. Nordic CLI: https://www.nordicsemi.com/eng/nordic/Products/nRF51822/nRF5x-Command-Line-Tools-Linux64/51386
This is needed for use of nrfjprog and mergehex. When you install the two binaries you need to add nrfjprog to your path. Add it to your path in your ~/.bashrc as follows:
export PATH=$PATH:/home/kishbrao/Documents/eecs473/nordicTools/nrfjprog

3. Segger J-Link Tools: https://www.segger.com/downloads/jlink
Needed for using J-links


Setup Structure:
Include/ : Contains our own custom headers
lib/ : Contains our own .c files. This includes main and our other libraries
sdk/ : All relevant components of the sdk have been stripped and placed here 
linker/ : Contains the linker script that properly sets up the memory regions of the device
.gitignore : Please add build/ to your gitignore

Makefile usage
nrfjprog -e: Erase all memory (non-voltaile and volatile) on device
make flash_softdevice : Uploads the softdevice to the device
make flash: Uploads application to the device

Note that you only need to do make flash most times as the softdevice is already on all of our devboards. The full instructions are just here in case things get messed up.


Using Segger RTT:
1. In one terminal type JLinkexe
2. Use all default settings except type SWD (For serial wire debugging)
3. In another terminal type JLinkRTTClient
4. In your code use NRF_LOG_INFO("Hello World"); 
5. Ensure that the proper libraries are included 
6. In sdk/config/sdk_config.h make sure that NRF_LOG_BACKEND_RTT_ENABLED 1 is there

# Migrating to FreeRTOS
This is only necessary if you have a repository (such as blood pressure) that was not using FreeRTOS and now you want to use it. 
1. Take a look a the Makefile and copy all include and source files into your makefile path. There are no duplicates compared to BloodPressure, but any added libraries since the fork from BloodPressure will need to be re-added
2. Copy the CFLAGS to be the ones in the Makefile. SPECIFICALLY we are looking to remove the BSP flags, add the FreeRTOS flag, and add the heap and stack size flags
3. Do the same for step 2 for the ASM flags, for the same reasons
4. (optional) Copy the freertos linker from this linker folder to your linker folder and change the makefile at the top to use this linker. I don't think this is necessary, but did so just in case I missed something.
5. Run make and confirm it works

**Note:** I recommend marking the files you added to the Makefile that was not in the original BloodPressure. Then, copy in the Makefile and add those respective files. If you do this, you must add the linker file in and change the project name.

**TROUBLESHOOTING:** 
* If you find compile errors for "BSP"-esque things, make sure that the BSP flags are removed from ASMFLAGS and CFLAGS
* If you are getting "invalid state", make sure that the hub the device is connected to sent a prompt to allow notification!





