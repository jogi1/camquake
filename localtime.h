#ifndef __LOCALTIME_H__
#define __LOCALTIME_H__

#ifdef _WIN32

typedef int INT;
#include <windef.h>
#include <windows.h> // FIXME: we should include it only at winquake.h

#else

#include <time.h>

typedef struct SYSTEMTIME_s
{
    int wYear;
    int wMonth;
    int wDay;
    int wDayOfWeek;
    int wHour;
    int wMinute;
    int wSecond;
    int wMilliseconds;
} SYSTEMTIME;

void GetLocalTime(SYSTEMTIME *);

#endif


int  GetFileLocalTime(char *, SYSTEMTIME *);
void UnixtimeToWintime(SYSTEMTIME *, struct tm *);
int  SYSTEMTIMEcmp(const SYSTEMTIME *, const SYSTEMTIME *);

#endif // __LOCALTIME_H__








