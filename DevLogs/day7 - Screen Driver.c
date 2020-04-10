Chapter 6
https://www.cs.bham.ac.uk/~exr/lectures/opsys/10_11/lectures/os-dev.pdf

So with our current VGA port, we are writing into the VGA text memory.
Using the graph from day4


                 ___________________    <--------- 0x00000000 
                |                   |
                |                   |
                |                   |
                |___________________|   <-------- 0x00007C00 (31 kb)
                |    bootsector     |
                |                   |   
                |___________________|  
                |                   |
                |                   |
                |                   |   
                |                   |
                |                   |   
                |                   |
                |___________________|   <--------- 0x000B8000 (736 kb)
                |                   |
                |   CGA Graphics    |
                |   CGA, EGA &      |
                |   VGA Text        |
                |___________________|   <--------- 0x000BC000 (752 kb)
                |                   |
                |                   |   
                |                   |
                |                   |               
                |                   |
                |                   |
                |                   |
                |                   |               
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |
                |                   |   <--------- 0x100000 (1MB)

So we want to understand how does CPU interact with hardware a little bit better. 

Taking the now-popular TFT monitor as an example, 


So the CPU and hardare talks to each other through I/O Bus.
https://www.karbosguide.com/hardware/module2c1.html

Visually it looks like this 

         _______________         _______________                         _______________
        |               |       |               |                       |               |
        |               |       |               |                       |               |
        |               |       |               |                       |               |
        |               |-------|               |-----------------------|               |
        |   CPU         |       |     RAM       |                       |    I/O        |
        |               |       |               |       Bus             |   Units       |
        |               |       |               |                       |               |
        |               |-------|               |-----------------------|               |
        |               |       |               |                       |               |
        |               |       |               |                       |               |
        |_______________|       |_______________|                       |_______________|

The bus controller is responsible for relaying, at a compatible rate, the instructions to a particular device_s controller. 

So hardware controller chips have several registers that can be read, written or both by the CPU, and it is the state of these 
registers that tell the hardware what to do. 

The question is, how do we tell our devices what to do programmtically? how do we communicate with our hardware devices?
In Intel architecture systems, the registers of hardware device controllers are mapped into an I/O address space. Then 
varients of the I/O instructions "in" and "out" are used to read and write data to the I/O addresses that are mapped to specific controller 
registers. 

What we mean by I/O address space is: recall this map from day 4
https://manybutfinite.com/post/motherboard-chipsets-memory-map/

we reserver a portion of the physical ram for devices

also referencing the Wiki page again,
https://en.wikipedia.org/wiki/Memory-mapped_I/O

it says:

        "The memory and registers of the I/O devices are mapped (associated with) address values. So when 
        and address is accessed by the CPU, it may refer to the a portion of physical RAM, or it can instead refer to memory 
        of the I/O device. 

        Each I/O device monitors the CPU's address bus and responds to any CPU access of an address assigned to that device, connecting the data bus 
        to the desired dvice's hardware register.
        "

Essentially, CPU modifies address values, which modifies register, which controls the hardware.

From the CPU_s perspective, an I/O device appears as a set of special-purpose registers, of three general types:
-   Control register
-   Status register
-   Data register

a logical circuit that contains these registers is called the device controller, and the software communicates with the controller is called a device driver

http://www.cs.uwm.edu/classes/cs315/Bacon/Lecture/HTML/ch14s03.html

                            +-------------------+           +-----------+
                            | Device controller |           |           |   
        +-------+           |                   |<--------->|  Device   |
        |       |---------->| Control register  |           |           |
        |  CPU  |<----------| Status register   |           |           |
        |       |<--------->| Data register     |           |           |
        +-------+           |                   |           |           |
                            +-------------------+           +-----------+



For example, flobby disk controller has its DOR register mapped to I/O address 0x3f2

https://en.wikipedia.org/wiki/Floppy-disk_controller
and in terms of assembly code, you can swtich on the motor of the first drive with the following instructions 


                mov dx , 0 x3f2         ; Must use DX to store port address
                in al , dx              ; Read contents of port ( i.e. DOR ) to AL
                or al , 00001000 b      ; Switch on the motor bit
                out dx , al             ; Update DOR of the device.





###########################################################
################# Direct Memory Access ####################
###########################################################

So there is another concept we need to introduce since we are on the topic of I/O devices, which is 
the Direct Memory Access Controller. Since port I/O invovles reading or writing individual bytes or words,
the transfer or large amouts of data between a disk device and memory could potentially take up a great 
deal of CPU time. So the CPU usually pass over this tedious task to the direct memory (DMA) controller.

https://en.wikipedia.org/wiki/Direct_memory_access

Quoting the wiki page:

        
        "Direct memory access is a feature of computer systems that allows certain hardware subsystems to access main system memory 
        (random-access memory), independent of the CPU. 

        Without DMA, when the CPU is using programmed input/output, it is typically fully occupied for the entire duration of the read or write operation, 
        and is thus unavailable to perform other work. With DMA, the CPU first initiates the transfer, 
        then it does other operations while the transfer is in progress, and it finally receives an interrupt from the DMA controller (DMAC) 
        when the operation is done. This feature is useful at any time that the CPU cannot keep up with the rate of data transfer, 
        or when the CPU needs to perform work while waiting for a relatively slow I/O data transfer."




###########################################################################
############################## Screen Driver ##############################
###########################################################################

So now we want to start writing our first device driver. We want to write a Screen Driver. Essentially, its an abstraction of the screen 
that will allow us to write print("Hello"); and perhaps clear_screen(); It will also allow us to easily substitute one display driver 
for another at a later date, perhaps if a certain computer could not support the colour VGA text mode that we currently assume. 

So we will write thhis screen.c, and we will put it in the drivers folder.

Another thing is that we organize all of our files into 3 folders now:

all the boot_sector related code to the boot folder, 
kernel.c in the kernel folder,
and the drivers folder that we just created 

        /boot 
        /drivers
        /kernel