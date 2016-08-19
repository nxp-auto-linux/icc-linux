*******************************************************************************
Freescale ARM Inter Core Communication SAC58R EAR 0.8.0
30 September 2014
Readme.txt
Freescale(TM) and the Freescale logo are trademarks of Freescale Semiconductor,
Inc. All other product or service names are the property of their respective
owners.
(C) Freescale Semiconductor, Inc. 2014
*******************************************************************************

This package contains Freescale ARM Inter Core Communication(ICC) SAC58R EAR 0.8.0.

ICC is designed to work on ARM platforms with two cores.
Prior to being used it needs to be integrated with the Operating Systems running
on each core.

The release was validated using the compiler:
- Linaro ARM NONE EABI Gcc Compiler 4.9-2014.7 
for both AUTOSAR and LINUX code.

This build includes direct support of the following derivatives:
SAC58R_529BGA, the silicon PAC58R5XXK1YMX6 cut 1.0, maskset# 0N98K.

and was validated on the boards specified in section '3.2 Target hardware
requirements'.


This document contains additional information about the product.
The notes presented are the latest available.

The following topics are covered:

1. Installation
2. Support Information
3. Development hardware and software recommendations
4. Tresos plugin
5. What's new?
6. Known problems
7. Additional Information
8. Release History


1. Installation

1.1 Installation and license file

Please check the Freescale Support website for new versions and updates (see
section 2 for customer support information).

Installation of Standard Software Core and System Generator are protected with
FLEXlm.

To get the installation decryption key please contact Freescale support.

1.2. Installation instructions

To setup Freescale ARM Inter Core Communication SAC58R EAR 0.8.0, on your hard
disk follow the steps below.

1.2.1. Installer execution

Run the SAC58R_ICC_EAR_0_8_0.exe program.

1.2.2. Preparing for installation

It is strongly recommended to close all other programs and login as
Administrator before installation. It helps to avoid an access error during
system icons installation and updating Windows Registry.

NOTES: 1. The full Freescale ARM Inter Core Communication file set takes about
          10 MB of disk space.
       2. About 30 MB of disk space is required for the temporary files during
          installation.
       3. It is required to install Freescale ARM Inter Core Communication into
          the directory without spaces. (Do not install ICC in a directory
          like "Program Files").

1.2.3. Select directories

Target Directory is a directory for ICC source files, user documentation
and platform specific system generator files.

1.2.4. Select components which you want to install

You can choose Custom installation and select Freescale ARM Inter Core Communication
components which have to be installed in Custom Installation dialog box. By default,
all components are selected.

1.2.5. Choose the tools (compilers and Tresos) to be used with ICC

It is recommended to enter the paths to the installation directories of the
tools which the installer requires. Otherwise, after the installation, manual
setup of the tools will be necessary for the tools to work correctly with the
installed ICC project.

1.2.6. Follow the prompts and instructions of the installation program.

1.2.7. The installation program copies to the target directory all needed
       files. The directory tree is described in the file filelist.txt.

NOTE: Installation of Adobe Acrobat Reader 5.0 or later is required to view
      the ARM Freescale Inter Core Communication User's Manual.

1.3. Freescale ARM Inter Core Communication ICC Uninstallation

To uninstall Freescale ARM Inter Core Communication use the 'Freescale SAC58R ICC 0.8.0'
item of the Add/Remove Programs module of the Windows Control Panel.


2. Support Information

To order Freescale products or literature, consult your local sales
representative.

For technical support please use the following Web address:
    http://www.freescale.com/support

To see the list of current support phone numbers under the section labeled
"Support" click on the link labeled "Contact Us".


3. Development hardware and software recommendations

3.1. Software requirements

The PC has to work under Windows XP/7 during Freescale ARM Inter Core Communication
installation.

For the M4's side the makefiles provided with ICC are written for GNU make v.3.81.
Higher versions of GNU make are not supported. Also, the CygWin package with make
utility shall be properly installed and path to CygWin binaries shall be in
the PATH variable.

The makefiles for Linux have no restrictions for higher versions of GNU make.

3.2. Target hardware requirements

ICC is designed to work on any ARM platform with two cores. Validation has been
performed on the following board:
- SOC-RAYL-N40LP(SCH-28406 REV A)


4. Tresos plugin

The plugin folder contains EB Tresos Studio 14.2.1  b140128-1223 configuration
plugins for ICC and configuration project samples for ICC module.
The plugin and samples have their own "readme.txt" files describing
some aspects of their installation.

For any questions about Tresos usage, details about Tresos installation and
configuration please refer to EB Tresos Studio documentation.

5. What's new?

    First ICC Release on SAC58R.

6. Known limitations

 - Cache memory coherence is not enabled in this release,
   the testing was done with cache disabled on M4 and non cacheable region for
   the shared memory with A7.
 - The ICC testing was realized on Linux without having activated SMP feature.
 - The compiler options regarding ICC data types need to be identical for both 
 AUTOSAR and LINUX ICC applications in order to have the same size for ICC types, 
 especially for enum type (-fshort-enums).

7. Additional Information

7.1 Freescale ARM Inter Core Communication Samples

The Sample application delivered with the Freescale ARM Inter Core Communication
should help to learn how to use ICC.


8. Release History

Freescale ARM Inter Core Communication SAC58R EAR 0.8.0 - this is the first release of
Freescale Inter Core Communication for SAC58R family.
