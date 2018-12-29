#ifndef __FONE_STD_H__
#define __FONE_STD_H__

#if defined(WIN32)
#define FLYFOX_DLL_EXPORT_API  __declspec(dllexport)
#define FLYFOX_DLL_IMPORT_API  __declspec(dllimport) 
#else
#define FLYFOX_DLL_EXPORT_API
#define FLYFOX_DLL_IMPORT_API
#endif

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#define FF_CONST const
#define FF_NULL NULL

#define FF_SUCCESS	0
#define FF_FAILED	-1

typedef int FF_BOOL;
#define FF_TRUE     1
#define FF_FALSE    0


#ifndef FF_UNREFERENCED_PARAMETER
#define FF_UNREFERENCED_PARAMETER(P){(P);}
#endif

#define FF_MIN(x,y)        (((x)<(y))?(x):(y))

typedef unsigned int       FF_SIZE;
typedef char               FF_CHAR;
typedef int                FF_INT;
typedef unsigned int       FF_UINT;
typedef long               FF_LONG;
typedef short              FF_SHORT;
typedef unsigned long      FF_DWORD;
typedef unsigned short     FF_WORD;
typedef unsigned char      FF_BYTE;
typedef unsigned char      FF_UTF8;
typedef unsigned short     FF_WCHAR;
typedef float              FF_FLOAT;
typedef double             FF_DOUBLE;
typedef void               FF_VOID;

typedef unsigned __int8    FF_UINT8;
typedef short              FF_INT16;

#if defined(WIN32)
typedef __int64			   FF_INT64;
typedef unsigned __int64   FF_UINT64;
#else
typedef long long          FF_INT64;
typedef unsigned long long FF_UINT64;
#endif

#define FF_MAKEDWORD(a, b) ((FF_DWORD)(((FF_WORD)(a)) | ((FF_DWORD)((FF_WORD)(b))) << 16))
#define FF_MAKEWORD(a, b)  ((FF_WORD)(((FF_BYTE)(a)) | ((FF_WORD)((FF_BYTE)(b))) << 8))
#define FF_LOWORD(l)       ((FF_WORD)(l))
#define FF_HIWORD(l)       ((FF_WORD)(((FF_DWORD)(l) >> 16) & 0xFFFF))
#define FF_LOBYTE(w)       ((FF_BYTE)(w))
#define FF_HIBYTE(w)       ((FF_BYTE)(((FF_WORD)(w) >> 8) & 0xFF))

typedef struct ff_time {
    FF_INT year;
    FF_INT mon;
    FF_INT day;
    FF_INT hour;
    FF_INT min;
    FF_INT sec;
    FF_INT millisec;
} FF_TIME;

typedef struct tagFF_Rect {
	FF_INT left;
	FF_INT top;
	FF_INT right;
	FF_INT bottom;
} FF_RECT;

typedef unsigned int FF_WPARAM;
typedef long FF_LPARAM;

typedef FF_INT ff_http_handle;
typedef FF_INT	FLYFOX_TIMER_ID;

#define FF_INFINITE 0xFFFFFFFF;

#if defined(WIN32)
#define FF_WAIT_TIMEOUT     258L
#define FF_STATUS_WAIT_0    ((FF_DWORD   )0x00000000L)  
#define FF_WAIT_OBJECT_0    ((FF_STATUS_WAIT_0 ) + 0 )
#endif

#define FF_HANDLE  FF_INT

#endif
