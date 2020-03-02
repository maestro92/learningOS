################################################################################
############################### Getting Started ################################
################################################################################

So I first read the Getting Started page from OSDev

https://wiki.osdev.org/Getting_Started


You need a platform to develop your new system on. Following the trends of general computing, 
the two most popular are Windows and GNU/Linux. 
Developers using a GNU/Linux system have a slight advantage in availability of tools, but this can be solved on Windows using a system such as Cygwin.



1.  so the first thing I did is 

        "In order to get the tools necessary, you should install the Cygwin environment."

so I did that.


2.  Next it says 

        "it is strongly suggested to build a cross-compiler, not only because the default compilers target different executable formats, 
        but because it's generally a good idea. Check the GCC Cross-Compiler page for details and instructions."

so I went on to read about building a GCC Cross-Compiler:
https://wiki.osdev.org/GCC_Cross-Compiler

regarding a cross compiler, essentially "a cross-compiler is a compiler that runs on platform A (the host), 
but generates executables for platform B (the target)." 


I essentially first followed this video to build a GCC Cross-Compiler
https://www.youtube.com/watch?v=aESwsmnA7Ec 
this video link was mentioned under the "External Links" Section.


So I am gonna use Cygwin, and I have be make sure everything under the "Installing Dependencies" Column is all installed. 


3.  Next I have to download the source code of binutils and gcc
binutils:   https://ftp.gnu.org/gnu/binutils/
gcc:        https://ftp.gnu.org/gnu/gcc/

in the video, it seems like it just downloads the latest ones for both binutils and gcc

I just created a "src" directory under "$HOME" and put both of them under there: $HOME/src

-   home directory in cygwin
so the home direcotry after a new installation of Cygwin will be in 

        C:/cygwin/home/<user> or 
        C:/cygwin64/home/<user> 

if you look into it, you see the list of directories that you typically see on a linux environment

        bin
        dev 
        etc 
        home 
        lib 
        sbin
        tmp
        usr 
        var 

The instructions mentioned in the youtube video pretty much mirros whats mentioned in the OSDev wiki page on the GCC Cross-Compiler


4.  next, again, we Update the path, this is doing the "Using the new Compiler" Section
we do this by updating our .bashrc file. This shown in the video as well.







###################################################################
############################### i686 CPU ##########################
###################################################################

so what is i686? Its 6th generation Intel x86 microarchitecture, implemented by the Pentium Pro microprocessor.
Frequently referred to as i686. 

https://en.wikipedia.org/wiki/P6_(microarchitecture)


