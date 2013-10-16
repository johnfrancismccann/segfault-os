// All necessary declarations for the Tux Controller driver must be in this file

#ifndef TUXCTL_H
#define TUXCTL_H

#define TUX_SET_LED _IOR('E', 0x10, unsigned long)
#define TUX_BUTTONS _IOW('E', 0x12, unsigned long*)
#define TUX_INIT _IO('E', 0x13)

#define MSK0 0xF; //mask lowest byte
#define MSK1 0xFF; //mask lowest 2 bytes
#define MSK2 0xFFF; //mask lowest 3 bytes
#define MSK3 0xFFFF; //mask lowest 4 bytes

#define MSK12 0x6; //manipulate bits 1 and 2 only

#endif
