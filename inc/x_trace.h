/**
 * @file      x_trace.h
 * @date      2002.6.03 20:05:12
 * @author    leizi email:847088355@qq.com
 * @copyright GNU Public License.
 * @remark    window trace

-- remark��
  0. surport: WIN32,MFC,Dll,UNICODE
  1. ����Ӧ��������,����,���������ֵǰ���Զ���ӱ���������
  2. �����˳�ʱ,������ַ������浽debug/DebugData.txt
  4. wtrace(x,y,z,w)��ԭ����4������,����������������������,�������ѽ�ֹ��4002,4003���ྯ��

-- ʹ��˵��:
  1. �� "trace.h"  ���Ƶ�����Ŀ¼��(���Բ���ӵ�����).
  2. NO_TRACE_WINDOW,TRACE_WINDOW:��û����ʱ,DEBUG�汾��������ַ���,Release�汾����
     #define NO_TRACE_WINDOW :DEBUG Release �汾�����������������ַ���
     #define TRACE_WINDOW :DEBUG Release �汾���������������ַ���

-- demo
  wtrace(X,Y,Z,W)  :����������ͱ���,�� int double short POINT RECT string
  wtracef()        :��ʽ���ַ���,����sprintf
  wtraceLastError():���ϵͳ�������,������GetLastError()
  wtraceRet(X,Y)   :��X=true,���"Y: successful" ; x=false,���"Y: failed"
  wtraceClear()    :��մ���

  wtracef(_T("tracef ������ %s"),_T("sprintf")); //��ʽ���ַ���,����sprintf
  int       a = 45;         short   d = 324;
  double    b = 67.45;      double  c = 847.424;
  CString   mfcstr;
  CPoint    point(88,421);
  CRect     rect(19,423,56,522);
  HWND      hWnd = ::GetForegroundWindow();
  wtrace(point,rect,mfcstr,hWnd);
  wtrace(a,b,c,d);
 *
 */
#ifndef __EASY_TRACE_H__
#define __EASY_TRACE_H__

#ifdef _AFX

#ifdef NO_TRACE_WINDOW
#else
    #ifdef  _DEBUG
        #define TRACE_WINDOW
    #else
        #ifdef  RELEASE_TRACE
            #define TRACE_WINDOW
        #endif
    #endif
#endif

#pragma warning(disable: 4002 4003 4996 4311 4302 4312)

#ifndef TRACE_WINDOW
    #define wtrace(X)
    #define wtracef  (void)0
    #define wtraceLastError()
    #define wtraceRet(X,Y)
    #define wtraceClear()
#endif

#ifdef TRACE_WINDOW

#define wtrace(X,Y,Z,W) {CEasyTrace traceobjtemp;    \
        traceobjtemp.name(#X);  traceobjtemp.MyTrace(X);    \
        traceobjtemp.name(#Y);  traceobjtemp.MyTrace(Y);    \
        traceobjtemp.name(#Z);  traceobjtemp.MyTrace(Z);    \
        traceobjtemp.name(#W);  traceobjtemp.MyTrace(W);}

#define wtracef              CEasyTrace::MyTracef
#define wtraceLastError()    {CEasyTrace traceobjtemp;traceobjtemp.TraceSysError();}
#define wtraceRet(X,Y)       {CEasyTrace traceobjtemp;traceobjtemp.TraceResult(X,_T(#Y));}
#define wtraceClear()        {::SendMessage(::FindWindow(NULL,TRACE_WND_TEXT),WM_COMMAND,3,NULL);}


#define TRACE_WND_TEXT _T("TraceWnd") // ���ڱ���
#define WM_CREATE_WND   WM_USER+888   // �Զ�����Ϣ
#define AUTO_HIDE_TIME  3000          // ������ٺ�����Զ�����
#ifndef _WINDOWS_
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef WIN32_LEAN_AND_MEAN
#endif
#include <tchar.h>
#include <sstream>
#include <locale.h>

class CTraceWnd
{
public:
    ~CTraceWnd()
    {
        ::DestroyWindow(::FindWindow(NULL, TRACE_WND_TEXT));
    }

    static TCHAR *GetIndex(bool IsReset = false)
    {
#define INDDEX_NUM 4  // �����λ��
        static size_t num = 1;
        static TCHAR buf[11] = {0};
        static TCHAR buf2[11] = {0};
        if (IsReset) {
            num = 1;
            return NULL;
        }
        _ultot((unsigned long)num++, buf, 10);
        size_t len = _tcslen(buf);
        size_t count = INDDEX_NUM - len;
        if (count > 0) {
            for (size_t i = 0; i < count; i++) {
                buf2[i] = ' ';
            }
            _tcscpy(buf2 + count, buf);
            return buf2;
        } else
            return buf;
    }

    // ����ַ���������
    static void CTraceWnd::PrintString(const TCHAR *OutputStr)
    {
        HWND hWnd =::FindWindow(NULL, TRACE_WND_TEXT);

        // ���������,�򴴽����ڲ���ʼ������
        if (!hWnd) {
            // �������������Ϊ�˳����˳�ʱ��������������
            // �����ļ��͹رմ���,����֮��û�������ô���
            static CTraceWnd ForGetCloseMessage;

            hWnd = (HWND)WindowProc(NULL, WM_CREATE_WND, NULL, NULL);
            if (!hWnd)  return;
        }

        ::SendMessage(hWnd, EM_REPLACESEL, FALSE, (LPARAM)OutputStr);
    }

    static LRESULT CALLBACK CTraceWnd::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        static WNDPROC  m_OldWndProc = NULL;
        static BOOL     m_IsAutoWndWidth = TRUE;
        static BOOL     m_IsAutoHide = false;
        static BOOL     m_IsHide = false;
        static int      m_WindowWidth = 200;
        static HWND     StaticWnd = NULL;


        switch (message) {
        case WM_CREATE_WND: { // ����Edit���� ,�����Զ�����Ϣ
            int x = GetProfileInt(TRACE_WND_TEXT, _T("x"), 0);
            int y = GetProfileInt(TRACE_WND_TEXT, _T("y"), 0);
            int cx = GetProfileInt(TRACE_WND_TEXT, _T("cx"), 200);
            int cy = GetProfileInt(TRACE_WND_TEXT, _T("cy"), 600);
            m_IsAutoWndWidth = GetProfileInt(TRACE_WND_TEXT, _T("m_IsAutoWndWidth"), 1);
            m_IsAutoHide    = GetProfileInt(TRACE_WND_TEXT, _T("m_IsAutoHide"), 0);
            m_IsHide        = GetProfileInt(TRACE_WND_TEXT, _T("m_IsHide"), false);

            // ������Ļ���µ���
            int SrcCx = GetSystemMetrics(SM_CXSCREEN);
            int SrcCy = GetSystemMetrics(SM_CYSCREEN);
            if (x < 0 || x >= SrcCx) x = 0;
            if (y < 0 || y >= SrcCy) y = 0;
            if (cx < 100) cx = 100;
            if (cy < 40)  cy = 40;
            if (x + cx > SrcCx) x = SrcCx - cx;
            if (y + cy > SrcCy) y = SrcCy - cy;

            // �������������״̬,�޸�x��ֵ
            if (m_IsHide)
                x = -cx + 4;

            HWND hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, TEXT("Edit"), TRACE_WND_TEXT ,
                                       WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL |
                                       ES_AUTOHSCROLL | ES_WANTRETURN | ES_MULTILINE ,
                                       x, y, cx, cy,
                                       NULL, NULL, NULL, NULL);

            if (hWnd == NULL) return NULL;

            // �����˵�,����ӵ�����
            HMENU hMenu = CreateMenu();
            AppendMenu(hMenu, MF_STRING, 3, _T("��մ���"));
            AppendMenu(hMenu, MF_STRING, 4, m_IsAutoWndWidth ? _T("��ֹ�Զ��������") : _T("�����Զ��������"));
            AppendMenu(hMenu, MF_STRING, 5, _T("���ص���Ļ���"));
            AppendMenu(hMenu, MF_STRING, 6, m_IsAutoHide ? _T("��ֹ�Զ�����") : _T("�����Զ�����"));
            ::SetMenu(hWnd, hMenu);

            // �޸�edit��������ַ�����
            ::SendMessage(hWnd, EM_SETLIMITTEXT, 1000000, 0);

            // ���Edit
            ::SendMessage(hWnd, WM_SETTEXT, NULL, (LPARAM)_T(""));

            // �����������
            LOGFONT LogFont;
            ::memset(&LogFont, 0, sizeof(LOGFONT));
            lstrcpy(LogFont.lfFaceName, _T("Fixedsys"));
            LogFont.lfHeight = -12;
            LogFont.lfCharSet = GB2312_CHARSET;
            HFONT hFont = CreateFontIndirect(&LogFont);
            ::SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, 0);

            // ��hWnd��Ĭ�ϴ��ڹ����滻ΪWindowProc,����Ĭ�Ϻ������̵ĺ���ָ��
            m_OldWndProc = (WNDPROC)SetWindowLong(hWnd, -4, (LONG)WindowProc);  // GWL_WNDPROC
            if (!m_OldWndProc)
                return NULL;

            // ��ʾʱ�޽���
            ::ShowWindow(hWnd, SW_SHOWNOACTIVATE);

            // ���û�п����Զ����ز����ִ�������״̬,�򵯳���ʾ����
            if (!m_IsAutoHide && m_IsHide) {
                StaticWnd = CreateWindowEx(WS_EX_TOPMOST, _T("Static"),
                                           _T("Trace������������Ļ���\n�����Ļ��߿ɻָ�����"),
                                           WS_POPUP | WS_VISIBLE | WS_BORDER
                                           | SS_CENTER ,
                                           0, y, 200, 40, NULL, NULL, NULL, NULL);

                SetTimer(hWnd, 2, 2000, NULL);
            }

            return (LRESULT)hWnd;
        }
        break;

        case WM_TIMER: {
            ::KillTimer(hWnd, 1);

            if (wParam == 1) {
                // ���ش���
                m_IsHide = true;
                RECT rect;
                ::GetWindowRect(hWnd, &rect);
                ::SetWindowPos(hWnd, HWND_TOPMOST, rect.left - rect.right + 4, rect.top, 0, 0, SWP_NOSIZE);
            } else if (wParam == 2) {
                ::PostMessage(StaticWnd, WM_CLOSE, NULL, NULL); // �ر���ʾ����
            }

        }
        break;

        case WM_ACTIVATE: {
            if (m_IsAutoHide) {
                if (LOWORD(wParam) == WA_INACTIVE) { // ����ʧȥ����
                    if (m_IsHide == false) {
                        m_IsHide = true;
                        ::SetTimer(hWnd, 1, 200, NULL); // ��ʱ100�������ش���
                    }
                } else { // ���ڱ�����
                    ::KillTimer(hWnd, 1);
                }
            }
        }
        break;

        case WM_NCLBUTTONDOWN: {
            if (m_IsHide) {
                // ������ʾ����
                m_IsHide = false;
                int x = GetProfileInt(TRACE_WND_TEXT, _T("x"), 0);
                int y = GetProfileInt(TRACE_WND_TEXT, _T("y"), 0);
                ::SetWindowPos(hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
            } else {
                // ����Ĭ�ϲ�������ı䴰�ڵ�λ�ô�С
                LRESULT rel =::CallWindowProc(m_OldWndProc, hWnd, message, wParam, lParam);

                if (wParam == HTLEFT   || wParam == HTTOP        || wParam == HTRIGHT       || wParam == HTBOTTOM || wParam == HTCAPTION ||
                        wParam == HTTOPLEFT || wParam == HTTOPRIGHT   || wParam == HTBOTTOMLEFT  || wParam == HTBOTTOMRIGHT) {
                    // ���洰������
                    TCHAR buf2[11];
                    _itot(m_IsAutoWndWidth, buf2, 10);
                    WriteProfileString(TRACE_WND_TEXT, _T("m_IsAutoWndWidth"), buf2);
                    _itot(m_IsAutoHide    , buf2, 10);
                    WriteProfileString(TRACE_WND_TEXT, _T("m_IsAutoHide"),    buf2);
                    _itot(m_IsHide        , buf2, 10);
                    WriteProfileString(TRACE_WND_TEXT, _T("m_IsHide"),        buf2);
                    if (!m_IsHide) {
                        RECT rect;
                        ::GetWindowRect(hWnd, &rect);
                        _itot(rect.left, buf2, 10);
                        WriteProfileString(TRACE_WND_TEXT, _T("x"), buf2);
                        _itot(rect.top, buf2, 10);
                        WriteProfileString(TRACE_WND_TEXT, _T("y"), buf2);
                        _itot(rect.right - rect.left, buf2, 10);
                        WriteProfileString(TRACE_WND_TEXT, _T("cx"), buf2);
                        _itot(rect.bottom - rect.top, buf2, 10);
                        WriteProfileString(TRACE_WND_TEXT, _T("cy"), buf2);
                    }
                }

                return rel;
            }
        }
        break;

        case EM_REPLACESEL: {
            // ���ָ�����
            ::SendMessage(hWnd, EM_SETSEL, 4294967290, 4294967290);

            // ����Ĭ�Ϲ�������ַ�����edit ����
            ::CallWindowProc(m_OldWndProc, hWnd, message, wParam, lParam);

            // �������Զ����ع���
            if (m_IsAutoHide) {
                if (m_IsHide) {
                    // ������ʾ����
                    m_IsHide = false;
                    int x = GetProfileInt(TRACE_WND_TEXT, _T("x"), 0);
                    int y = GetProfileInt(TRACE_WND_TEXT, _T("y"), 0);
                    ::SetWindowPos(hWnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
                }

                ::SetTimer(hWnd, 1, AUTO_HIDE_TIME, NULL);
            }

            // �������Զ��������ڵĿ�ȹ���
            if (m_IsAutoWndWidth) {
                if (!m_IsHide) {
                    int MinPos, MaxPos;
                    GetScrollRange(hWnd, SB_HORZ, &MinPos, &MaxPos);
                    if (m_WindowWidth < MaxPos + 40) {
                        m_WindowWidth = MaxPos + 40;
                        RECT rect;
                        ::GetWindowRect(hWnd, &rect);
                        ::SetWindowPos(hWnd, NULL, 0, 0, m_WindowWidth, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOACTIVATE);
                    }
                }
            }
        }
        break;

        case WM_DESTROY: {
            // ɾ������
            DeleteObject((HFONT)::SendMessage(hWnd, WM_GETFONT, 0, 0));
            // ɾ���˵�
            DestroyMenu(GetMenu(hWnd));

            // ���ó������е�Ŀ¼�������ļ�����Ŀ¼��ͬ
            // �����ǵĳ������������ʱ,���Ĺ���Ŀ¼�ʹ�������.
            // �������Ŀ¼����ͬ��.����������Ҫ��Ŀ¼�Ļ���
            TCHAR AppPath[MAX_PATH];
            int nlen = GetModuleFileName(NULL, AppPath, MAX_PATH);
            while (AppPath[--nlen] != '\\');
            AppPath[nlen] = '\0';
            SetCurrentDirectory(AppPath);

            // ��ȡ���ڵ��ַ���
            LRESULT len =::SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
            TCHAR *WindowText = new TCHAR[(len + 1)*sizeof(TCHAR)];
            ::GetWindowText(hWnd, WindowText, (int)(len + 1)); // GetWindowText���Զ����NULL������

            // �����ڱ��������ļ�DebugData.txt,
            FILE *fp = fopen("DebugData.txt", "wb");

#ifdef  _UNICODE// ���������unicode,����תΪansi�ٱ���
            // len��Դ�ַ����ĳ��� len=_tcslen(WindowText)
            // (len+1)*2��Դ�ַ���ռ�õ��ڴ�ռ�,����NULL������
            // nChars����ת�����ַ�����,������NULL,��nChars=_tcslen(buf)
            // wcstombs���Զ����NULL������
            char *buf = new char[(len + 1) * 2];
            setlocale(LC_ALL, ".936");
            size_t nChars = wcstombs(buf, WindowText, (len + 1) * 2);
            setlocale(LC_ALL, "C");
            fwrite(buf, 1, nChars, fp);
            delete[] buf;
#else
            fwrite(WindowText, 1, len, fp);
#endif
            delete[] WindowText;
            fclose(fp);

            // ���洰������
            TCHAR buf2[11];
            _itot(m_IsAutoWndWidth, buf2, 10);
            WriteProfileString(TRACE_WND_TEXT, _T("m_IsAutoWndWidth"), buf2);
            _itot(m_IsAutoHide    , buf2, 10);
            WriteProfileString(TRACE_WND_TEXT, _T("m_IsAutoHide"),    buf2);
            _itot(m_IsHide        , buf2, 10);
            WriteProfileString(TRACE_WND_TEXT, _T("m_IsHide"),        buf2);
            if (!m_IsHide) {
                RECT rect;
                ::GetWindowRect(hWnd, &rect);
                _itot(rect.left, buf2, 10);
                WriteProfileString(TRACE_WND_TEXT, _T("x"), buf2);
                _itot(rect.top, buf2, 10);
                WriteProfileString(TRACE_WND_TEXT, _T("y"), buf2);
                _itot(rect.right - rect.left, buf2, 10);
                WriteProfileString(TRACE_WND_TEXT, _T("cx"), buf2);
                _itot(rect.bottom - rect.top, buf2, 10);
                WriteProfileString(TRACE_WND_TEXT, _T("cy"), buf2);
            }
        }
        break;

        case WM_CLOSE:
            GetIndex(true);
            ::DestroyWindow(hWnd);
            break;

        case WM_KEYDOWN: {
            // select all
            if (wParam == 'A' && GetKeyState(VK_CONTROL) < 0)
                ::SendMessage(hWnd, EM_SETSEL, 0, -1);
        }
        break;

        case WM_COMMAND: {
            switch (wParam) {
            case 3: { // �������
                GetIndex(true);
                ::SendMessage(hWnd, WM_SETTEXT, NULL, (LPARAM)_T(""));
            }
            break;
            case 4: { // modify the Auto Adjust window width option
                m_IsAutoWndWidth = !m_IsAutoWndWidth;
                HMENU hMenu = GetMenu(hWnd);
                ModifyMenu(hMenu, 1, MF_BYPOSITION | MF_STRING, 4,
                           m_IsAutoWndWidth ? _T("��ֹ�Զ��������") : _T("�����Զ��������"));
                SetMenu(hWnd, hMenu);
            }
            break;
            case 5: { // hide window int the left of the screen
                // ���ش���
                m_IsHide = true;
                RECT rect;
                ::GetWindowRect(hWnd, &rect);
                ::SetWindowPos(hWnd, HWND_TOPMOST, rect.left - rect.right + 4, rect.top, 0, 0, SWP_NOSIZE);
            }
            break;
            case 6: { // modify the Auto hide window option
                m_IsAutoHide = !m_IsAutoHide;
                HMENU hMenu = GetMenu(hWnd);
                ModifyMenu(hMenu, 3, MF_BYPOSITION | MF_STRING, 6,
                           m_IsAutoHide ? _T("��ֹ�Զ�����") : _T("�����Զ�����"));
                SetMenu(hWnd, hMenu);
            }
            break;
            }
        }
        break;

        default:
            return ::CallWindowProc(m_OldWndProc, hWnd, message, wParam, lParam);
        }
        return 0;
    }
};


class CEasyTrace
{
#if defined(_UNICODE) || defined(UNICODE)
    std::wostringstream FormatString;
#else
    std::ostringstream FormatString;
#endif

public:
    void MyTrace() {} // ��trace���������ʱ����ô˺���
    void name() {}   // ��trace���������ʱ����ô˺���

    CEasyTrace()
    {
        FormatString << CTraceWnd::GetIndex() << _T("  "); // �кź��������ո�
    }

    ~CEasyTrace()
    {
        FormatString << _T("\r\n"); // ��������С�
#ifdef  _CONSOLE
#if defined(_UNICODE) || defined(UNICODE)
        wprintf(_T("%s"), std::wstring(FormatString.str()).c_str());
#else
        printf("%s", std::string(FormatString.str()).c_str());
#endif
#else
#if defined(_UNICODE) || defined(UNICODE)
        CTraceWnd::PrintString(std::wstring(FormatString.str()).c_str());
#else
        CTraceWnd::PrintString(std::string(FormatString.str()).c_str());
#endif
#endif
    }

    void name(char *pStr)
    {
        if (strchr(pStr, '\"') != NULL) return;

#if defined(_UNICODE) || defined(UNICODE)
        size_t len = strlen(pStr);
        wchar_t *buf;
        buf = new wchar_t[len + 1];
        setlocale(LC_ALL, ".936");
        mbstowcs(buf, pStr, len + 1);
        setlocale(LC_ALL, "C");
        FormatString << buf << _T("\t\t=  ");
        delete []buf;
#else
        FormatString << pStr << _T("\t\t=  ");
#endif
    }

    template <class T> void MyTrace(T t)
    {
        FormatString << t << _T(";\r\n      ");
    }

    static void MyTracef(LPCTSTR lpszFormat, ...)
    {
        va_list args;
        va_start(args, lpszFormat);

        TCHAR szBuffer[512];
        int rel = _vsntprintf(szBuffer, sizeof(szBuffer) / sizeof(TCHAR) - 1, lpszFormat, args);
        if (rel == -1)
            szBuffer[sizeof(szBuffer) / sizeof(TCHAR) - 1] = NULL;

        va_end(args);

        CEasyTrace traceobjtemp;
        traceobjtemp.MyTrace(szBuffer);
    }

    void MyTrace(POINT point)
    {
        FormatString << (_T("(")) << (point.x) << (_T(",")) << (point.y) << (_T(");\r\n      "));
    }

    void MyTrace(RECT rect)
    {
        FormatString << (_T("(")) << (rect.left) << (_T(",")) << (rect.top) << (_T(",")) << (rect.right) << (_T(",")) << (rect.bottom) << (_T(")-(")) << (rect.right - rect.left) << (_T(",")) << (rect.bottom - rect.top) << (_T(");\r\n      "));
    }

    void MyTrace(HWND hWnd)
    {
        if (!::IsWindow(hWnd)) {
            FormatString << _T("Invalid Window");
            return;
        }

        TCHAR WindowText[40];
        ::SendMessage(hWnd, WM_GETTEXT, (WPARAM)sizeof(WindowText) / sizeof(TCHAR), (LPARAM)WindowText);
        TCHAR ClassName[40];
        ::GetClassName(hWnd, ClassName, sizeof(ClassName) / sizeof(TCHAR));
        RECT  rect;
        ::GetWindowRect(hWnd, &rect);

        FormatString
                << _T("\r\n      ------------------------------------------------")
                << _T("\r\n      Window Title:\t\"") << WindowText
                << _T("\"\r\n      Class Name:\t\"") << ClassName
                << _T("\"\r\n      Handle:\t\t0x") << HWND(hWnd)
                << _T("\r\n      Window Rect:\t");
        FormatString << (_T("(")) << (rect.left) << (_T(",")) << (rect.top) << (_T(",")) << (rect.right) << (_T(",")) << (rect.bottom) << (_T(")-(")) << (rect.right - rect.left) << (_T(",")) << (rect.bottom - rect.top) << (_T(");"));
        FormatString << _T("\r\n      ------------------------------------------------\r\n");
    }

#ifdef __AFXWIN_H__
    void MyTrace(CPoint point)
    {
        FormatString << (_T("(")) << (point.x) << (_T(",")) << (point.y) << (_T(");\r\n      "));
    }
    void MyTrace(CRect rect)
    {
        FormatString << (_T("(")) << (rect.left) << (_T(",")) << (rect.top) << (_T(",")) << (rect.right) << (_T(",")) << (rect.bottom) << (_T(")-(")) << (rect.right - rect.left) << (_T(",")) << (rect.bottom - rect.top) << (_T(");\r\n      "));
    }
    void MyTrace(CString str)
    {
        FormatString << (LPTSTR)(LPCTSTR)str << _T(";\r\n      ");
    }
#endif

    void TraceSysError()
    {
        FormatString << _T("System Error Codes:") << GetLastError() << _T("\r\n");
    }

    void TraceResult(bool IsOk, TCHAR *str)
    {
        FormatString << str << _T("\t\t") << (IsOk ? _T(": Successful") : _T(": Failed")) << _T("\r\n");
    }
};

#endif // #define TRACE_WINDOW
#endif // _AFX
#endif // #define __EASY_TRACE_H__
