There are alot of different kinds of file systems. 
Some are widley use (Like FAT12, FAT16, FAT32, NTFS, ext (Linux), HFS (Used in older MACs); 
other filesystems are only used by specific companies for in house use (Like the GFS - Google File System). 
Some filesystems are used in networking only (NFS). You can also develop and design your own file system implimentation.



File Systems are used for data storage and organizing data. 
They help provide a straightforward way to access files and directories on removal media (floppies, flash drives, CDs, DVDs), 
local drives (hard disk drives), and network clients. File Systems can also exist as an in-memory image. 
For example, you can load a file that containes a "foot print" of a special type of file system in it.


Unix-like systems, has Device Files which are speical "files" that represent a device. For example, NULL (null device),
CLOCK$, PRN (printer), etc, Here is the list of device files:

CON
PRN
AUX
CLOCK$
NUL
COM0, COM1, ... COM9
LPT0, LPT1, ... LPT9
Because these names have special meaning in DOS and Windows, you cannot name a file or folder any of the above names.


#################################################################
######################## File System Driver #####################
#################################################################

 a file system driver containes the implimentation of a specific type of file system. 
 A good example of a file system driver is ntfs.sys which containes Microsoft_s implimentation of the NTFS File System.




 virtual file system is just like an interface AI ontop of specific file system implementations




#################################################################
############################### Operations ######################
#################################################################

-   Open
-   Close 
-   Read 
-   Write 
-   Mount 
-   Unmount



https://wiki.osdev.org/File_Systems

A filesystem provides a generalized structure over persistent storage, allowing the low-level structure of the devices (e.g., disk, tape, flash memory storage) to be abstracted away. Generally speaking, the goal of a filesystem is allow logical groups of data to be organized into files, which can be manipulated as a unit. In order to do this, the filesystem must provide some sort of index of the locations of files in the actual secondary storage. The fundamental operations of any filesystem are:

Tracking the available storage space
Tracking which block or blocks of data belong to which files
Creating new files
Reading data from existing files into memory
Updating the data in the files
Deleting existing files
(Perceptive readers will note that the last four operations - Create, Read, Update, and Delete, or CRUD - are also applicable to many other data structures, and are fundamental to databases as well as filesystems.)

Additionally, there are other features which go along with a practical filesystem:

Assigning human-readable names to files, and renaming files after creation
Allowing files to be divided among non-contiguous blocks in storage, and tracking the parts of files even when they are fragmented across the medium
Providing some form of hierarchical structure, allowing the files to be divided into directories or folders
Buffering reading and writing to reduce the number of actual operation on the physical medium
Caching frequently accessed files or parts of files to speed up access
Allowing files to be marked as 'read-only' to prevent unintentional corruption of critical data
Providing a mechanism for preventing unauthorized access to a user——s files
Additional features may be found on some filesystems as well, such as automatic encryption, or journalling of read/write activity.



#################################################################
########################## Volume ###############################
#################################################################

a volume or  logical drive is a single accessible sotrage area with a single file system. 
typically resident on a single partition of a hard disk. 
