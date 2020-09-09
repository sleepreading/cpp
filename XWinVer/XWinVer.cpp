// XWinVer.cpp  Version 1.2
//
// Public APIs:
//             NAME                             DESCRIPTION
//     ---------------------   -------------------------------------------------
//     GetMajorVersion()       Get major version number
//     GetMinorVersion()       Get minor version number
//     GetBuildNumber()        Get build number (ANDed with 0xFFFF for Win9x)
//     GetServicePackNT()      Get service pack number
//     GetPlatformId()         Get platform id
//     GetServicePackString()  Get service pack string
//     GetVistaProductString() Get Vista product string (e.g., "Business Edition")
//     GetVistaProductType()   Get Vista product type
//     GetWinVersion()         Get windows version (CXWinVersion code)
//     GetWinVersionString()   Get windows version as string
//     IsMediaCenter()         TRUE = Media Center Edition
//     IsWin95()               TRUE = Win95
//     IsWin98()               TRUE = Win98
//     IsWin2KorLater()        TRUE = Win2000 or later
//     IsWin2003()             TRUE = Win2003
//     IsWinCE()               TRUE = WinCE
//     IsXP()                  TRUE = XP
//     IsXPorLater()           TRUE = XP or later
//     IsXPHome()              TRUE = XP Home
//     IsXPPro()               TRUE = XP Pro
//     IsXPSP2()               TRUE = XP SP2
//     IsVista()               TRUE = Vista
//     IsVistaHome()           TRUE = Vista Home
//     IsVistaBusiness()       TRUE = Vista Business
//     IsVistaEnterprise()     TRUE = Vista Enterprise
//     IsVistaUltimate()       TRUE = Vista Ultimate
///////////////////////////////////////////////////////////////////////////////

#define XWINVER_CPP
#include "XWinVer.h"
#include <windows.h>

#pragma warning(disable: 4996)

// from winbase.h
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS      1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT           2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE           3
#endif
// from winnt.h
#ifndef VER_NT_WORKSTATION
#define VER_NT_WORKSTATION              0x0000001
#endif
#ifndef VER_SUITE_PERSONAL
#define VER_SUITE_PERSONAL              0x00000200
#endif


///////////////////////////////////////////////////////////////////////////////
/*
    This table has been assembled from Usenet postings, personal
    observations, and reading other people's code.  Please feel
    free to add to it or correct it.


         dwPlatFormID  dwMajorVersion  dwMinorVersion  dwBuildNumber
95             1              4               0             950
95 SP1         1              4               0        >950 && <=1080
95 OSR2        1              4             <10           >1080
98             1              4              10            1998
98 SP1         1              4              10       >1998 && <2183
98 SE          1              4              10          >=2183
ME             1              4              90            3000

NT 3.51        2              3              51            1057
NT 4           2              4               0            1381
2000           2              5               0            2195
XP             2              5               1            2600
2003           2              5               2            3790
Vista          2              6               0
Win7           2              6               1            7601
Win8           2              6               2
Win8.1         2              6               3
Win10          2              10              0       >9888 && <10572(2015-10-21) //(9888时由6.4变为10.0)

CE 1.0         3              1               0
CE 2.0         3              2               0
CE 2.1         3              2               1
CE 3.0         3              3               0
*/

CXWinVersion::CXWinVersion() : m_initialized(false),m_is64os(false),m_product_info(0)
{
	if (!m_initialized) { Init(); } 
}

void CXWinVersion::Init()
{
	memset(m_version_str,0,sizeof(m_version_str));
	memset(m_product_info_str,0,sizeof(m_product_info_str));
	memset(&m_os_info, 0, sizeof(m_os_info));
	m_os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx((LPOSVERSIONINFO) &m_os_info))
	{
		m_initialized = TRUE;
		if ((m_os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) &&	(m_os_info.dwMajorVersion >= 5))
		{// get extended version info for 2000 and later
			memset(&m_os_info, 0, sizeof(m_os_info));
			m_os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
			GetVersionEx((LPOSVERSIONINFO) &m_os_info);
		}
	}
	typedef void (__stdcall *LPFN_PGNSI)(LPSYSTEM_INFO);
	SYSTEM_INFO si = { 0 };
	LPFN_PGNSI pGNSI = (LPFN_PGNSI)GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");
	if (pGNSI != NULL) {
		pGNSI(&si);
		if (si.wProcessorArchitecture==6 || si.wProcessorArchitecture==9) {
			m_is64os = TRUE;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// GetWinVersionString
TCHAR* CXWinVersion::GetWinVersionString()
{
	_tcscpy(m_version_str,WUNKNOWNSTR);

	int nVersion = GetWinVersion();
	switch (nVersion)
	{
		default:
		case WUNKNOWN:									break;
		case W95:	 _tcscpy(m_version_str,W95STR);		break;
		case W95SP1: _tcscpy(m_version_str,W95SP1STR);	break;
		case W95OSR2:_tcscpy(m_version_str,W95OSR2STR);	break;
		case W98:	 _tcscpy(m_version_str,W98STR);		break;
		case W98SP1: _tcscpy(m_version_str,W98SP1STR);	break;
		case W98SE:	 _tcscpy(m_version_str,W98SESTR);	break;
		case WME:	 _tcscpy(m_version_str,WMESTR);		break;
		case WNT351: _tcscpy(m_version_str,WNT351STR);	break;
		case WNT4:	 _tcscpy(m_version_str,WNT4STR);		break;
		case W2K:	 _tcscpy(m_version_str,W2KSTR);		break;
		case WXP:	 _tcscpy(m_version_str,WXPSTR);		break;
		case W2003:
		{
			if (m_os_info.wProductType==3 || m_os_info.wProductType==2) {
				_tcscpy(m_version_str,W2003STR);
			} else {
				_tcscpy(m_version_str,WXP64STR);
			}
			break;
		}
		case WVISTA:
		{
			if (m_os_info.wProductType==3 || m_os_info.wProductType==2) {
				_tcscpy(m_version_str,W2008STR);
			} else {
				_tcscpy(m_version_str,WVISTASTR);
			}
			break;
		}
		case WIN7:
		{
			if (m_os_info.wProductType==3 || m_os_info.wProductType==2) {
				_tcscpy(m_version_str,W2008R2STR);
			} else {
				_tcscpy(m_version_str,WIN7STR);
			}
			break;
		}
		case WIN8:
		{
			if (m_os_info.wProductType==3 || m_os_info.wProductType==2) {
				_tcscpy(m_version_str,W2012STR);
			} else {
				_tcscpy(m_version_str,WIN8STR);
			}
			break;
		}
		case WIN81:
		{
			if (m_os_info.wProductType==3 || m_os_info.wProductType==2) {
				_tcscpy(m_version_str,W2012R2STR);
			} else {
				_tcscpy(m_version_str,WIN81STR);
			}
			break;
		}
		case WIN10:	_tcscpy(m_version_str,WIN10STR);	break;
		case WCE:	_tcscpy(m_version_str,WCESTR);		break;
	}

	return m_version_str;
}

///////////////////////////////////////////////////////////////////////////////
// GetWinVersion
int CXWinVersion::GetWinVersion()
{
	int nVersion = WUNKNOWN;

	unsigned long dwPlatformId   = m_os_info.dwPlatformId;
	unsigned long dwMinorVersion = m_os_info.dwMinorVersion;
	unsigned long dwMajorVersion = m_os_info.dwMajorVersion;
	unsigned long dwBuildNumber  = m_os_info.dwBuildNumber & 0xFFFF;	// Win 9x needs this

	if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
	{
		if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
		{
			nVersion = W95;
		}
		else if ((dwMinorVersion < 10) &&
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
		{
			nVersion = W95SP1;
		}
		else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
		{
			nVersion = W95OSR2;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
		{
			nVersion = W98;
		}
		else if ((dwMinorVersion == 10) &&
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
		{
			nVersion = W98SP1;
		}
		else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
		{
			nVersion = W98SE;
		}
		else if (dwMinorVersion == 90)
		{
			nVersion = WME;
		}
	}
	else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
		{
			nVersion = WNT351;
		}
		else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
		{
			nVersion = WNT4;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
		{
			nVersion = W2K;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
		{
			nVersion = WXP;
		}
		else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
		{
			nVersion = W2003;
		}
		else if ((dwMajorVersion == 6) && (dwMinorVersion == 0))
		{
			nVersion = WVISTA;
			GetProductInfo(6,0);
		}
		else if ((dwMajorVersion == 6) && (dwMinorVersion == 1))
		{
			nVersion = WIN7;
			GetProductInfo(6,1);
		}
		else if ((dwMajorVersion == 6) && (dwMinorVersion == 2))
		{
			nVersion = WIN8;
			GetProductInfo(6,2);
		}
		else if ((dwMajorVersion == 6) && (dwMinorVersion == 3))
		{
			nVersion = WIN81;
			GetProductInfo(6,3);
		}
		else if (dwMajorVersion == 10)
		{
			nVersion = WIN10;
			GetProductInfo(10,0);
		}
	}
	else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
	{
		nVersion = WCE;
	}

	return nVersion;
}

///////////////////////////////////////////////////////////////////////////////
// GetServicePackNT - returns a valid service pack number only for NT platform
int CXWinVersion::GetServicePackNT()
{
	int nServicePack = 0;

	for (int i = 0; (m_os_info.szCSDVersion[i] != _T('\0')) && (i < (sizeof(m_os_info.szCSDVersion)/sizeof(TCHAR))); i++)
	{
		if (_istdigit(m_os_info.szCSDVersion[i]))
		{
			nServicePack = _ttoi(&m_os_info.szCSDVersion[i]);
			break;
		}
	}

	return nServicePack;
}

///////////////////////////////////////////////////////////////////////////////
// IsXP
bool CXWinVersion::IsXP()
{
	if (GetWinVersion()==WXP || (GetWinVersion()==W2003 && m_os_info.wProductType!=3 && m_os_info.wProductType!=2))
	{
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsXPHome
bool CXWinVersion::IsXPHome()
{
	if (IsXP())
	{
		if (m_os_info.wSuiteMask & VER_SUITE_PERSONAL)
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsXPPro
bool CXWinVersion::IsXPPro()
{
	if (IsXP())
	{
		if ((m_os_info.wProductType == VER_NT_WORKSTATION) && !IsXPHome())
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsXPSP2
bool CXWinVersion::IsXPSP2()
{
	if (IsXP())
	{
		if (GetServicePackNT() == 2)
			return true;
	}
	return false;
}

#ifndef SM_MEDIACENTER
#define SM_MEDIACENTER          87
#endif

///////////////////////////////////////////////////////////////////////////////
// IsMediaCenter
bool CXWinVersion::IsMediaCenter()
{
	if (GetSystemMetrics(SM_MEDIACENTER))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsWin2003
bool CXWinVersion::IsWin2003()
{
	if ((m_os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		(m_os_info.dwMajorVersion == 5) &&
		(m_os_info.dwMinorVersion == 2))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// GetVistaProductType
unsigned long CXWinVersion::GetProductInfo(unsigned long major, unsigned long minor)
{
	if (m_product_info == 0)
	{
		typedef BOOL (PASCAL *lpfnGetProductInfo) (DWORD, DWORD, DWORD, DWORD, PDWORD);
		HMODULE hKernel32 = GetModuleHandle(_T("KERNEL32.DLL"));
		if (hKernel32)
		{
			lpfnGetProductInfo pGetProductInfo = (lpfnGetProductInfo) GetProcAddress(hKernel32, "GetProductInfo"); 
			if (pGetProductInfo)
				pGetProductInfo(major, minor, 0, 0, &m_product_info);
		}  
	}

	return m_product_info;
}

///////////////////////////////////////////////////////////////////////////////
// GetVistaProductString
TCHAR* CXWinVersion::GetProductString()
{
	memset(m_product_info_str,0,sizeof(m_product_info_str));
	switch (m_product_info)
	{
		case PRODUCT_BUSINESS:						 _tcscpy(m_product_info_str,_T("Business Edition")); break;
		case PRODUCT_BUSINESS_N:					 _tcscpy(m_product_info_str,_T("Business Edition")); break;
		case PRODUCT_CLUSTER_SERVER:				 _tcscpy(m_product_info_str,_T("Cluster Server Edition")); break;
		case PRODUCT_DATACENTER_SERVER:				 _tcscpy(m_product_info_str,_T("Server Datacenter Edition (full installation)")); break;
		case PRODUCT_DATACENTER_SERVER_CORE:		 _tcscpy(m_product_info_str,_T("Server Datacenter Edition (core installation)")); break;
		case PRODUCT_ENTERPRISE:					 _tcscpy(m_product_info_str,_T("Enterprise Edition")); break;
		case PRODUCT_ENTERPRISE_N:					 _tcscpy(m_product_info_str,_T("Enterprise Edition")); break;
		case PRODUCT_ENTERPRISE_SERVER:				 _tcscpy(m_product_info_str,_T("Server Enterprise Edition (full installation)")); break;
		case PRODUCT_ENTERPRISE_SERVER_CORE:		 _tcscpy(m_product_info_str,_T("Server Enterprise Edition (core installation)")); break;
		case PRODUCT_ENTERPRISE_SERVER_IA64:		 _tcscpy(m_product_info_str,_T("Server Enterprise Edition for Itanium-based Systems")); break;
		case PRODUCT_HOME_BASIC:					 _tcscpy(m_product_info_str,_T("Home Basic Edition")); break;
		case PRODUCT_HOME_BASIC_N:					 _tcscpy(m_product_info_str,_T("Home Basic Edition")); break;
		case PRODUCT_HOME_PREMIUM:					 _tcscpy(m_product_info_str,_T("Home Premium Edition")); break;
		case PRODUCT_HOME_PREMIUM_N:				 _tcscpy(m_product_info_str,_T("Home Premium Edition")); break;
		case PRODUCT_HOME_SERVER:					 _tcscpy(m_product_info_str,_T("Home Server Edition")); break;
		case PRODUCT_SERVER_FOR_SMALLBUSINESS:		 _tcscpy(m_product_info_str,_T("Server for Small Business Edition")); break;
		case PRODUCT_SMALLBUSINESS_SERVER:			 _tcscpy(m_product_info_str,_T("Small Business Server")); break;
		case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:	 _tcscpy(m_product_info_str,_T("Small Business Server Premium Edition")); break;
		case PRODUCT_STANDARD_SERVER:				 _tcscpy(m_product_info_str,_T("Server Standard Edition (full installation)")); break;
		case PRODUCT_STANDARD_SERVER_CORE:			 _tcscpy(m_product_info_str,_T("Server Standard Edition (core installation)")); break;
		case PRODUCT_STARTER:						 _tcscpy(m_product_info_str,_T("Starter Edition")); break;
		case PRODUCT_STORAGE_ENTERPRISE_SERVER:		 _tcscpy(m_product_info_str,_T("Storage Server Enterprise Edition")); break;
		case PRODUCT_STORAGE_EXPRESS_SERVER:		 _tcscpy(m_product_info_str,_T("Storage Server Express Edition")); break;
		case PRODUCT_STORAGE_STANDARD_SERVER:		 _tcscpy(m_product_info_str,_T("Storage Server Standard Edition")); break;
		case PRODUCT_STORAGE_WORKGROUP_SERVER:		 _tcscpy(m_product_info_str,_T("Storage Server Workgroup Edition")); break;
		case PRODUCT_UNDEFINED:						 _tcscpy(m_product_info_str,_T("An unknown product")); break;
		case PRODUCT_ULTIMATE:						 _tcscpy(m_product_info_str,_T("Ultimate Edition")); break;
		case PRODUCT_ULTIMATE_N:					 _tcscpy(m_product_info_str,_T("Ultimate Edition")); break;
		case PRODUCT_WEB_SERVER:					 _tcscpy(m_product_info_str,_T("Web Server Edition")); break;

		default: break;
	}

	return m_product_info_str;
}

///////////////////////////////////////////////////////////////////////////////
// IsVista
bool CXWinVersion::IsVista()
{
	if (GetWinVersion()==WVISTA && (m_os_info.wProductType!=3 && m_os_info.wProductType!=2))
	{
		return true;
	}
	return false;
}

bool CXWinVersion::IsVistaHome()
{
	if (IsVista())
	{
		switch (m_product_info)
		{
			case PRODUCT_HOME_BASIC:	
			case PRODUCT_HOME_BASIC_N:	
			case PRODUCT_HOME_PREMIUM:	
			case PRODUCT_HOME_PREMIUM_N:
			case PRODUCT_HOME_SERVER:	
				return true;
		}
	}
	return false;
}

bool CXWinVersion::IsVistaBusiness()
{
	if (IsVista())
	{
		switch (m_product_info)
		{
			case PRODUCT_BUSINESS:	
			case PRODUCT_BUSINESS_N:
				return true;
		}
	}
	return false;
}

bool CXWinVersion::IsVistaEnterprise()
{
	if (IsVista())
	{
		switch (m_product_info)
		{
			case PRODUCT_ENTERPRISE:			
			case PRODUCT_ENTERPRISE_N:			
			case PRODUCT_ENTERPRISE_SERVER:		
			case PRODUCT_ENTERPRISE_SERVER_CORE:
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				return true;
		}
	}
	return false;
}

bool CXWinVersion::IsVistaUltimate()
{
	if (IsVista())
	{
		switch (m_product_info)
		{
			case PRODUCT_ULTIMATE:	
			case PRODUCT_ULTIMATE_N:
				return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsWin2KorLater
bool CXWinVersion::IsWin2KorLater()
{
	if ((m_os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		(m_os_info.dwMajorVersion >= 5))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsXPorLater
bool CXWinVersion::IsXPorLater()
{
	if ((m_os_info.dwPlatformId == VER_PLATFORM_WIN32_NT) && 
		(((m_os_info.dwMajorVersion == 5) && (m_os_info.dwMinorVersion > 0)) || 
		(m_os_info.dwMajorVersion > 5)))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsWin95
bool CXWinVersion::IsWin95()
{
	if ((m_os_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && 
		(m_os_info.dwMajorVersion == 4) && 
		(m_os_info.dwMinorVersion < 10))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsWin98
bool CXWinVersion::IsWin98()
{
	if ((m_os_info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && 
		(m_os_info.dwMajorVersion == 4) && 
		(m_os_info.dwMinorVersion >= 10))
		return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// IsWinCE
bool CXWinVersion::IsWinCE()
{
	return (GetWinVersion() == WCE);
}

#pragma warning(default: 4996)

