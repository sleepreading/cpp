#ifndef __X_WIN32_H__
#define __X_WIN32_H__

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#include <tchar.h>
#ifndef _WINDOWS_
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
    #undef WIN32_LEAN_AND_MEAN
#endif

//////////////////////////////////////////////////////////////////////////
// User Account Manager
class UserManager
{
public:
    UserManager()
    {
        memset(m_username, 0, sizeof(m_username));
    };
    virtual ~UserManager() {};

    int AddUser(wchar_t *szUserName, wchar_t *szPassword, wchar_t *szServerName = NULL);
    int DelUser(wchar_t *szUserName, wchar_t *szServerName = NULL);
    int AddUserToGroup(wchar_t *szUserName, wchar_t *szGroup, wchar_t *szServerName = NULL);
    int ListUsers(wchar_t *pszServerName = NULL);
    int ListGroup(wchar_t *pszServerName = NULL);
    int GetUserInfo(wchar_t *pszUserName, wchar_t *pszServerName = NULL);
    TCHAR *get_current_username();
    void get_user_groups(TCHAR *username = NULL);
private:
    TCHAR m_username[256];
};

//////////////////////////////////////////////////////////////////////////
// Service Control Manager:SCManager
// 只能操作系统现有服务
class ServiceManager
{
public:
    ServiceManager() : m_hSCManager(NULL)
    {
        m_hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    };
    virtual ~ServiceManager()
    {
        if (m_hSCManager) CloseServiceHandle(m_hSCManager);
    };

    void getServices(unsigned int dwServiceType = -1);
    void getServiceStatus(TCHAR *lpServiceName);
    bool startService(TCHAR *lpServiceName); //有的服务是链式的依赖关系,后续需要增加这方面的处理
    bool stopService(TCHAR *lpServiceName);
    bool isServiceInstalled(TCHAR *lpServiceName);
    bool installService(TCHAR *lpServiceName, TCHAR *szFilePath); //注册SCD程序
    bool UninstallService(TCHAR *lpServiceName); //卸载SCD程序,下次开机时删除

private:
    SC_HANDLE   m_hSCManager;
};

// 局域网资源
class Network
{
public:
    Network();
    virtual ~Network();
    bool getNetResources(HANDLE hNetRes = NULL);

private:
    HANDLE  m_hNetResource;
    void DumpError(unsigned long errcode, TCHAR *funname);
};


#endif  // __X_WIN32_H__
