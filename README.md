SegFault-OS
===========

Overview
-----------
Segfault-OS is a basic operating system that was originally built for the final, team-project in ECE 391 at UIUC. Before starting the project, we named it Segfault OS as a joke in anticipation of the many segfaults that would be encountered while developing and testing, but, as it stands, it's actually reasonably stable. The OS is written for an i386 processor, and provides concurrent process execution, virtual memory abstraction, a basic file system, and drivers for the keyboard, display, and real-time clock.

Running
-----------
To run Segfault OS, boot from student-distrib/mp3.img disk image on a computer with ideally at least 256 MB of memory. The easiest thing is to just run the OS in a virtual machine such as QEMU or VirtualBox.

Building
-----------
Change into student-distrib function and run "sudo make dep" followed by "sudo make" to build mp3.img. The Makefile was written for compilation on a 32-bit Linux machine. I have had success compiling on a 32-bit Lubuntu system.

More Information
-----------
More information, including a video of Sefault-OS in action, can be found [here](http://www.johnmccann.io/#segfault-os) on my personal website.

Files
-----------

- **createfs:** This program takes a flat source directory (i.e. no subdirectories: in the source directory) and creates a filesystem image in the: format specified for this MP.  Run it with no parameters to see: usage.
-**elfconvert** This program takes a 32-bit ELF (Executable and Linking Format) file - the standard executable type on Linux - and converts it to the executable format specified for this MP.  The output filename is <exename>.converted.
- **fish/** This directory contains the source for the fish animation program. It can be compiled two ways - one for your operating system, and one for Linux using an emulation layer.  The Makefile is currently set up to build "fish" for your operating system using the elfconvert utility described above.  If you want to build a Linux version, do ``` make fish_emulated ```.  You can then run ``` fish_emulated ``` as superuser at a standard Linux console, and you should see the fish animation.
- **fsdir/** This is the directory from which your filesystem image was created. It contains versions of cat, fish, grep, hello, ls, and shell, as well as the frame0.txt and frame1.txt files that fish needs to run. If you want to change files in your OS's filesystem, modify this directory and then run the ``` createfs ``` utility on it to create a new filesystem image.
The only problem with this directory is that there is no "rtc" device file.  This is due to limitations with the Samba network filesystem that your home directories are shared from. If you wish to include the "rtc" device file in this directory (to make the fish animation work properly), you must copy the entire "fsdir" directory to a Linux filesystem (say, in ``` /home/user ```).  Then, as root, run ``` mknod /home/user/fsdir/rtc c 10 61 ```.  Then you can run the ``` createfs ``` utility on ``` /home/user/fsdir ``` to obtain a new filesystem image that contains the rtc device file.
- **LICENSE** Terms of use for this code.  Based on MIT license.
- **README** This file.
- **student-distrib/** This is the directory that contains the source code for your operating system.  Currently, a skeleton is provided that will build and boot you into protected mode, printing out various boot parameters.  Read the INSTALL file in that directory for instructions on how to set up the bootloader to boot this OS.
- **syscalls/** This directory contains a basic system call library that is used by the utility programs such as cat, grep, ls, etc.  The library provides a C interface to the system calls, much like the C library (libc) provides on a real Linux/Unix system.  A few support functions have also been written (things like strlen, strcpy, etc.) that are used by the utility programs.  The Makefile is set up to build these programs for your OS.
