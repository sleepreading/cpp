/**
 * @file      ztypes.h
 * @date      2013-7-29 9:56:17
 * @author    leizi
 * @copyright zhanglei. email:847088355@qq.com
 * @remark    types & functions that compatible with windows
 */
#ifndef __Z_TYPES_H__
#define __Z_TYPES_H__

#ifndef _WIN32

    #define VOID            void
    typedef void           *PVOID;
    typedef void           *LPVOID;
    typedef void           *HANDLE;
    typedef HANDLE          HGLOBAL;

    typedef int SOCKET;
    typedef int BOOL;
    typedef int INT;
    typedef int LPARAM;
    typedef unsigned int    WPARAM;
    typedef unsigned int    UINT;
    typedef unsigned int   *PUINT;

    typedef char            CHAR;
    typedef unsigned char   BYTE;
    typedef unsigned char   UCHAR;
    typedef const char     *LPCTSTR;
    typedef const char     *LPCSTR;
    typedef const char     *LPSTR;

    typedef short           SHORT;
    typedef unsigned short  wchar_t;
    typedef unsigned short  USHORT;
    typedef unsigned short  WORD;
    typedef long            LONG;
    typedef long           *LPLONG;
    typedef unsigned long   DWORD;
    typedef unsigned long   ULONG;
    typedef float           FLOAT;

    typedef BYTE           *PBYTE;
    typedef BYTE           *LPBYTE;
    typedef WORD            ATOM;
    typedef WORD           *PWORD;
    typedef WORD           *LPWORD;

    typedef struct sockaddr_in  SOCKADDR_IN;
    typedef struct sockaddr     SOCKADDR;
    typedef struct hostent      HOSTENT;
    typedef struct hostent     *PHOSTENT;
    typedef struct in_addr      IN_ADDR;
    typedef struct in_addr     *PIN_ADDR;
    typedef struct in_addr     *LPIN_ADDR;

    #define far
    #define near
    #define IN
    #define OUT
    #define FALSE           0
    #define TRUE            1

    #ifdef __cplusplus
        #define NULL            0
    #else
        #define NULL            ((void *)0)
    #endif

    #define __stdcall
    #define pascal          __stdcall
    #define WINAPI          __stdcall
    #define CALLBACK        __stdcall
    #define PASCAL          pascal
    #define APIENTRY        WINAPI

    #define SOCKET_ERROR    (-1)
    #define MAX_PATH        260
    #define INFINITE        0xFFFFFFFF
    #define INVALID_SOCKET  (SOCKET)(~0)


   /*
    * win32 macros
    */
    #define stricmp         strcasecmp
    #define wcsicmp         wcscasecmp
    #define _findfirst      findfirst
    #define _findnext       findnext
    #define _findclose      findclose
    #define sleep(x)        Sleep(x*1000)
    #define MAKEWORD(a, b)  ((WORD)(((BYTE)(a)) | ((WORD)((BYTE)(b))) << 8))
    #define MAKELONG(a, b)  ((LONG)(((WORD)(a)) | ((DWORD)((WORD)(b))) << 16))
    #define LOWORD(l)       ((WORD)(l))
    #define HIWORD(l)       ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
    #define LOBYTE(w)       ((BYTE)(w))
    #define HIBYTE(w)       ((BYTE)(((WORD)(w) >> 8) & 0xFF))

    #ifndef ASSERT
        #ifdef _DEBUG
            #include <assert.h>
            #define ASSERT(f)   assert(f)
        #else
            #define ASSERT(f)   ((void)0)
        #endif
    #endif

    #ifndef VERIFY
        #ifdef _DEBUG
            #define VERIFY(f)   ASSERT(f)
        #else
            #define VERIFY(f)   ((void)(f))
        #endif
    #endif

    #if defined(_UNICODE) || defined(UNICODE)
        typedef wchar_t TCHAR;
        #ifndef _T
            #define _T(s) L##s
        #endif
        #ifndef _TSTR
            #define _TSTR(s) L##s
        #endif
    #else
        typedef char TCHAR;
        #ifndef _T
            #define _T(s) s
        #endif
        #ifndef _TSTR
            #define _TSTR(s) s
        #endif
    #endif

    #ifndef NOMINMAX
        #ifndef max
            #define max(a,b)        (((a) > (b)) ? (a) : (b))
        #endif
        #ifndef min
            #define min(a,b)        (((a) < (b)) ? (a) : (b))
        #endif
    #endif  // NOMINMAX

#endif  // !define _WIN32


/**
 * portable types
 */
#ifndef int64
    #if defined(_MSC_VER) &&  _MSC_VER<1310
        typedef __int64 int64;
    #else
        typedef long long int int64;
    #endif
#endif  // int64

#ifndef uint64
    #if defined(_MSC_VER) &&  _MSC_VER<1310
        typedef unsigned __int64 uint64;
    #else
        typedef unsigned long long int uint64;
    #endif
#endif  // uint64

#ifndef int32
    typedef int int32;
#endif  // int32

#ifndef uint32
    typedef unsigned int uint32;
#endif  // uint32

#ifndef int16
    typedef short int16;
#endif  // int16

#ifndef uint16
    typedef unsigned short uint16;
#endif  // uint16

#ifndef int8
    typedef char int8;
#endif  // int8

#ifndef uint8
    typedef unsigned char uint8;
#endif  // uint8

#ifndef byte
    typedef unsigned char byte;
#endif  // byte


/* misc */
#ifndef tstring
    #if defined(_UNICODE) || defined(UNICODE)
        #define tstring wstring
    #else
        #define tstring string
    #endif
#endif

#ifndef tifstream
    #if defined(_UNICODE) || defined(UNICODE)
        #define tifstream wifstream
    #else
        #define tifstream ifstream
    #endif
#endif

#ifndef tofstream
    #if defined(_UNICODE) || defined(UNICODE)
        #define tofstream wofstream
    #else
        #define tofstream ofstream
    #endif
#endif

#ifndef tfstream
    #if defined(_UNICODE) || defined(UNICODE)
        #define tfstream wfstream
    #else
        #define tfstream fstream
    #endif
#endif

#ifndef tiostream
    #if defined(_UNICODE) || defined(UNICODE)
        #define tiostream wiostream
    #else
        #define tiostream iostream
    #endif
#endif

#ifndef tistream
    #if defined(_UNICODE) || defined(UNICODE)
        #define tistream wistream
    #else
        #define tistream istream
    #endif
#endif

#ifndef tostream
    #if defined(_UNICODE) || defined(UNICODE)
        #define tostream wostream
    #else
        #define tostream ostream
    #endif
#endif

#endif  // zTypes.h

