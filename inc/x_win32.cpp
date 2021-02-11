#include "x_win32.h"
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <lm.h>    // lan manage
#include <winnetwk.h>   // 局域网资源, WNetEnum...

#pragma comment(lib, "Netapi32.lib")  // NetUser、NetGroup
#pragma comment(lib, "Advapi32.lib")  // SCManager
#pragma comment(lib, "mpr.lib")  // Multiple Provider Router

#pragma warning(disable: 4018 4996 4477)

//////////////////////////////////////////////////////////////////////////
// szServerName:\\.开头, 为NULL表示为本机添加用户
int UserManager::AddUser(wchar_t *szUserName, wchar_t *szPassword, wchar_t *szServerName/*=NULL*/)
{
    USER_INFO_1 ui; //_1与NetUserAdd第二个参数相对应
    DWORD dwError = 0;
    NET_API_STATUS nStatus;
    // 填充 USER_INFO_1
    ui.usri1_name = szUserName; // 用户名
    ui.usri1_password = szPassword; // 密码
    ui.usri1_priv = USER_PRIV_USER; // privilege
    ui.usri1_home_dir = NULL;
    ui.usri1_comment = NULL;
    ui.usri1_flags = UF_SCRIPT;
    ui.usri1_script_path = NULL;

    nStatus = NetUserAdd(szServerName, 1, (LPBYTE)&ui, &dwError);
    if (nStatus == NERR_Success) {
        wprintf(L"User %s has been successfully added on %s\n", szUserName, szServerName);
    } else {
        wprintf(L"A system error has occurred: %d\n", nStatus);
    }
    return 0;
}

int UserManager::DelUser(wchar_t *szUserName, wchar_t *szServerName/*=NULL*/)
{
    NET_API_STATUS nStatus = NetUserDel(szServerName, szUserName);
    if (nStatus == NERR_Success)
        wprintf(L"User %s has been successfully deleted on %s\n", szUserName, szServerName);
    else
        wprintf(L"A system error has occurred: %d\n", nStatus);

    return 0;
}

int UserManager::AddUserToGroup(wchar_t *szUserName, wchar_t *szGroup, wchar_t *szServerName/*=NULL*/)
{
    NET_API_STATUS nStatus;
    nStatus =  NetGroupAddUser(szServerName, szGroup, szUserName);
    if (nStatus == NERR_Success)
        wprintf(L"User %s has been successfully added on %s\n", szUserName, szServerName);
    else
        wprintf(L"NetGroupAddUser A system error has occurred: %d\n", nStatus);

    return 0;
}

int UserManager::ListUsers(wchar_t *pszServerName/*=NULL*/)
{
    LPUSER_INFO_0 pBuf = NULL;
    LPUSER_INFO_0 pTmpBuf;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD dwResumeHandle = 0;
    DWORD i;
    DWORD dwTotalCount = 0;
    NET_API_STATUS nStatus;

    do {
        nStatus = NetUserEnum(pszServerName,
                              0, // 这里设置为0,使用 LPUSER_INFO_0 返回结果
                              FILTER_NORMAL_ACCOUNT, // 只列举“正常”类型的用户
                              (LPBYTE *)&pBuf, // OUT, 内存由API分配,调用该函数后需 NetApiBufferFree 释放
                              MAX_PREFERRED_LENGTH,
                              &dwEntriesRead,// 读了的 Entries
                              &dwTotalEntries,// 一共的 Entries
                              &dwResumeHandle); //第一次调用必须为0

        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
            if ((pTmpBuf = pBuf) != NULL) {
                for (i = 0; (i < dwEntriesRead); i++) {
                    if (pTmpBuf) {
                        wprintf(L"\t-- %s\n", pTmpBuf->usri0_name);
                    }
                    pTmpBuf++;
                    dwTotalCount++;
                }
            }
        } else {
            wprintf(L"A system error has occurred: %d\n", nStatus);
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }

    } while (nStatus == ERROR_MORE_DATA);

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    wprintf(L"Total of %d users\n\n", dwTotalCount);
    return dwTotalCount;
}

int UserManager::ListGroup(wchar_t *pszServerName/*=NULL*/)
{
    LPLOCALGROUP_INFO_0 pBuf = NULL;
    LPLOCALGROUP_INFO_0 pTmpBuf;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    PDWORD_PTR dwResumeHandle = 0;
    DWORD i;
    DWORD dwTotalCount = 0;
    NET_API_STATUS nStatus;

    do {
        nStatus = NetLocalGroupEnum(
                      pszServerName,
                      0,
                      (LPBYTE *)&pBuf,
                      MAX_PREFERRED_LENGTH,
                      &dwEntriesRead,
                      &dwTotalEntries,
                      dwResumeHandle);

        if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA)) {
            if ((pTmpBuf = pBuf) != NULL) {
                for (i = 0; (i < dwEntriesRead); i++) {
                    if (pTmpBuf) {
                        wprintf(L"\t-- %s\n", pTmpBuf->lgrpi0_name);
                        pTmpBuf++;
                        dwTotalCount++;
                    }
                }
            }
        } else {
            wprintf(L"A system error has occurred: %d\n", nStatus);
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }
    } while (nStatus == ERROR_MORE_DATA);

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }

    wprintf(L"Total of %d groups\n\n", dwTotalCount);
    return dwTotalCount;
}

int UserManager::GetUserInfo(wchar_t *pszUserName, wchar_t *pszServerName/*=NULL*/)
{
    DWORD dwLevel = 4;
    USER_INFO_2 *pBuf = NULL;
    NET_API_STATUS nStatus;
    nStatus = NetUserGetInfo(pszServerName, pszUserName, 2, (LPBYTE *)&pBuf);
    if (nStatus == NERR_Success) {
        if (pBuf != NULL) {
            wprintf(L"\tAccount:\t%s\n", pBuf->usri2_name);
            wprintf(L"\tusr_comment:\t%s\n", pBuf->usri2_usr_comment);
            wprintf(L"\tFull name:\t%s\n", pBuf->usri2_full_name);
            wprintf(L"\tpassword:\t%s\n", pBuf->usri2_password);
            wprintf(L"\tlast_logoff:\t%d\n", pBuf->usri2_last_logoff);
            //wprintf(L"\tlogon_hours:\t%d\n", pBuf->usri2_logon_hours);
            wprintf(L"\thome_dir:\t%s\n", pBuf->usri2_home_dir);
        }
    } else {
        wprintf(L"A system error has occurred: %d\n", nStatus);
    }

    if (pBuf != NULL) {
        NetApiBufferFree(pBuf);
    }
    return 0;
}

TCHAR *UserManager::get_current_username()
{
    unsigned long username_len = 257;
    GetUserName(m_username, &username_len);
    return m_username;
}

void UserManager::get_user_groups(TCHAR *username/*NULL*/)
{
    if (!username) username = m_username;
    LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
    unsigned long dwLevel = 0;
    unsigned long dwFlags = LG_INCLUDE_INDIRECT;
    unsigned long dwPrefMaxLen = MAX_PREFERRED_LENGTH;
    unsigned long dwEntriesRead = 0;
    unsigned long dwTotalEntries = 0;
    NET_API_STATUS nStatus;
    nStatus = NetUserGetLocalGroups(NULL, (LPCWSTR)username, dwLevel, dwFlags, (LPBYTE *)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries);
    if (nStatus == NERR_Success && pBuf != NULL) {
        for (unsigned long i = 0; i < dwEntriesRead; i++) {
            _tprintf(_T("%s belongs group: %s\n"), username, pBuf[i].lgrui0_name);
        }
    }
    if (pBuf != NULL) NetApiBufferFree(pBuf);
}

void ServiceManager::getServices(unsigned int dwServiceType/*=-1*/)
{
    if (!m_hSCManager) return;
    DWORD dwType = dwServiceType;
    if (dwServiceType == -1) dwType = SERVICE_WIN32;

    DWORD nRemainSize = 0;   //[OUT]如果lpServices过小的话,返回剩余需要的字节数
    DWORD nNumServices;      //[OUT]服务的个数
    DWORD nResumeHandle = 0; //[OUT]第一次必须为0,一般不使用
    #if _MSC_VER > 1300
    ENUM_SERVICE_STATUS_PROCESS *lpServices = (ENUM_SERVICE_STATUS_PROCESS *)LocalAlloc(LPTR, 30720);  // [OUT]服务的状态
    if (!EnumServicesStatusEx(m_hSCManager, SC_ENUM_PROCESS_INFO, dwType, SERVICE_STATE_ALL, (LPBYTE)lpServices, 30720, &nRemainSize, &nNumServices, &nResumeHandle, NULL)) {
        _tprintf(_T("### EnumServicesStatus errno:%d\n"), GetLastError());
        return;
    }
    #else
    ENUM_SERVICE_STATUS *lpServices = (ENUM_SERVICE_STATUS *)LocalAlloc(LPTR, 30720);  //[OUT]服务的状态
    if (!EnumServicesStatus(m_hSCManager, dwType, SERVICE_STATE_ALL, lpServices, 30720, &nRemainSize, &nNumServices, &nResumeHandle)) {
        _tprintf(_T("### EnumServicesStatus errno:%d\n"), GetLastError());
        return;
    }
    #endif

    _tprintf(_T("------------------------------------------------------\n"));
    _tprintf(_T("PID \t ServiceName {Service Display Name} <Current status>\n"));
    _tprintf(_T("------------------------------------------------------\n"));
    for (int n=0; n<nNumServices; n++) {
        #if _MSC_VER > 1300
        SERVICE_STATUS_PROCESS ss = lpServices[n].ServiceStatusProcess;
        #else
        SERVICE_STATUS ss = lpServices[n].ServiceStatus;
        #endif
        if (!(dwServiceType == -1 && ss.dwCurrentState != SERVICE_RUNNING)) {
            #if _MSC_VER > 1300
            _tprintf(_T("pid:%d."), ss.dwProcessId); if (ss.dwProcessId == 0) _tprintf(_T("\t"));
            #else
            _tprintf(_T("\t"));
            #endif
            _tprintf(_T("%s\t\t{%-20.20s}\t"), lpServices[n].lpServiceName, lpServices[n].lpDisplayName);

            //Show the status of service
            switch (ss.dwCurrentState) {
            case SERVICE_STOPPED:
                _tprintf(_T("<not running.>"));
                break;
            case SERVICE_START_PENDING:
                _tprintf(_T("<starting.>"));
                break;
            case SERVICE_STOP_PENDING:
                _tprintf(_T("<stopping.>"));
                break;
            case SERVICE_RUNNING:
                _tprintf(_T("<running.>"));
                break;
            case SERVICE_CONTINUE_PENDING:
                _tprintf(_T("<continue pending.>"));
                break;
            case SERVICE_PAUSE_PENDING:
                _tprintf(_T("<pause pending.>"));
                break;
            case SERVICE_PAUSED:
                _tprintf(_T("<paused.>"));
                break;
            }
            _tprintf(_T("\n"));
        }
    }

    _tprintf(_T("\n"));
    if (lpServices) LocalFree(lpServices);
}

void ServiceManager::getServiceStatus(TCHAR *lpServiceName)
{
    if (!m_hSCManager) return;

    SERVICE_STATUS *lpStatus = (SERVICE_STATUS *)LocalAlloc(LPTR, 1024);
    if (!lpStatus) return;
    QUERY_SERVICE_CONFIG *lpConfig = (QUERY_SERVICE_CONFIG *)LocalAlloc(LPTR, 1024);
    if (!lpConfig) return;
    SERVICE_DESCRIPTION *lpDescription = (SERVICE_DESCRIPTION *)LocalAlloc(LPTR, 1024);
    if (!lpDescription) return;
    SERVICE_FAILURE_ACTIONS *lpAction = (SERVICE_FAILURE_ACTIONS *)LocalAlloc(LPTR, 2048);
    if (!lpAction) return;

    SC_HANDLE hService = OpenService(m_hSCManager, lpServiceName, SERVICE_ALL_ACCESS);
    if (!hService) {
        _tprintf(_T("###OpenService error!\n"));
        return;
    }

    if (!QueryServiceStatus(hService, lpStatus)) {
        CloseServiceHandle(hService);
        _tprintf(_T("###QueryServiceStatus failed!\n"));
        return;
    }
    switch (lpStatus->dwCurrentState) {
    case SERVICE_STOPPED:
        _tprintf(_T("<not running.>"));
        break;
    case SERVICE_START_PENDING:
        _tprintf(_T("<starting.>"));
        break;
    case SERVICE_STOP_PENDING:
        _tprintf(_T("<stopping.>"));
        break;
    case SERVICE_RUNNING:
        _tprintf(_T("<running.>"));
        break;
    case SERVICE_CONTINUE_PENDING:
        _tprintf(_T("<continue pending.>"));
        break;
    case SERVICE_PAUSE_PENDING:
        _tprintf(_T("<pause pending.>"));
        break;
    case SERVICE_PAUSED:
        _tprintf(_T("<paused.>"));
        break;
    }

    switch (lpStatus->dwServiceType) {
    case SERVICE_FILE_SYSTEM_DRIVER:
        _tprintf(_T("The service is a file system driver."));
        break;
    case SERVICE_KERNEL_DRIVER:
        _tprintf(_T("The service is a kernel driver."));
        break;
    case SERVICE_WIN32_OWN_PROCESS:
        _tprintf(_T("The service runs in its own process."));
        break;
    case SERVICE_WIN32_SHARE_PROCESS:
        _tprintf(_T("The service shares a process with other services."));
        break;
    }

    DWORD dwByteNeeded = 0;
    if (!QueryServiceConfig(hService, lpConfig, 1024, &dwByteNeeded)) {
        CloseServiceHandle(hService);
        _tprintf(_T("###QueryServiceConfig failed!\n"));
        return;
    }
    _tprintf(_T("\nPath:%s"), lpConfig->lpBinaryPathName);
    _tprintf(_T("\nDependency:%s"), lpConfig->lpDependencies);
    _tprintf(_T("\nLoadOrderGroup:%s"), lpConfig->lpLoadOrderGroup);
    _tprintf(_T("\nStartType:\n"));
    switch (lpConfig->dwStartType) {
    case SERVICE_AUTO_START:
        _tprintf(_T("The service started automatically during system startup.\n"));
        break;
    case SERVICE_BOOT_START:
        _tprintf(_T("The device driver started by the system loader.\n"));
        break;
    case SERVICE_DEMAND_START:
        _tprintf(_T("The service started need demand by a process.\n"));
        break;
    case SERVICE_DISABLED:
        _tprintf(_T("The service that cannot be started.\n"));
        break;
    case SERVICE_SYSTEM_START:
        _tprintf(_T("The device driver started by the IoInitSystem function.\n"));
        break;
    }

    if (!QueryServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, (LPBYTE)lpDescription, 1024, &dwByteNeeded)) {
        CloseServiceHandle(hService);
        _tprintf(_T("###QueryServiceConfig2 failed!\n"));
        return;
    }
    _tprintf(_T("Description: %s\n"), lpDescription->lpDescription);

    if (!QueryServiceConfig2(hService, SERVICE_CONFIG_FAILURE_ACTIONS, (LPBYTE)lpAction, 2048, &dwByteNeeded)) {
        CloseServiceHandle(hService);
        _tprintf(_T("###QueryServiceConfig2 failed!\n"));
        return;
    }
    _tprintf(_T("ResetPeriod Time: %ds\n"), lpAction->dwResetPeriod);  //单位:秒,每隔此段时间久reset failure count为0
    _tprintf(_T("RebootMSG: %s\n"), lpAction->lpRebootMsg);  //Message to broadcast to server users before rebooting in response to the SC_ACTION_REBOOT service controller action.
    _tprintf(_T("Command: %s\n"), lpAction->lpCommand);   //Command line of the process for the CreateProcess function to execute in response to the SC_ACTION_RUN_COMMAND service controller action
    _tprintf(_T("Actions in the array: %d\n"), lpAction->cActions);   //Number of elements in the lpsaActions array.

    LocalFree(lpStatus);
    LocalFree(lpConfig);
    LocalFree(lpDescription);
    LocalFree(lpAction);
    CloseServiceHandle(hService);
}

/// 启动服务是耗时操作,调用该函数将会Sleep一定时间
bool ServiceManager::startService(TCHAR *lpServiceName)
{
    if (!m_hSCManager) return false;

    SC_HANDLE hService = OpenService(m_hSCManager, lpServiceName, SERVICE_ALL_ACCESS);
    if (!hService) {
        _tprintf(_T("###OpenService error!\n"));
        return false;
    }

    SERVICE_STATUS ssStatus;
    if (!QueryServiceStatus(hService, &ssStatus)) {
        CloseServiceHandle(hService);
        _tprintf(_T("### Query service status failed!\n"));
        return false;
    }
    if (ssStatus.dwCurrentState == SERVICE_RUNNING || ssStatus.dwCurrentState == SERVICE_START_PENDING) {
        CloseServiceHandle(hService);
        _tprintf(_T("\n$$$ Service:%s has being running or is being starting!\n"), lpServiceName);
        return false;
    }

    DWORD dwSize = 0;
    QUERY_SERVICE_CONFIG *lpConfig = (QUERY_SERVICE_CONFIG *)LocalAlloc(LPTR, 1024);
    if (!lpConfig) return false;
    if (!QueryServiceConfig(hService, lpConfig, 1024, &dwSize)) {
        CloseServiceHandle(hService);
        _tprintf(_T("### QueryServiceConfig failed!\n"));
        return false;
    }
    if (lpConfig->dwStartType == SERVICE_DISABLED) {
        _tprintf(_T("The service is disabled!\n"));
        _tprintf(_T("Try to change the service's start type...\n"));
        if (!ChangeServiceConfig(hService, SERVICE_NO_CHANGE, SERVICE_DEMAND_START, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
            CloseServiceHandle(hService);
            _tprintf(_T("### ChangeServiceConfig Failed!\n"));
            return false;
        }
        _tprintf(_T("--- ChangeServiceConfig OK!\n"));
    }

    if (!StartService(hService, 0, NULL)) {
        DWORD err = GetLastError();
        _tprintf(_T("###StartService failed: "));
        switch (err) {
        case ERROR_ACCESS_DENIED:
            _tprintf(_T("The specified handle was not opened with SERVICE_START access.\n"));
            break;
        case ERROR_INVALID_HANDLE:
            _tprintf(_T("The specified handle is invalid.\n"));
            break;
        case ERROR_PATH_NOT_FOUND:
            _tprintf(_T("The service binary file could not be found.\n"));
            break;
        case ERROR_SERVICE_ALREADY_RUNNING:
            _tprintf(_T("An instance of the service is already running\n"));
            break;
        case ERROR_SERVICE_DATABASE_LOCKED:
            _tprintf(_T("The database is locked.\n"));
            break;
        case ERROR_SERVICE_DEPENDENCY_DELETED:
            _tprintf(_T("The service depends on a service that does not exist or has been marked for deletion.\n"));
            break;
        case ERROR_SERVICE_DEPENDENCY_FAIL:
            _tprintf(_T("The service depends on another service that has failed to start.\n"));
            break;
        case ERROR_SERVICE_DISABLED:
            _tprintf(_T("The service has been disabled.\n"));
            break;
        case ERROR_SERVICE_LOGON_FAILED:
            _tprintf(_T("The service could not be logged on. This error occurs if the service was started from an account that does not have the Log on as a service right.\n"));
            break;
        case ERROR_SERVICE_MARKED_FOR_DELETE:
            _tprintf(_T("The service has been marked for deletion.\n"));
            break;
        case ERROR_SERVICE_NO_THREAD:
            _tprintf(_T("A thread could not be created for the service.\n"));
            break;
        case ERROR_SERVICE_REQUEST_TIMEOUT:
            _tprintf(_T("The process for the service was started, but it did not call StartServiceCtrlDispatcher, or the thread that called StartServiceCtrlDispatcher may be blocked in a control handler function.\n"));
            break;
        }
        CloseServiceHandle(hService);
        return false;
    }

    if (!(QueryServiceStatus(hService, &ssStatus))) {
        CloseServiceHandle(hService);
        _tprintf(_T("###QueryServiceStatus failed!\n"));
        return false;
    }

    DWORD dwWaitTime; //An estimate time, in milliseconds. the service expects a pending to start, stop, pause...
    DWORD dwOldCheckPoint = 0; //每次成功启动后该值都会改变,用来防止长启动中的重复启动
    DWORD dwStartTickCount;
    while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
        dwWaitTime = ssStatus.dwWaitHint / 10;
        if (dwWaitTime < 1000) dwWaitTime = 1000;
        else if (dwWaitTime > 10000) dwWaitTime = 10000;
        Sleep(dwWaitTime);

        // Check the status again.
        if (!QueryServiceStatus(hService, &ssStatus))
            break;

        if (ssStatus.dwCheckPoint > dwOldCheckPoint) { // The service is making progress.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                // No progress made within the wait hint
                break;
            }
        }
    }

    if (ssStatus.dwCurrentState != SERVICE_RUNNING) {
        return false;
    }

    _tprintf(_T("Service:%s has been successfully started!\n"), lpServiceName);
    return true;
}

bool ServiceManager::stopService(TCHAR *lpServiceName)
{
    if (!m_hSCManager) return false;

    SC_HANDLE hService = OpenService(m_hSCManager, lpServiceName, SERVICE_ALL_ACCESS);
    if (hService == NULL) {
        _tprintf(_T("###OpenService failed!\n"));
        return false;
    }

    SERVICE_STATUS ssStatus;
    if (!QueryServiceStatus(hService, &ssStatus)) {
        _tprintf(_T("Query service failed!\n"));
        CloseServiceHandle(hService);
        return false;
    }

    if (ssStatus.dwCurrentState == SERVICE_STOPPED || ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        CloseServiceHandle(hService);
        _tprintf(_T("Service:%s has been stopped!\n"), lpServiceName);
        return true;
    }

    _tprintf(_T("stopping the service <%s>...\n"), lpServiceName);
    if (!(ControlService(hService, SERVICE_CONTROL_STOP, &ssStatus))) {
        CloseServiceHandle(hService);
        _tprintf(_T("###ControlService failed!\n"));
        return false;
    }

    DWORD dwWaitTime = ssStatus.dwWaitHint / 10;
    if (dwWaitTime < 1000) dwWaitTime = 1000;
    else if (dwWaitTime > 10000) dwWaitTime = 10000;
    Sleep(dwWaitTime);

    if (!QueryServiceStatus(hService, &ssStatus)) {
        _tprintf(_T("###QueryServiceStatus failed!\n"));
        return false;
    }

    if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
        _tprintf(_T("%d\n"), ssStatus.dwCurrentState);
        _tprintf(_T("$$$ Service is not started! CurrentState:%d\n"), ssStatus.dwCurrentState);
        return false;
    }

    _tprintf(_T("Service is stopped successfully!\n"));
    return true;
}

bool ServiceManager::isServiceInstalled(TCHAR *lpServiceName)
{
    if (!m_hSCManager) return false;

    SC_HANDLE hService = OpenService(m_hSCManager, lpServiceName, SERVICE_QUERY_CONFIG);
    if (hService != NULL) {
        CloseServiceHandle(hService);
        return true;
    }
    return false;
}

bool ServiceManager::installService(TCHAR *lpServiceName, TCHAR *szFilePath)
{
    if (isServiceInstalled(lpServiceName)) return true;
    if (!m_hSCManager) return false;

    SC_HANDLE hService = CreateService(m_hSCManager, lpServiceName, lpServiceName, //服务名称,显示的服务名称
                                       SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, //SERVICE_ERROR_IGNORE:忽略错误处理
                                       szFilePath, NULL, NULL, NULL, NULL, NULL);
    if (hService == NULL) {
        CloseServiceHandle(hService);
        _tprintf(_T("###CreateService: %s failure!\n"), lpServiceName);
        return false;
    }
    CloseServiceHandle(hService);
    return true;
}

bool ServiceManager::UninstallService(TCHAR *lpServiceName)
{
    if (!m_hSCManager) return false;

    SC_HANDLE hService = OpenService(m_hSCManager, lpServiceName, SERVICE_STOP | DELETE);
    if (hService == NULL) {
        CloseServiceHandle(hService);
        _tprintf(_T("###OpenService: %s failure!\n"), lpServiceName);
        return false;
    }

    if (!stopService(lpServiceName)) {
        _tprintf(_T("###Can't stop the service:%s!\n"), lpServiceName);
    }

    if (!DeleteService(hService)) {
        _tprintf(_T("###DeleteService:%s failure!\n"), lpServiceName);
    }
    ::CloseServiceHandle(hService);

    return true;
}


Network::Network() : m_hNetResource(NULL)
{
    if (WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, NULL, &m_hNetResource) != NO_ERROR) {
        DumpError(GetLastError(), _T("WNetOpenEnum"));
        return;
    }
}

Network::~Network()
{
    if (m_hNetResource)
        WNetCloseEnum(m_hNetResource);
}

void Network::DumpError(unsigned long errcode, TCHAR *funname)
{
    unsigned long ret = errcode;
    _tprintf(_T("\n###error %s(): "), funname);
    if (ret != ERROR_EXTENDED_ERROR) {
        if (ret == ERROR_MORE_DATA) _tprintf(_T("More entries are available with subsequent calls.\n\n"));
        else if (ret == ERROR_INVALID_PARAMETER) _tprintf(_T("One or more parameters contain invalid values.\n\n"));
        else if (ret == ERROR_NO_NETWORK) _tprintf(_T("No network is present.\n\n"));
        else if (ret == ERROR_INVALID_HANDLE) _tprintf(_T("hEnum parameter is not valid.\n\n"));
        else if (ret == ERROR_NOT_CONTAINER) _tprintf(_T("Parameter does not point to a container.\n\n"));
        else if (ret == 5) _tprintf(_T("Access Denial.\n\n"));
        else _tprintf(_T("Unknown error!\n\n"));
    } else {
        TCHAR errinfo[32] = {0};
        TCHAR provider[32] = {0};
        WNetGetLastError(&ret, errinfo, 32, provider, 32);
        _tprintf(_T("%s report this error,errcode:%d,errinfo:%s\n\n"), provider, ret, errinfo);
    }
}

/**
 * DWORD WNetOpenEnum(DWORD dwScope,DWORD dwType,DWORD dwUsage,LPNETRESOURCE lpNetResource,LPHANDLE lphEnum)
 * @brief   枚举网络资源或者已经存在的连接
 * @param   lpNetResource:需要被枚举的资源,可以为NULL表示所有
 * @param   lphEnum:[out]枚举到的资源句柄,后续作为参数传入WNetEnumResource中以进入资源的细节
 * @return  ERROR_SUCCESS:成功, ERROR_NO_NETWORK:当前无网络, ERROR_NOT_CONTAINER:lpNetResource指针并未指向一个网络资源
 * @remark
    typedef struct _NETRESOURCE {
        DWORD dwScope; //网络枚举的范围: RESOURCE_CONNECTED(忽略dwUsage) RESOURCE_GLOBALNET RESOURCE_REMEMBERED(只枚举永久性连接)
        DWORD dwType;  //资源的类型: RESOURCETYPE_ANY RESOURCETYPE_DISK RESOURCETYPE_PRINT
        DWORD dwDisplayType; //影响网络对象如何在network browsing界面中显示: RESOURCEDISPLAYTYPE_DOMAIN RESOURCEDISPLAYTYPE_GENERIC..
        DWORD dwUsage; //资源的使用,只在dwScope=RESOURCE_GLOBALNET时有效: RESOURCEUSAGE_CONNECTABLE(只枚举那些能够连接的资源) RESOURCEUSAGE_CONTAINER
        LPTSTR lpLocalName;  //如果没有localname时,此值为NULL
        LPTSTR lpRemoteName; //remote network name
        LPTSTR lpComment;    //provider-supplied comment
        LPTSTR lpProvider;   //provider the owner of this resource
    } NETRESOURCE;
 */
bool Network::getNetResources(HANDLE hNetRes/*=NULL*/)
{
    if (!hNetRes && !m_hNetResource) return false;

    HANDLE hEnum = hNetRes == NULL ? m_hNetResource : hNetRes;
    DWORD dwSize    = 16384;
    DWORD dwCount   = -1; //资源的数目
    DWORD ret       = 0;
    TCHAR szDisplayType[32]  = {0};
    TCHAR szResourceType[32] = {0};

    NETRESOURCE *lpNET = (NETRESOURCE *)malloc(dwSize);
    if (!lpNET) return false;
    ret = WNetEnumResource(hEnum, &dwCount, (LPVOID)lpNET, &dwSize);
    if (ret != ERROR_SUCCESS) {
        DumpError(GetLastError(), _T("WNetEnumResource"));
        free(lpNET);
        return false;
    }

    for (int i = 0; i < dwCount; i++) {
        switch (lpNET[i].dwDisplayType) {
        case RESOURCEDISPLAYTYPE_DOMAIN:
            _tcscpy(szDisplayType, _T("DOMAIN: "));
            break;
        case RESOURCEDISPLAYTYPE_SERVER:
            _tcscpy(szDisplayType, _T("SERVER: "));
            break;
        case RESOURCEDISPLAYTYPE_SHARE:
            _tcscpy(szDisplayType, _T("SHARE: "));
            switch (lpNET[i].dwType) {
            case RESOURCETYPE_DISK:
                _tcscpy(szResourceType, _T("DISK: "));
                break;
            case RESOURCETYPE_PRINT:
                _tcscpy(szResourceType, _T("PRINTER: "));
                break;
            default:
                _tcscpy(szResourceType, _T("UNKNOWN: "));
                break;
            }
            break;
        case RESOURCEDISPLAYTYPE_GENERIC:
            _tcscpy(szDisplayType, _T("GENERIC: "));
            break;
        default:
            _tcscpy(szDisplayType, _T("UNKNOWN: "));
            break;
        }

        _tprintf(_T("DisplayType:%s\tResourceType:%s\tLocalName:%s\nRemoteName:%s\nComment:%s\tProvider:%s\n\n"),
                 szDisplayType, szResourceType, lpNET[i].lpLocalName, lpNET[i].lpRemoteName, lpNET[i].lpComment, lpNET[i].lpProvider);

        if (lpNET[i].dwUsage & RESOURCEUSAGE_CONTAINER) {
            HANDLE hContainer;
            ret = WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY, 0, &lpNET[i], &hContainer);
            if (ret == NO_ERROR) {
                getNetResources(hContainer);
                WNetCloseEnum(hContainer);
            } else
                DumpError(GetLastError(), _T("WNetOpenEnum"));
        }
    }

    free(lpNET);
    return true;
}
