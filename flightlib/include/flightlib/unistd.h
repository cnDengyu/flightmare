// ONLY FOR WINDOWS:
// copy this file to C:\Program Files\Microsoft Visual Studio\20xx\Community\VC\Tools\MSVC\xx.xx.xxxxx\include
#ifndef _UNISTD_H

#define _UNISTD_H
#define _WINSOCKAPI_
#include <windows.h>

//#include <io.h>
//#include <process.h>
#define sleep(sec)   Sleep(sec * 1000)
#define msleep(msec) Sleep(msec)
#define usleep(usec) Sleep(usec / 1000)
#endif