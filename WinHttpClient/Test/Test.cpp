// Test.cpp : Defines the entry point for the console application.
//

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
#endif

#include <stdio.h>
#include <tchar.h>
#include <fstream>
#include <tchar.h>
#include "xWinHttpClient.h"
#include "StringSpliter.h"
using std::ifstream;

#if defined(_UNICODE)||defined(UNICODE)
#define tstring wstring
#else
#define tstring string
#endif


// Progress - finished percentage.
bool ProgressProc(double progress)
{
    _tprintf(_T("Current progress: %-.1f%%\r\n"), progress);
    return true;
}

void UserAgentTest(void)
{
    WinHttpClient client(_T("http://www.codeproject.com/"));

    // Set the user agent to the same as IE8.
    client.SetUserAgent(_T("Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.2; CIBA; MS-RTC LM 8)"));

    client.SendHttpRequest();
    tstring httpResponseHeader = client.GetResponseHeader();
    tstring httpResponseContent = client.GetResponseContent();
}

void ProxyTest(void)
{
    WinHttpClient client(_T("http://www.codeproject.com/"));

    // Set the proxy to 192.168.0.1 with port 8080.
    client.SetProxy(_T("192.168.0.1:8080"));

    client.SendHttpRequest();
    tstring httpResponseHeader = client.GetResponseHeader();
    tstring httpResponseContent = client.GetResponseContent();
}

void CookiesTest(void)
{
    WinHttpClient client(_T("http://www.codeproject.com/"));

    // Set the cookies to send.
    client.SetAdditionalRequestCookies(_T("username=jack"));

    client.SendHttpRequest();

    // Get the response cookies.
    tstring httpResponseCookies = client.GetResponseCookies();
	_tprintf(_T("%s"),httpResponseCookies.c_str());
    tstring httpResponseHeader = client.GetResponseHeader();
	_tprintf(_T("%s"),httpResponseHeader.c_str());
    tstring httpResponseContent = client.GetResponseContent();
//	wcout<<httpResponseContent<<endl;
}

void HttpsTest(void)
{
    WinHttpClient client(_T("https://www.google.com/"));

    // Accept any certificate while performing HTTPS request.
    client.SetRequireValidSslCertificates(false);

    // Get the response cookies.
    tstring httpResponseCookies = client.GetResponseCookies();
	_tprintf(_T("%s"),httpResponseCookies.c_str());
    tstring httpResponseHeader = client.GetResponseHeader();
	_tprintf(_T("%s"),httpResponseHeader.c_str());
    tstring httpResponseContent = client.GetResponseContent();
	_tprintf(_T("%s"),httpResponseContent.c_str());
}

void CompleteTest(void)
{
    bool ret = false;
	// 1. Get the initial cookie.
    WinHttpClient getClient(_T("http://www.codeproject.com/script/Membership/LogOn.aspx"));
    getClient.SetAdditionalRequestHeaders(
		_T("Accept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*\r\nAccept-Language: en-us\r\n")
		_T("Host: www.codeproject.com\r\n")
		_T("User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; QQPinyin 730; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.2; CIBA; MS-RTC LM 8)\r\n")
		_T("Accept-Encoding: gzip, deflate\r\n")
		_T("Proxy-Connection: Keep-Alive\r\n"));
    ret = getClient.SendHttpRequest(); if (!ret) return;
	tstring sCookies = getClient.GetResponseCookies(); //SessionGUID=075a6cb2-7e4b-4a9b-a970-a745db054a2c; path=/;
	_tprintf(_T("%s"),sCookies.c_str());

    // 2. Post data to get the authentication cookie.
    WinHttpClient postClient(_T("http://www.codeproject.com/script/Membership/LogOn.aspx?rp=%2fscript%2fMembership%2fLogOn.aspx"));

    // Post data.
    // Change this to your Codeproject username and password.
    tstring username = _T("YourCodeProjectUsername");
    tstring password = _T("YourPassword");
    postClient.SetAdditionalRequestCookies(sCookies);
    tstring data = _T("FormName=MenuBarForm&Email=");
    data += username;
    data += "&Password=";
    data += password;
    data += "&RememberMeCheck=1";
    postClient.SetAdditionalDataToSend((BYTE *)data.c_str(), data.size());

    // Post headers.
    tstring headers = 
		_T("Accept: image/gif, image/jpeg, image/pjpeg, image/pjpeg, application/x-shockwave-flash, application/x-ms-application, application/x-ms-xbap, application/vnd.ms-xpsdocument, application/xaml+xml, application/vnd.ms-excel, application/vnd.ms-powerpoint, application/msword, */*\r\n")
		_T("Host: www.codeproject.com\r\n")
		_T("Referer: http://www.codeproject.com/script/Membership/LogOn.aspx\r\nAccept-Language: en-us\r\n")
		_T("User-Agent: Mozilla/4.0 (compatible; MSIE 8.0; Windows NT 5.1; Trident/4.0; QQPinyin 730; .NET CLR 1.1.4322; .NET CLR 2.0.50727; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; InfoPath.2; CIBA; MS-RTC LM 8)\r\n")
		_T("Content-Type: application/x-www-form-urlencoded\r\n")
		_T("Content-Length: %d\r\n")
		_T("Proxy-Connection: Keep-Alive\r\n")
		_T("Pragma: no-cache\r\n");
    TCHAR szHeaders[MAX_PATH * 10] = _T("");
    _stprintf_s(szHeaders, MAX_PATH * 10, headers.c_str(), data.size());
    postClient.SetAdditionalRequestHeaders(szHeaders);
    ret = postClient.SendHttpRequest(_T("POST"), true); if (!ret) return;

    // 3. Finally get the zip file.    
    WinHttpClient downloadClient(_T("http://www.codeproject.com/KB/IP/win_HTTP_wrapper/WinHttpClient_Src.zip"),ProgressProc);
    // Sending this cookie make server believe you have already logged in.
    downloadClient.SetAdditionalRequestCookies(sCookies);
    ret = downloadClient.SendHttpRequest(); if (!ret) return;
    downloadClient.SaveResponseToFile(_T("C:\\WinHttpClient_Src.zip"));
}

int _tmain(int /*argc*/, _TCHAR** /*argv*/)
{
	//CookiesTest();
	//HttpsTest();
    //CompleteTest();
	char szBuf[1048] = {0};
	tstring sessionid = _T("");
	tstring url = 
		_T("http://218.249.15.109:8087//exchange_admin/com.forlink.exchange.admin.settle.settleList.SettleListForC")
		_T("?module_name=settle&request_type=login&return_code=&return_desc=&list_num=&emp_acct=999101&password=1111");
	WinHttpClient downloadClient(url);
	bool ret = downloadClient.SendHttpRequest();
	if (ret) {
		downloadClient.SaveResponseToFile(_T(".\\loginRespose.txt"));
		ifstream fR(".\\loginRespose.txt"); if (!fR.is_open()) return -1;
		while (fR.good()) {fR.getline(szBuf,1024);}
		fR.clear();
		fR.close();
	}
	StringSpliter ss(szBuf,"><"); if (ss.size() < 6) return -1;
	_tprintf(_T("SessionID: %s\n"),ss[6].c_str());
#if defined(_UNICODE) || defined(UNICODE)
	sessionid = AnsiToUnicode(ss[6].c_str());
#else
	sessionid = ss[6].c_str();
#endif
#if 1
	url =
		_T("http://218.249.15.109:8087//exchange_admin/com.forlink.exchange.admin.settle.settleList.SettleListForC")
		_T("?module_name=settle&request_type=file_list&return_code=&return_desc=&list_num=&session_id=") + sessionid +
		_T("&client_id=99910100010001&file_name=/"); //99910900010001
	downloadClient.UpdateUrl(url);
	ret = downloadClient.SendHttpRequest();
	if (ret) {
		downloadClient.SaveResponseToFile(_T(".\\fileListRespose.html"));
	}

	url =
		_T("http://218.249.15.109:8087//exchange_admin/com.forlink.exchange.admin.settle.settleList.SettleListForC")
		_T("?module_name=settle&request_type=file_content&return_code=&return_desc=&list_num=&session_id=") + sessionid +
		_T("&client_id=99910100010001&file_name=LAST_ASSO_KX"); //99910900010001
	downloadClient.UpdateUrl(url);
	ret = downloadClient.SendHttpRequest();
	if (ret) {
		downloadClient.SaveResponseToFile(_T(".\\fileRequestRespose.html"));
	}
#endif

	system("pause");
	return 0;
}

