






so following this link:

https://wiki.osdev.org/Raspberry_Pi_Bare_Bones


                "The first thing you should do is set up a GCC Cross-Compiler for arm-none-eabi."



##########################################################################
############################ arm-none-eabi ###############################
##########################################################################

so what is arm-none-eabi
https://web.eecs.umich.edu/~prabal/teaching/resources/eecs373/toolchain-notes.pdf

GCC is a popular, open source toolchain that can generate code for a wide range of architectures including Intel’s x86, 
ARM v4/5/6/7, TI’s MSP, Atmel’s AVR, and many others


 If you search for an ARM compiler, you
might stumble across the following toolchains: arm-none-linux-gnueabi, arm-none-eabi, arm-eabi, and
arm-softfloat-linux-gnu, among others. This might leave you wondering about the method to the naming
madness. Unix cross compilers are loosely named using a convention of the form arch[-vendor][-os]-abi.
The arch refers to the target architecture, which in our case is ARM. The vendor nominally refers to the
toolchain supplier. The os refers to the target operating system, if any, and is used to decide which libraries
(e.g. newlib, glibc, crt0, etc.)




• arm-none-eabi is the toolchain we use in this class. This toolchain targets the ARM architecture,
has no vendor, does not target an operating system (i.e. targets a “bare metal” system), and complies
with the ARM EABI.



ARM EABI

EABI is the new "Embedded" ABI by ARM ltd. EABI is actually a family of EBAIs and one of the "sbuABIs" is GNU EABI, for Linux. 

https://wiki.debian.org/ArmEabiPort






-   ABI 
https://en.wikipedia.org/wiki/Application_binary_interface

an ABI defines how data structures or computational routines are accessed in machine code, 


System V ABI is the standard ABI used by the major Unix Operating system such as Linux, the BSD systems, and many others. 
The Executable and Linkable Format (ELF) is part of the System V ABI. 











-   Linux 
Linux is perfect in this regard because nowadays everything from small IoT devices to large servers tend to run Linux.

https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/Introduction.md


Each OS has 2 fundamental goals 
1.  Run user process in isolation
2.  Provide ach user process with a unified view of the machine hardware.




from this link 
https://hackaday.com/2018/01/19/roll-your-own-raspberry-pi-os/
                


                Writing an operating system is no small task, but like everything else it is easier than it used to be. 
                [JSandler] has a tutorial on how to create a simple operating system for the Raspberry Pi. 
                One thing that makes it easier is the development environment used. 
                QEMU emulates a Raspberry Pi so you can do the development on a desktop PC 
                and test in the virtual environment. When you are ready, you can set up a bootable SD card and try your work on a real device.


so lets download QEMU.




http://wiki.laptop.org/go/Using_QEMU_on_Windows


                The accelerator for QEMU named KQEMU, while not required, is highly recommended as a performance booster.


so lets also download KQEMU











#####################################################################################
####################################### QEMU ########################################
#####################################################################################

according to wikipedia 

                QEMU is Quick EMUlator is a free and open-source emulator that performs hardware virtualization. 


                QEMU is a hosted virtual machine monitor: it emulates the machines processor through dynamic binary translation 
                and provides a set of different hardware and device models for the machine, enabling it to run a variety of guest operating systems. 
                It also can be used with KVM to run virtual machines at near-native speed (by taking advantage of hardware extensions such as Intel VT-x). 
                QEMU can also do emulation for user-level processes, allowing applications compiled for one architecture to run on another.






#################################################################################
###################### Raspberry pi zero Specs ##################################
#################################################################################


Raspberry pi model comparisons

so the raspberrypi pi zero is in this link
https://socialcompare.com/en/comparison/raspberry-pi-alternatives

the raspberrypi zero w is in this link
http://socialcompare.com/en/comparison/raspberrypi-models-comparison


from looking at the model, two looks very similar to the one I have:

but it seems like I have the raspberry pi zero w becuz its got the camera connector on the right side.

'w' stands for wireless.


https://www.youtube.com/watch?v=TUz2mVtJVsM








The specs of it is here:
http://socialcompare.com/en/review/raspberry-pi-zero-wireless

        802.11 b/g/n wireless LAN
        Bluetooth(R) 4.1
        Bluetooth Low Energy (BLE)
        1GHz, single-core CPU
        512MB RAM
        Mini HDMI and USB On-The-Go ports
        Micro USB power
        HAT-compatible 40-pin header pins
        Composite video and reset headers
        CSI camera connector



if you want to use the Mini HDMI, usually you would get a HDMI to mini HDMI adapter.


so you need a micro SD card




Getting Started with the Raspberry Pi Zero Wireless
https://learn.sparkfun.com/tutorials/getting-started-with-the-raspberry-pi-zero-wireless/all
https://media.digikey.com/pdf/Data%20Sheets/Sparkfun%20PDFs/Getting_Started_with_RaspberryPiZeroWireless_Web.pdf







The Raspberry Pi Zero has a 32-bit ARMv6Z architecture with the Broadcom BCM2835 SoC found in the Model A and Model B+ Pis. Similarly, the CPU is a 1GHz single-core ARM1176JZF-S, similar to that found on the original Pis (but bumped up from 700MHz). It has 512MB shared RAM, and the 1.3 revision boards (those released since May 2016) also have the MIPI camera interface.

Equipped with a micro USB for power, and another for data only, the Pi Zero has mini HDMI-out and a microSD slot as expected. Stereo audio can be output via the GPIO. Although the GPIO pins are removed, the array — along with the Run and TV I/O — remain. This means that they can still be used, either by soldering, or manually adding GPIO pins (kits are available).

https://www.makeuseof.com/tag/raspberry-pi-board-guide/



so some of the videos I watched to better understand how the raspberry Pi work:

Raspberry Pi 4 Getting Started
https://www.youtube.com/watch?v=BpJCAafw2qE


so you need micro SD card





-       Raspberry Pi Zero Specs
