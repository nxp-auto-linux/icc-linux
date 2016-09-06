************************************************************
* Linux ICC sample
* readme.txt
*
************************************************************

The samples for the following derivatives are provided:
- SAC58R


To build ICC Linux sample:

Before starting the building process ensure that the following variables are 
correctly filled
or indicates the correct path:

in common_cfg.mak file:
- ARCH          - the platform (ARCH=arm)
- CROSS_COMPILE - GCC Compiler path including the toolchain prefix
- KERNELDIR     - Linux Kernel Path

in each project makefile(kbuild):
- PROJECT_ROOT - Project path (path to desired sample)
- SSC_ROOT     - standard software core (SSC) path
- ICC_ROOT     - ICC code sources

Note: Compiler path shall be without final backslash.

To build ICC Linux sample :

 1. First, you should check if the ICC Linux code (linux directory) was 
 copied into the code directory and the Linux sample code (linux_app directory)
 into the sample directory.
 2. Get the address for ICC_Config[0-9]+ from ICC Autosar sample map and update it into 
 ICC_lib.c file from ICC_module.
 3. Go to ICC_module directory and compile the ICC library in order to result ICC_dev.ko.
 4. Change directory to ICC_Sample_module and compile the sample resulting ICC_Sample.ko.
 5. Copy them into Linux's filesystem of the target.
 
 To run ICC Linux sample:
 
 1. Before to insert the kernel modules for ICC sample, the M4's elf has to be
 loaded into the shared memory in order to provide to the Linux 
 the possibility to have access to ICC configuration.
 2. Insert the ICC library: insmod ICC_dev.ko
 3. When insmod ICC_Sample.ko command is run, the ICC Sample starts on Linux 
 and the sample on M4 has to be started.
 
To clean sample project go to directory where desired kernel module
is located and run:
   make clean

