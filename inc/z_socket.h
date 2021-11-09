/**
 * @file      z_socket.h
 * @date      2014-7-30 17:06:33
 * @author    leizi
 * @copyright zhanglei. email:847088355@qq.com
 * @remark    socket functions
 */
#ifndef __Z_SOCKET_H__
#define __Z_SOCKET_H__

#include <errno.h>
#include <string.h>
#ifdef  _WIN32
    #include <winsock2.h>
#else    // linux
    #include <netinet/in.h> // includes sys/socket.h + bits/types.h
    #include <fcntl.h>
    #include <unistd.h>    // close
    #include <sys/types.h> // FD_ZERO..
    #include <netdb.h>     // gethostbyname
#endif   // _WIN32

#ifdef  _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
    #pragma warning(disable: 4996 4244 4018)
#elif __GNUC__ > 4 || __GNUC__==4 && __GNUC_MINOR__ > 2
    //#pragma GCC diagnostic ignored "-Wno-sign-compare"
#endif

void sock_close(int sock)
{
    if (sock == -1) return;
    #ifdef _WIN32
    closesocket(sock);  // timeout, connect failure!
    #else
    close(sock);
    #endif
}

/**
 * zrecv
 * @brief  :接收指定字节的数据
 * @param  :buf:接收数据; len:指定接收的字节数; bUdpFlag:使用UDP发送
 * @return :实际接收的字节数.注意:如果分几次接收,后面有一次对方关闭了连接,则本封装与原始不同,会返回成功接收的字节数!实际上,每次接收的字节数多数是有意义的,所以要么就成功要么就失败,应该维持与原始一致的行为!
 * @note   :因为协议要兼顾远端主机接收窗口大小,OS还要控制本地发送缓冲区的大小,所以:
            单独一次调用send()发送大数据,并不能保证与实际发送的数据相符,接收情况类似!
            注意:对于epoll的ET模式这种EAGAIN的处理方式还是有漏洞!ET模式应该一直去读,直到返回0或EAGAIN;或一直写直到EAGAIN!
 */
long tcp_recv(int sock, char *buf, long len, long timeout=0, bool etmode=false)
{
    if (sock == -1 || len == 0 || buf == NULL) return 0;
    if (timeout > 0) {
        #ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
        #else
        struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
        #endif
    }

    int nTmp = 0;
    long nTotal = 0;
    while (nTotal < len) {
        nTmp = recv(sock, buf + nTotal, len - nTotal, 0);  // nTmp stands for actually received bytes
        if (-1 == nTmp) { // for blocking socket, -1 means an error occurred
            if (errno == EAGAIN) { // for nonblocking socket, this error means you should try again!
                if (etmode) return nTotal;
                continue;
            } else if (EINTR == errno) {
                continue;
            }
            return -1;
        } else if (0 == nTmp) { // for blocking socket, this means remote close the connection
            return nTotal;
        }
        nTotal += nTmp;
    }

    return nTotal;
}
long udp_recv(int sock, char* buf, long len, long timeout = 0, sockaddr_in* addr_from = 0)
{
    if (sock == -1 || len == 0 || buf == NULL) return 0;
    if (timeout > 0) {
    #ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    #else
        struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
    #endif
    }

    int nTmp = 0;
    long nTotal = 0;

    struct sockaddr_in addrFrom = { 0 };
#ifdef  _WIN32
    int nLen = sizeof(addrFrom);
#else
    unsigned int nLen = sizeof(addrFrom);
#endif
    while (nTotal < len) {
        nTmp = recvfrom(sock, buf + nTotal, len - nTotal, 0, (struct sockaddr*) & addrFrom, &nLen);
        if (-1 == nTmp) {
            return -1;
        } else if (0 == nTmp) {
            return nTotal;
        }
        nTotal += nTmp;
    }
    if (addr_from != NULL)
        memcpy(addr_from, &addrFrom, sizeof(addrFrom));

    return nTotal;
}

/**
 * zsend
 * @brief  :发送指定字节的数据
 * @param  :buf:待发送数据的缓冲区; len:需要发送的字节数; szAddrTo:使用bUdpFlag时需要发送的地址,如"192.168.1.2"
 * @return :实际发送的字节数
 */
long sock_send(int sock, char *buf, long len, long timeout=0, bool use_udp=false, bool etmode=false, const char *addr_to=0)
{
    if (sock == -1 || len == 0) return 0;
    int flag = 0;
#ifndef _WIN32
    flag = MSG_NOSIGNAL;
#endif
    if (timeout > 0) {
#ifdef _WIN32
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
#else
        struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(struct timeval));
#endif
    }

    int nTmp = 0; // stands for actually sent bytes
    long nTotal = 0;
    if (!use_udp) { // TCP
        while (nTotal < len) {
            nTmp = send(sock, buf + nTotal, len - nTotal, flag);
            if (-1 == nTmp) { // for blocking socket, -1 means an error occurred
                if (errno == EAGAIN) { // for nonblocking socket, this error means you should try again!
                    if (etmode) return nTotal;
                    continue;
                } else if (EINTR == errno) {
                    continue;
                }
                return -1;
            }
            nTotal += nTmp;
        }
    } else {
        struct sockaddr_in addrTo = { 0 };
        addrTo.sin_family = AF_INET;
        int iEnabel = 1;
        struct hostent *pHostNameTo = 0;
        if (addr_to)
            pHostNameTo = gethostbyname(addr_to);
        if (pHostNameTo)
            memcpy(&addrTo.sin_addr.s_addr, pHostNameTo->h_addr, 4);
        else
            setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (const char *)&iEnabel, sizeof(iEnabel));

        int nLen = sizeof(sockaddr_in);
        while (nTotal < len) {
            nTmp = sendto(sock, buf + nTotal, len - nTotal, 0, (struct sockaddr *)&addrTo, nLen);
            if (-1 == nTmp)
                return -1;
            nTotal += nTmp;
        }
    }
    return nTotal;
}

bool sock_async(int sock, bool async=true)
{
#ifdef _WIN32
    unsigned long ul = async ? 1U : 0U;
    if (ioctlsocket(sock, FIONBIO, &ul)) {
        return false;
    }
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (async)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags)) {
        return false;
    }
#endif
    return true;
}

/**
 * zselect
 * @brief  :于指定的套接字上等待指定的时间,检测该段时间内有无想要的事件发生
 */
bool sock_select(int sock, int timeout=1000/*毫秒*/, bool recv_or_send=true/*监测是否可读*/)
{
    if (sock == -1) return false;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock, &fds);

    timeval tmv = {timeout / 1000, timeout % 1000 * 1000};
    int count = 0;
    if (recv_or_send)
        count = select(sock + 1, &fds, NULL, NULL, &tmv);
    else
        count = select(sock + 1, NULL, &fds, NULL, &tmv);

    return (count <= 0 || !FD_ISSET(sock, &fds)) ? false : true;
}

/**
 * zconnect
 * @brief  :于指定的时间连接到指定的 IP:Port.指定时间内未成功连接返回-1
 * @return :主动连接的套接字(非阻塞的)
 * @note   :如果timeout>=0则返回的socket自动被置为异步
            判定一个socket是否在线应该使用心跳机制或者ping,不应该Connect检测!
 */
int sock_connect(const char *ip_or_hostname, unsigned short port, long timeout=-1)
{
    int sockConn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockConn == -1 || !ip_or_hostname || *ip_or_hostname == 0) return -1;

    struct sockaddr_in addrSrv = { 0 };
    addrSrv.sin_family = AF_INET;
    addrSrv.sin_port = htons(port);
    struct hostent *pHostNameTo = gethostbyname(ip_or_hostname);
    if (pHostNameTo)
        memcpy(&addrSrv.sin_addr.s_addr, pHostNameTo->h_addr, 4);
    int iLen = sizeof(addrSrv);

    if (timeout < 0) {
        return connect(sockConn, (struct sockaddr *)&addrSrv, iLen) == 0 ? sockConn : -1;
    } else {
        if (!sock_async(sockConn)) return -1;
        if (-1 == connect(sockConn, (struct sockaddr *)&addrSrv, iLen)) {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(sockConn, &fds);
            long sec = timeout / 1000;
            long usec = timeout % 1000 * 1000;
            timeval tm = { sec, usec };
            // 这个地方要注意了,select第1个参数如果为0,在linux上会造成timeout!!
            if (select(sockConn + 1, NULL, &fds, NULL, &tm) > 0) {
                int err = -1;
#ifdef  _WIN32
                int len = sizeof(int);
#else
                unsigned int len = sizeof(int);
#endif
                if (getsockopt(sockConn, SOL_SOCKET, SO_ERROR, (char *)&err, &len) < 0) {
                    return -3;
                }
                if (err == 0) {
                    sock_async(sockConn, false);
                    return sockConn;
                } else {
                    return -4;
                }
            }
            sock_close(sockConn);  // timeout, connect failure!
            return -2;
        }
    }

    return sockConn;  // LAN(some host), connect OK immediately
}

/**
 * zlisten
 * @brief  :在指定的端口使用TCP/UDP(只绑定)监听
 * @return :监听的套接字, -1:error,InvalidSocket!
 */
int sock_bind(unsigned short port, bool use_udp=false, bool listen_broadcast=true)
{
    sockaddr_in addr_any = { 0 };
    int sock_listen = -1, sock_flag = 1;

    if (!use_udp) {
        sock_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        addr_any.sin_family = AF_INET;
        addr_any.sin_port = htons(port);
        setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_flag, sizeof(int));
        if (!bind(sock_listen, (struct sockaddr *)&addr_any, sizeof(addr_any)) && !listen(sock_listen, 5)) {
            return sock_listen;
        }
    } else {
        sock_listen = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        addr_any.sin_family = AF_INET;
        addr_any.sin_port = htons(port);
        if (listen_broadcast) setsockopt(sock_listen, SOL_SOCKET, SO_BROADCAST, (char *)&sock_flag, sizeof(int));
        setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, (char *)&sock_flag, sizeof(int));
        setsockopt(sock_listen, SOL_SOCKET, SO_OOBINLINE, (char *)&sock_flag, sizeof(int));
        if (bind(sock_listen, (struct sockaddr *)&addr_any, sizeof(addr_any)) != -1)
            return sock_listen;
    }
    return -1;
}

/**
 * zaccept
 * @brief  :接收连入的套接字
 * @return :新的通讯套接字
 * @note   :注意:如果客户端在连接后尚未建立时就RST终止连接,则accept会一直阻塞下去,直到某个新的客户建立新的链接为止.解决办法是使用异步套接字监听!
 */
int sock_accept(int sock, int timeout=1000, sockaddr_in *addr_from=0)
{
    int sockAccept = -1;
    if (sock_select(sock, timeout)) {
#ifdef  _WIN32
        int nAddrSize = sizeof(sockaddr_in);
#else
        unsigned int nAddrSize = sizeof(sockaddr_in);
#endif
        sockaddr_in addrFrom = { 0 };
        sockAccept = accept(sock, (struct sockaddr *)&addrFrom, &nAddrSize);
        if (addr_from)
            memcpy(addr_from, &addrFrom, nAddrSize);
    }
    return sockAccept;
}

#ifdef  _MSC_VER
    #pragma warning(default: 4996 4244 4018)
#endif

#endif // __ZSOCKET_H__
