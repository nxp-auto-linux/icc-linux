************************************************************
* Linux ICC code
* readme.txt
************************************************************

To use the ICC code for Linux follow the below steps:

1. Untar the SAC58R_ICC_EAR_0.8.0.tar.gz archive.
2. Copy ICC Linux code(from <untar_directory>\code\linux) into code directory (<install_dir>\code\linux).
3. Copy ICC Linux sample code (from <untar_directory>\sample\linux_app) into sample directory (<install_dir>\sample\linux_app).

After the above steps were realised, the filelist for the Linux components (code and sample)
has to be similar to the following one (root directory is where ICC was installed):

===== Code ====

.\code                                                 ; The Inter Core Communication code directory

.\code\linux\                                          ; Inter Core Communication code files for Linux OS

.\code\linux\hw                                        ; Inter Core Communication hardware definitions files
.\code\linux\hw\ICC_Hw.h                               ; Inter Core Communication hardware specific definitions
.\code\linux\hw\ICC_Hw_Cache.c                         ; Cache coherence API implementation
.\code\linux\hw\ICC_Hw_Cache.h                         ; Cache coherence API interface
.\code\linux\hw\ICC_Hw_Lmem.h                          ; LMEM register definitions
.\code\linux\hw\ICC_Hw_Mscm.h                          ; MSCM register definitions
.\code\linux\hw\ICC_Hw_platform.h                      ; Inter Core Communication platform specific definitions

.\code\linux\inc                                       ; Inter Core Communication API definitions files
.\code\linux\inc\ICC_Api.h                             ; Inter Core Communication API
.\code\linux\inc\ICC_Err.h                             ; Inter Core Communication Error codes
.\code\linux\inc\ICC_Fifo.h                            ; Inter Core Communication FIFO API
.\code\linux\inc\ICC_MemMap.h                          ; Inter Core Communication Memory Sections
.\code\linux\inc\ICC_Private.h                         ; Inter Core Communication Private Types
.\code\linux\inc\ICC_Sig_Fifo.h                        ; Inter Core Communication SIGNALS FIFO API
.\code\linux\inc\ICC_Types.h                           ; Inter Core Communication Types

.\code\linux\os                                        ; Inter Core Communication Specific OS files
.\code\linux\os\ICC_Os.h                               ; Inter Core Communication OS API
.\code\linux\os\ICC_Os_Types.h                         ; Inter Core Communication OS types

.\code\linux\os\LinuxOS                                ; Inter Core Communication Linux OS files
.\code\linux\os\LinuxOS\ICC_Os_Asr.c                   ; Inter Core Communication Linux API implementation

.\code\linux\src
.\code\linux\src\ICC_Api.c                             ; Inter Core Communication API implementation
.\code\linux\src\ICC_Fifo.c                            ; Inter Node Communication FIFO API implementation
.\code\linux\src\ICC_Sig_Fifo.c                        ; Inter Node Communication Signal FIFO API implementation


===== Sample ====

.\sample                                               ; The Inter Core Communication sample directory

.\sample\linux_app                                     ; The Inter Core Communication Linux Sample directory

.\sample\linux_app\common_cfg.mak                      ; ICC Linux Configuration makefile for cross-compiler, Linux Kernel directory.
.\sample\linux_app\readme.txt                          ; Linux Sample Readme.

.\sample\linux_app\ICC_module                          ; ICC Linux library directory
.\sample\linux_app\ICC_module\ICC_lib.c                ; ICC Linux library file
.\sample\linux_app\ICC_module\Kbuild                   ; ICC Linux library's kbuild file
.\sample\linux_app\ICC_module\makefile                 ; ICC Linux library's makefile

.\sample\linux_app\ICC_Sample_module                   ; ICC Linux Sample directory
.\sample\linux_app\ICC_Sample_module\ICC_Sample_dev.c  ; ICC Linux Sample kernel device
.\sample\linux_app\ICC_Sample_module\ICC_Sample.c      ; ICC Linux Sample implementation file
.\sample\linux_app\ICC_Sample_module\ICC_Sample.h      ; ICC Linux Sample header file
.\sample\linux_app\ICC_Sample_module\Kbuild            ; ICC Linux Sample's kbuild file
.\sample\linux_app\ICC_Sample_module\makefile          ; ICC Linux library's makefile
