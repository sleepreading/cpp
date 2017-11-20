// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER      0x0501
#define _WIN32_WINNT    0x0501
#define _WIN32_IE   0x0600
#define _RICHEDIT_VER   0x0300

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;
#define _WTL_USE_CSTRING
#include <atlwin.h>
#include <atlmisc.h>


class ICallBack
{
public:
    // 1xx��2xx��3xx��4xx��5xxΪrtsp��Ӧ��Ϣ,600����֡,601�ְ�֡��ʼ,602�ְ�֡�м�,603�ְ�֡����
    virtual long Notify(long lCode, const unsigned char *sHead, const unsigned char *sBody, long lBodyLen) = 0;
};
