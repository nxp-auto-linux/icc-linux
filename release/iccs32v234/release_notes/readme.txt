*******************************************************************************
Freescale ARM Inter Core Communication S32V234 EAR 0.8.0
9 September 2016
Readme.txt
Freescale(TM) and the Freescale logo are trademarks of Freescale Semiconductor,
Inc. All other product or service names are the property of their respective
owners.
(c) Copyright 2014,2016 Freescale Semiconductor Inc.
(c) Copyright 2016 NXP
*******************************************************************************

This package contains Freescale ARM Inter Core Communication(ICC) S32V234 EAR 0.8.0.

ICC is designed to work on ARM platforms with two cores.
Prior to being used it needs to be integrated with the Operating Systems running
on each core.

The release was validated using the compiler:
- Linaro ARM NONE EABI Gcc Compiler 4.9-2014.7 for AUTOSAR code
- Linaro GCC ARM-2014.11 (based on GCC 4.9.3) for LINUX code

This build includes direct support of the following derivatives:
the silicon S32V234 cut 1.0, maskset #0N71N.

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

This release of the Inter Core Communication is provided with the Yocto Linux
BSP starting with version 9.0.

Please see the User Manual provided with the Linux BSP for details on how to 
build and run the sample application.


2. Support Information

To order NXP products or literature, consult your local sales
representative.

For technical support please use the following Web address:
    http://www.nxp.com/support

To see the list of current support phone numbers under the section labeled
"Support" click on the link labeled "Contact Us".


3. Development hardware and software recommendations

3.1. Software requirements

The makefiles for Linux have no restrictions for higher versions of GNU make.

3.2. Target hardware requirements

ICC is designed to work on any ARM platform with two cores. Validation has been
performed on the following board:
- S32V234EVB (SCH-28899 Rev D)


4. Tresos plugin

The plugin folder contains EB Tresos Studio 14.2.1  b140128-1223 configuration
plugins for ICC and configuration project samples for ICC module.
The plugin and samples have their own "readme.txt" files describing
some aspects of their installation.

For any questions about Tresos usage, details about Tresos installation and
configuration please refer to EB Tresos Studio documentation.

5. What's new?

    First ICC Release on S32V234.

6. Known limitations

 - Cache memory coherence is not enabled in this release,
   the testing was done with cache disabled on M4 and non cacheable region for
   the shared memory with A53.
  - The compiler options regarding ICC data types need to be identical for both 
   AUTOSAR and LINUX ICC applications in order to have the same size for ICC types, 
   especially for enum type (-fshort-enums).
  - The Heartbeat functionality is not enabled for this release. 

7. Additional Information

7.1 Freescale ARM Inter Core Communication Samples

The Sample application delivered with the Freescale ARM Inter Core Communication
should help to learn how to use ICC.

Please see the User Manual provided with the Linux BSP for details on how to 
build and run the sample application.


8. Release History

Freescale ARM Inter Core Communication S32V234 EAR 0.8.0 - this is the first release of
Freescale Inter Core Communication for S32V234 family.