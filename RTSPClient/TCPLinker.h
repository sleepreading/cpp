
#pragma once

#include <assert.h>
#include <stdio.h>
#include <winsock.h>
#include <string>
#include <vector>
#include <map>
#pragma comment( lib, "Ws2_32.lib" )
#define WM_SOCK_MSG 3345

// default parser, only output the received data
class CParserDefault
{
public:
    long Take(char *sData, long lLength)
    {
        char c = sData[ lLength ];
        sData[ lLength ] = 0;
        ::OutputDebugString(sData);
        sData[ lLength ] = c;

        return lLength;
    }
};

template< class TParser = CParserDefault >
class CTCPLinkT
{
public:
    enum enumState {
        e_init = 0,
        e_connecting = 1,
        e_connected = 2,
        e_close_by_user = 3,
        e_close_by_net = 4,
        e_close_by_net_error = 5,
        e_close_by_error_data = 6
    };
    enumState m_state;

private:
    long m_PrevWndProc;
    unsigned char *m_sBuffer;
    char *m_sOutBuffer;
    long m_lBufUsed;
protected:
    SOCKET m_socket;
    long m_nErrorCode;
    std::string m_sErrorMessage;
    long m_lTimeCount;
    std::string m_sIP;
    long m_lPort;
    HWND m_hWnd;

private:
    static LRESULT WindowProc_s(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        CTCPLinkT *s = (CTCPLinkT *)GetWindowLong(hWnd, GWL_USERDATA);
        if (s) {
            s->WindowProc(Msg, wParam, lParam);
            if (Msg == WM_DESTROY) {
                s->OutPut("WM_DESTROY\n");
                SetWindowLong(s->m_hWnd, GWL_USERDATA, NULL);
                SetWindowLong(s->m_hWnd, GWL_WNDPROC, (LONG)s->m_PrevWndProc);
                s->m_hWnd = NULL;
            }
        }

        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

public:
    CTCPLinkT() : m_hWnd(NULL), m_PrevWndProc(0), m_socket(INVALID_SOCKET),
        m_nErrorCode(0), m_state(e_init), m_lPort(0), m_lTimeCount(0), m_lBufUsed(0)
    {
        m_sBuffer = (unsigned char *)malloc(64 * 1024 + 1);
        m_sOutBuffer = (char *)malloc(64 * 1024 + 1);
    }
    ~CTCPLinkT()
    {
        Close();
        if (m_hWnd) {
            ::DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }
        free(m_sBuffer);
        free(m_sOutBuffer);
    }

    HWND GetWnd()
    {
        return m_hWnd;
    }
    HWND Create()
    {
        static bool bReg = false;
        if (!bReg) {
            WNDCLASS wndclass = {0};
            wndclass.lpfnWndProc = DefWindowProc;
            wndclass.hInstance = GetModuleHandle(NULL);
            wndclass.lpszClassName = "tcplink_window_class_name";
            RegisterClass(&wndclass);
            bReg = true;
        }

        m_hWnd = ::CreateWindow("tcplink_window_class_name",
                                "tcplink_window_class_name", 0, 0, 0, 0, 0,
                                NULL, NULL, GetModuleHandle(NULL), NULL);
        if (m_hWnd) {
            SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
            m_PrevWndProc = (long)SetWindowLong(m_hWnd, GWL_WNDPROC, (long)WindowProc_s);
        };

        //SetTimer( m_hWnd, 2332, 1000, NULL );
        m_lTimeCount = 0;
        return m_hWnd;
    }
    const char *GetIP()
    {
        return m_sIP.c_str();
    }
    long GetPort()
    {
        return m_lPort;
    }
    void ErrMsg(long nErrorCode)
    {
        m_nErrorCode = nErrorCode;
        m_sErrorMessage = GetErrorMsg(m_nErrorCode);
        OutPut("$$$ Socket error(ip=%s,port=%d):%d, %s\n", m_sIP.c_str(), m_lPort, nErrorCode, m_sErrorMessage.c_str());
    }
    long GetErrorCode()
    {
        return m_nErrorCode;
    }
    std::string GetErrorMsg(long nErrorCode)
    {
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
        std::string sErrorMessage = lpMsgBuf ? (const char *)lpMsgBuf : "";
        LocalFree(lpMsgBuf);

        return sErrorMessage;
    }
    void OutPut(const char *sFormat, ...)
    {
        va_list args;
        va_start(args, sFormat);
        memset(m_sOutBuffer, 0, 1024 * 64);
#if _MSC_VER > 1300
        vsprintf_s(m_sOutBuffer, 1024 * 64, sFormat, args);
#else
        vsprintf(m_sOutBuffer, sFormat, args);
#endif
        ::OutputDebugString(m_sOutBuffer);
    }

    std::string GetMyIP()
    {
        char name[255];
        std::string ip;
        PHOSTENT hostinfo;
        if (gethostname(name, sizeof(name)) == 0) {
            if ((hostinfo = gethostbyname(name)) != NULL) {
                ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
            }
        }
        return ip;
    }

protected:
    virtual LRESULT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        if (Msg == WM_SOCK_MSG) {
            switch (WSAGETSELECTEVENT(lParam)) {
//          case FD_WRITE:
//              OnWrite();
            case FD_READ:
                OnReceive(WSAGETSELECTERROR(lParam));
                break;
            case FD_CLOSE:
                OutPut("FD_CLOSE\n");
                OnClose(WSAGETSELECTERROR(lParam), e_close_by_net);
                break;
            case FD_CONNECT:
                OnConnect(WSAGETSELECTERROR(lParam));
                break;
//          case FD_ACCEPT:
//              OnAccept( WSAGETSELECTERROR( lParam ) );
            }
            return 0;
        } else if (Msg == WM_TIMER) {
            if (wParam == 2332) {
                m_lTimeCount++;
                if (m_state == e_connecting) {
                    if (m_lTimeCount >= 10) {  // 10 seconds
                        OnConnect(WSAETIMEDOUT);
                    }
                }
            }

            OnTimer((UINT_PTR)wParam);
        }

        return 0;
    }

    virtual void OnConnect(long nErrorCode)
    {
        if (nErrorCode != 0) {
            Close();
            ErrMsg(nErrorCode);
            m_state = e_close_by_net_error;
        } else
            m_state = e_connected;
    }

    virtual void OnReceive(long nErrorCode)
    {
        if (m_state != e_connected)  // Socket关闭后,缓冲区的数据还会到这里
            return;

        if (nErrorCode != 0) {
            OnClose(nErrorCode, e_close_by_net_error);
            return;
        }

        long lSize = Receive(((char *)m_sBuffer) + m_lBufUsed, 64 * 1024 - m_lBufUsed - 1);  // 最后一个字节要空出来
        if (lSize < 0) {
            OnClose(nErrorCode, e_close_by_net_error);
            return;
        }

        m_lTimeCount = 0;
        m_lBufUsed += lSize;

        unsigned char c = m_sBuffer[ m_lBufUsed ];
        m_sBuffer[ m_lBufUsed ] = 0;
        ::OutputDebugString((char *)m_sBuffer + m_lBufUsed - lSize);
        m_sBuffer[ m_lBufUsed ] = c;

        long lTake = 0;
        while (m_lBufUsed > 0 && 0 < (lTake = ((TParser *)this)->Take((char *)m_sBuffer, m_lBufUsed))) {
            if (m_state != e_connected) // 以防止在Answer中关闭连接
                return;
            memcpy(m_sBuffer, ((char *)m_sBuffer) + lTake, m_lBufUsed - lTake);
            m_lBufUsed -= lTake;
        }
        if (lTake < 0)
            OnClose(0, e_close_by_error_data);
    }

    virtual void OnTimer(UINT_PTR nIDEvent)
    {
        if (nIDEvent == 2332) {
            m_lTimeCount++;
            if (m_state == e_connecting) {
                if (m_lTimeCount >= 10) { // 连接超时10秒
                    OnConnect(WSAETIMEDOUT);
                }
            }
        }
    }
    virtual void OnClose(long nErrorCode, enumState eState)
    {
        Close();
        ErrMsg(nErrorCode);
        m_state = eState;
    }

public:
    bool Attach(SOCKET hSocket) //socket和窗口绑定
    {
        if (hSocket == INVALID_SOCKET)
            return false;

        assert(m_state != e_connecting && m_state != e_connected);
        if (!::IsWindow(GetWnd())) {
            if (!Create())
                return false;
        }
        //m_hWnd = hWnd;
        m_socket = hSocket;
        if (SOCKET_ERROR == WSAAsyncSelect(m_socket, GetWnd(), WM_SOCK_MSG, /*FD_WRITE | */FD_READ | FD_CLOSE | FD_CONNECT))
            //if ( SOCKET_ERROR == WSAAsyncSelect( m_socket, m_hWnd, WM_SOCK_MSG, /*FD_WRITE | */FD_READ | FD_CLOSE | FD_CONNECT ) )
        {
            ErrMsg(WSAGetLastError());
            Close();
            return false;
        }

        m_state = e_connected;
        return true;
    }

    bool Connect(const char *sIP, long lPort)
    {
        assert(m_state != e_connecting && m_state != e_connected);
        if (!::IsWindow(GetWnd())) {
            if (!Create())
                return false;
        }

        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket == INVALID_SOCKET) {
            ErrMsg(WSAGetLastError());
            Close();
            return false;
        }

        if (SOCKET_ERROR == WSAAsyncSelect(m_socket, GetWnd(), WM_SOCK_MSG, /*FD_WRITE | */FD_READ | FD_CLOSE | FD_CONNECT)) {
            ErrMsg(WSAGetLastError());
            Close();
            return false;
        }
        sockaddr_in saHost = {0};
        saHost.sin_family = AF_INET;
        if (SOCKET_ERROR == bind(m_socket, (sockaddr *)&saHost, sizeof(saHost))) {
            ErrMsg(WSAGetLastError());
            Close();
            return false;
        }
        sockaddr_in sinDest = {0};
        sinDest.sin_family = AF_INET;
        sinDest.sin_port = htons((unsigned short)lPort);
        sinDest.sin_addr.s_addr = inet_addr(sIP);
        if (SOCKET_ERROR == connect(m_socket, (SOCKADDR *)&sinDest, sizeof(sinDest))) {
            int nError = WSAGetLastError();
            if (nError != 10035) {
                ErrMsg(WSAGetLastError());
                Close();
                return false;
            }
        }
        m_state = e_connecting;
        m_sIP = sIP;
        m_lPort = lPort;
        return true;
    }

    virtual bool Close()
    {
        if (m_state != e_connecting && m_state != e_connected)
            return false;

        if (m_hWnd) {
            ::DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        m_state = e_close_by_user;
        m_lBufUsed = 0;

        return true;
    }

    long Receive(char *sBuf, long lBufLen)
    {
        assert(m_state == e_connected);
        assert(lBufLen > 0);
        long r = recv(m_socket, (char *)sBuf, lBufLen, 0);
        if (r == 0) { // msdn; if the connection has been gracefully closed, the return value is zero            
            this->OnClose(0, e_close_by_net);
        } else if (r <= SOCKET_ERROR) {
            long nLastError = WSAGetLastError();
            if (nLastError == WSAEWOULDBLOCK)
                return 0;

            OnClose(WSAGetLastError(), e_close_by_net_error);
        }
        return r;
    }

    long Send(const char *pData, DWORD dwSize)
    {
        if (m_state != e_connected)
            return -1;

        ::OutputDebugString(pData);
        long r = send(m_socket, (LPCTSTR)pData, dwSize, 0);
        if (r == SOCKET_ERROR) {
            long nLastError = WSAGetLastError();
            if (nLastError == WSAEWOULDBLOCK)
                return 0;  // need to send data again

            ErrMsg(nLastError);
            OnClose(nLastError, e_close_by_net_error);
            return -1;
        }

        // the cache not longer enough, close it
        if (r < (long)dwSize) {
            this->OutPut("$$$ 网速过慢,TCPLink关闭了这个连接!");
            this->Close();
            return -1;
        }

        return r;
    }
};

class CTCPListen
{
    SOCKET m_socket;
    HWND m_hWnd;
    long m_PrevWndProc;
    char *m_sOutBuffer;

public:
    CTCPListen(): m_socket(INVALID_SOCKET), m_hWnd(NULL), m_PrevWndProc(NULL)
    {
        m_sOutBuffer = (char *)malloc(64 * 1024);
    }
    ~CTCPListen()
    {
        if (m_hWnd) {
            ::DestroyWindow(m_hWnd);
            m_hWnd = NULL;
        }
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        free(m_sOutBuffer);
    }
    bool Listen(long lPort)
    {
        assert(m_socket == INVALID_SOCKET);
        if (!::IsWindow(GetWnd())) {
            if (!Create())
                return false;
        }

        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket == INVALID_SOCKET) {
            m_socket = NULL;
            long nError = WSAGetLastError();
            return false;
        }
        sockaddr_in saHost;
        ZeroMemory(&saHost, sizeof(saHost));
        saHost.sin_family = AF_INET;
        saHost.sin_port = htons((WORD)lPort);

        if (bind(m_socket, (sockaddr *)&saHost, sizeof(saHost)) == SOCKET_ERROR)
            return false;
        if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
            return false;

        return WSAAsyncSelect(m_socket, GetWnd(), WM_SOCK_MSG, FD_ACCEPT) != SOCKET_ERROR;
    }
    HWND GetWnd()
    {
        return m_hWnd;
    }
    HWND Create()
    {
        static bool bReg = false;
        if (!bReg) {
            WNDCLASS wndclass = {0};
            wndclass.lpfnWndProc = DefWindowProc;
            wndclass.hInstance = GetModuleHandle(NULL);
            wndclass.lpszClassName = "tcplink_window_class_name12";
            RegisterClass(&wndclass);
            bReg = true;
        }

        m_hWnd = ::CreateWindow("tcplink_window_class_name12",
                                "tcplink_window_class_name", 0, 0, 0, 0, 0,
                                NULL, NULL, GetModuleHandle(NULL), NULL);
        if (m_hWnd) {
            SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
            m_PrevWndProc = (long)SetWindowLong(m_hWnd, GWL_WNDPROC, (long)WindowProc_s);
        };

        SetTimer(m_hWnd, 2332, 1000, NULL);
        return m_hWnd;
    }

private:
    static LRESULT WindowProc_s(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        CTCPListen *s = (CTCPListen *)GetWindowLong(hWnd, GWL_USERDATA);
        if (s) {
            s->WindowProc(Msg, wParam, lParam);
            if (Msg == WM_DESTROY) {
                s->OutPut("WM_DESTROY\n");
                SetWindowLong(s->m_hWnd, GWL_USERDATA, NULL);
                SetWindowLong(s->m_hWnd, GWL_WNDPROC, (LONG)s->m_PrevWndProc);
                s->m_hWnd = NULL;
            }
        }

        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

protected:
    virtual LRESULT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        if (Msg == WM_DESTROY) {
            Close();
        } else if (Msg == WM_SOCK_MSG) {
            switch (WSAGETSELECTEVENT(lParam)) {
//          case FD_WRITE:
//              OnWrite();
//          case FD_READ:
//              OnReceive( WSAGETSELECTERROR( lParam ) );
//              break;
            case FD_CLOSE:
                OutPut("FD_CLOSE\n");
                OnClose(WSAGETSELECTERROR(lParam));
                break;
//          case FD_CONNECT:
//              OnConnect( WSAGETSELECTERROR( lParam ) );
//              break;
            case FD_ACCEPT:
                OnAccept(WSAGETSELECTERROR(lParam));
            }
            return 0;
        } else if (Msg == WM_TIMER) {
        }

        return 0;
    }

    virtual void OnClose(long nErrorCode)
    {
        Close();
        ErrMsg(nErrorCode);
    }

    virtual bool Close()
    {
        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            return true;
        }
        return false;
    }

    virtual SOCKET OnAccept(long nErrorCode)
    {
        sockaddr sockAddrIn = {0};
        int nSockAddrLen = sizeof(sockAddrIn);
        SOCKET socket = accept(m_socket, &sockAddrIn, &nSockAddrLen);
        if (socket == INVALID_SOCKET) {
            nErrorCode = WSAGetLastError();
            return NULL;
        }

        return socket;
    }

    void ErrMsg(long nErrorCode)
    {
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
        OutPut("$$$ Socket error: %d, %s\n", nErrorCode, lpMsgBuf);
        LocalFree(lpMsgBuf);
    }

    virtual void OutPut(const char *sFormat, ...)
    {
        va_list args;
        va_start(args, sFormat);
#if _MSC_VER > 1300
        vsprintf_s(m_sOutBuffer, 1024 * 64, sFormat, args);
#else
        vsprintf(m_sOutBuffer, sFormat, args);
#endif
        memset(m_sOutBuffer, 0, 1024 * 64);
        ::OutputDebugString(m_sOutBuffer);
    }
    virtual void OnTimer()
    {
    }
};

template< class TLink >
class CTCPListenT : public CTCPListen
{
public:
    std::vector< TLink * > m_vectorLink;

    CTCPListenT() {}
    ~CTCPListenT()
    {
        std::vector< TLink * >::iterator pos;
        for (pos = m_vectorLink.begin(); pos != m_vectorLink.end(); pos++) {
            (*pos)->Close();
            delete *pos;
        }
        m_vectorLink.clear();
    }

    virtual SOCKET OnAccept(long nErrorCode)
    {
        SOCKET socket = CTCPListen::OnAccept(nErrorCode);
        if (socket != INVALID_SOCKET) {
            TLink *pLink = new TLink;
            OnNewLink(pLink, socket);
            m_vectorLink.push_back(pLink);
        }
        return socket;
    }

    virtual void OnNewLink(TLink *pLink, SOCKET socket)
    {
        pLink->Attach(socket);
    }
    virtual void OnDeleteLink(TLink *pLink)
    {

    }
    void OnTimer()
    {
        bool bFound = true;
        while (bFound) {
            bFound = false;
            std::vector< TLink * >::iterator pos;
            for (pos = m_vectorLink.begin(); pos != m_vectorLink.end(); pos++) {
                if ((*pos)->m_state > 2) {
                    OnDeleteLink(*pos);
                    delete *pos;
                    m_vectorLink.erase(pos);
                    bFound = true;
                }
            }
        }
    }
};

template< class TParser >
class CUDPStreamT
{
    long m_PrevWndProc;
    long m_lPort;
    HWND m_hWnd;

private:
    HWND WndCreate()
    {
        static bool bReg = false;
        if (!bReg) {
            WNDCLASS wndclass = {0};
            wndclass.lpfnWndProc = DefWindowProc;
            wndclass.hInstance = GetModuleHandle(NULL);
            wndclass.lpszClassName = "udpstream_window_class_name";
            RegisterClass(&wndclass);
            bReg = true;
        }

        m_hWnd = ::CreateWindow("udpstream_window_class_name",
                                "udpstream_window_class_name", 0, 0, 0, 0, 0,
                                NULL, NULL, GetModuleHandle(NULL), NULL);
        if (m_hWnd) {
            SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);
            m_PrevWndProc = (long)SetWindowLong(m_hWnd, GWL_WNDPROC, (long)WindowProc_s);
        };

        return m_hWnd;
    }
    static LRESULT WindowProc_s(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        CUDPStreamT<TParser> *s = (CUDPStreamT<TParser> *)GetWindowLong(hWnd, GWL_USERDATA);
        if (s) {
            s->WindowProc(Msg, wParam, lParam);
            if (Msg == WM_DESTROY) {
                SetWindowLong(s->m_hWnd, GWL_USERDATA, NULL);
                SetWindowLong(s->m_hWnd, GWL_WNDPROC, (LONG)s->m_PrevWndProc);
                s->m_hWnd = NULL;
            }
        }

        return DefWindowProc(hWnd, Msg, wParam, lParam);
    }

    virtual LRESULT WindowProc(UINT Msg, WPARAM wParam, LPARAM lParam)
    {
        if (Msg == WM_DESTROY) {
            Close();
        } else if (Msg == WM_SOCK_MSG) {
            switch (WSAGETSELECTEVENT(lParam)) {
//          case FD_WRITE:
//              OnWrite();
            case FD_READ:
                OnReceive(WSAGETSELECTERROR(lParam));
                break;
            //case FD_CLOSE:
            //    OutPut( "FD_CLOSE\n" );
            //    OnClose( WSAGETSELECTERROR( lParam ) );
            //    break;
            //case FD_CONNECT:
            //    OnConnect( WSAGETSELECTERROR( lParam ) );
            //    break;
            //case FD_ACCEPT:
            //    OnAccept( WSAGETSELECTERROR( lParam ) );
            }
            return 0;
        } else if (Msg == WM_TIMER) {
        }

        return 0;
    }

protected:
    SOCKET m_socket;

    void ErrMsg(int nErrorCode)
    {
        LPVOID lpMsgBuf;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                      FORMAT_MESSAGE_IGNORE_INSERTS, NULL, nErrorCode,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

        char sBuffer[1024] = {0};
        sprintf_s(sBuffer, 1023, "$$$ Socket error: %d, %s\n", nErrorCode, lpMsgBuf);
        ::OutputDebugString(sBuffer);
        LocalFree(lpMsgBuf);
    }

    virtual void OnReceive(long nErrorCode)
    {
        char *sBuffer = new char[ 2048 ];
        sockaddr_in sockaddr = {0};
        int len = sizeof(sockaddr);
        long lRecved = recvfrom(m_socket, sBuffer, 2048, 0, (struct sockaddr *)&sockaddr, &len);
        ((TParser *)this)->Take((char *)sBuffer, lRecved);
    }

public:
    CUDPStreamT() : m_socket(INVALID_SOCKET)
    {
    }
    ~CUDPStreamT()
    {
        if (m_socket != INVALID_SOCKET)
            Close();
    }
    bool Close()
    {
        ::DestroyWindow(m_hWnd);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return true;
    }

    long GetPort()
    {
        sockaddr_in local = {0};
        int len = sizeof(local);
        getsockname(m_socket, (struct sockaddr *)&local, &len);
        return ntohs(local.sin_port);
    }

    bool Create(long lPort = 0)
    {
        m_lPort = lPort;
        if (!::IsWindow(m_hWnd))
            WndCreate();

        m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (m_socket == INVALID_SOCKET) {
            ErrMsg(WSAGetLastError());
            return false;
        }

        if (lPort != 0) {
            struct sockaddr_in local;
            local.sin_family = AF_INET;
            local.sin_port = htons((short)lPort);
            local.sin_addr.s_addr = INADDR_ANY;
            if (0 != bind(m_socket, (struct sockaddr *)&local, sizeof(local))) {
                ErrMsg(WSAGetLastError());
                Close();
                return false;
            }
        }

        if (SOCKET_ERROR == WSAAsyncSelect(m_socket, m_hWnd, WM_SOCK_MSG, FD_READ | FD_CLOSE | FD_CONNECT)) {
            ErrMsg(WSAGetLastError());
            Close();
            return false;
        }

        return true;
    }

    long Send(const char *sIP, long lPort, const char *pData, DWORD dwSize)
    {
        int len = sizeof(m_sockaddr);
        return sendto(m_socket, pData, dwSize, 0, (struct sockaddr *)&m_sockaddr, len);
    }
};
