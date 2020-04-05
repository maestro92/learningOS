We will be creating our own libc (Standard C Library);. The GCC documentation explicity states the libgcc requires
the freestanding environment to supply the memcmp, mempcpy, memmove, and memset functions, as well as abort on some platforms.

This tutorial willsatisfy this requirement by creating a special kernel C library (libk); that contains the parts of the 
user-space libc that are freestanding.

https://wiki.osdev.org/Meaty_Skeleton



####################################################################
#################### Linux Root Directory ##########################
####################################################################

So since we are talking about our OS development, we gotta talk about root directory. 

so what is a root directory? The root directory on Unix-like operating systems is the '/' directory. 
It contains all the every directories and files on the system.

so if you write something like 
                
                cd /

you are navigating to the system root directory. If you think about all the other directories that you commonly see 

                /usr/lib 
                /lib 
                /home 

these are all directories under your system root diretories. 

So if you look at the tutorial, it says 

                "so /home/marty/myos/sysroot directory acts as a fake root directory for your OS. This is called a system root, or sysroot."

this totally make sense. Essentially, you want to create directory, and that is gonna be our system root directory
So in my setup, we will have: 

                cygwin64/marty92/myos/sysroot 

https://www.quora.com/What-is-a-sysroot




######################################################################
######################## /usr directory ##############################
######################################################################

so now lets look at one of the directory under our system root, the /usr directory. 

So general information about /usr directory. /usr usually contains by far the largest share of data on a system. Hence, this is
one of the most important directories in the system as it contains all the user binaries, their documentation, libraries, header files, etc...
X and its supporting libraries can be found here. User programs like telnet, ftp, etc... are also placed here. In the original Unix 
implementations, /usr was where the home directories of the users were placed (this is to say, /usr/someone was then the directory now 
known as /home/someone); In current Unixs, /usr is where user-land programs and data (as opposed to 'system land' programs and data) are. 


                The /usr directory consists of several subdirectories that contain additional UNIX commands and data files. 
                It is also the default location of user home directories.

                The /usr/bin directory contains more UNIX commands. 
                These commands are used less frequently or are considered nonessential to UNIX system operation.

                The /usr/include directory contains header files for compiling C programs.

                The /usr/lib directory contains more libraries and data files used by various UNIX commands.

                The /usr/spool directory contains various directories for storing files to be printed, mailed, or passed through networks.

                The /usr/tmp directory contains more temporary files.

                The /usr/adm directory contains data files associated with system administration and accounting.

                https://osr507doc.xinuos.com/en/OSAdminG/dirC.usr.html




Normally when you compile programs for your local operating system, the compiler locates development files such 
as headers and libraries in system directories such as:

                /usr/include
                /usr/lib


So for our operating system, we cant really use all the programs in "/usr/include", and "/usr/lib". 
You want to have own version of these directories that contains files for your operating system 

                /home/marty/myos/sysroot/usr/include 
                /home/marty/myos/sysroot/usr/lib 





Your build process will build each component of your OS (Kernel, standard library, programs); and gradually install them 
into the system root. 

in our setup, our system root will be populated by the "make install" targets. We will write a makefile that install the system 
headers into the sysroot/usr/include directory, the system libraries into the sysroot/usr/lib directory and the kernel 
itself into the sysroot/boot directory. 




We will also configure the compiler with system support, so it will look in ${SYSROOT}/usr/lib as expected. 

At first, We preovent the compiler from searching for a standard library using the --without-headers option when building 
i686-elf-gcc, so it will not look in ${SYSROOT}/usr/include.

Once you add a user-space and a libc, you will configure your custom cross-gcc with --with-sysroot and it will look in 
${SYSROOT}usr/include. As a temporary work-around until you get that far, we fix it by passing -isystem=/usr/include








Linux Filesystem Hierarchy 
https://www.tldp.org/LDP/Linux-Filesystem-Hierarchy/html/usr.html


