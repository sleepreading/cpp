// XWinVer.h  Version 1.2
#ifndef XWINVER_H
#define XWINVER_H
#include <tchar.h>
///////////////////////////////////////////////////////////////////////////////
//
// version strings
//
#define WUNKNOWNSTR	_T("unknown Windows version")

#define W95STR		_T("Windows 95")
#define W95SP1STR	_T("Windows 95 SP1")
#define W95OSR2STR	_T("Windows 95 OSR2")
#define W98STR		_T("Windows 98")
#define W98SP1STR	_T("Windows 98 SP1")
#define W98SESTR	_T("Windows 98 SE")
#define WMESTR		_T("Windows ME")

#define WNT351STR	_T("Windows NT 3.51")
#define WNT4STR		_T("Windows NT 4")
#define W2KSTR		_T("Windows 2000")
#define WXPSTR		_T("Windows XP")
#define WXP64STR	_T("Windows XP x64 Edition")
#define W2003STR	_T("Windows Server 2003")
#define W2008STR	_T("Windows Server 2008")
#define W2008R2STR	_T("Windows Server 2008 R2")
#define W2012STR	_T("Windows Server 2012")
#define W2012R2STR	_T("Windows Server 2012 R2")
#define WVISTASTR	_T("Windows Vista")
#define WIN7STR		_T("Windows 7")
#define WIN8STR		_T("Windows 8")
#define WIN81STR	_T("Windows 8.1")
#define WIN10STR	_T("Windows 10")

#define WCESTR		_T("Windows CE")


///////////////////////////////////////////////////////////////////////////////
//
// version integer values
//
#define WUNKNOWN	0

#define W9XFIRST	1
#define W95			1
#define W95SP1		2
#define W95OSR2		3
#define W98			4
#define W98SP1		5
#define W98SE		6
#define WME			7
#define W9XLAST		99

#define WNTFIRST	101
#define WNT351		101
#define WNT4		102
#define W2K			103
#define WXP			104
#define W2003		105
#define WVISTA		106
#define WIN7		107
#define WIN8		108
#define WIN81		109
#define WIN10		110
#define WNTLAST		199

#define WCEFIRST	201
#define WCE			201
#define WCELAST		299

// flags returned from GetVistaProductType()
#ifndef PRODUCT_BUSINESS
#define PRODUCT_BUSINESS						0x00000006 	// Business Edition
#define PRODUCT_BUSINESS_N						0x00000010 	// Business Edition
#define PRODUCT_CLUSTER_SERVER					0x00000012 	// Cluster Server Edition
#define PRODUCT_DATACENTER_SERVER				0x00000008 	// Server Datacenter Edition (full installation)
#define PRODUCT_DATACENTER_SERVER_CORE			0x0000000C 	// Server Datacenter Edition (core installation)
#define PRODUCT_ENTERPRISE						0x00000004 	// Enterprise Edition
#define PRODUCT_ENTERPRISE_N					0x0000001B 	// Enterprise Edition
#define PRODUCT_ENTERPRISE_SERVER				0x0000000A 	// Server Enterprise Edition (full installation)
#define PRODUCT_ENTERPRISE_SERVER_CORE			0x0000000E 	// Server Enterprise Edition (core installation)
#define PRODUCT_ENTERPRISE_SERVER_IA64			0x0000000F 	// Server Enterprise Edition for Itanium-based Systems
#define PRODUCT_HOME_BASIC						0x00000002 	// Home Basic Edition
#define PRODUCT_HOME_BASIC_N					0x00000005 	// Home Basic Edition
#define PRODUCT_HOME_PREMIUM					0x00000003 	// Home Premium Edition
#define PRODUCT_HOME_PREMIUM_N					0x0000001A 	// Home Premium Edition
#define PRODUCT_HOME_SERVER						0x00000013 	// Home Server Edition
#define PRODUCT_SERVER_FOR_SMALLBUSINESS		0x00000018 	// Server for Small Business Edition
#define PRODUCT_SMALLBUSINESS_SERVER			0x00000009 	// Small Business Server
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM	0x00000019 	// Small Business Server Premium Edition
#define PRODUCT_STANDARD_SERVER					0x00000007 	// Server Standard Edition (full installation)
#define PRODUCT_STANDARD_SERVER_CORE			0x0000000D 	// Server Standard Edition (core installation)
#define PRODUCT_STARTER							0x0000000B 	// Starter Edition
#define PRODUCT_STORAGE_ENTERPRISE_SERVER		0x00000017 	// Storage Server Enterprise Edition
#define PRODUCT_STORAGE_EXPRESS_SERVER			0x00000014 	// Storage Server Express Edition
#define PRODUCT_STORAGE_STANDARD_SERVER			0x00000015 	// Storage Server Standard Edition
#define PRODUCT_STORAGE_WORKGROUP_SERVER		0x00000016 	// Storage Server Workgroup Edition
#define PRODUCT_UNDEFINED						0x00000000 	// An unknown product
#define PRODUCT_ULTIMATE						0x00000001 	// Ultimate Edition
#define PRODUCT_ULTIMATE_N						0x0000001C 	// Ultimate Edition
#define PRODUCT_WEB_SERVER						0x00000011 	// Web Server Edition
#endif

///////////////////////////////////////////////////////////////////////////////
//
// CXWinVersion singleton class
//
class CXWinVersion
{
public:
	CXWinVersion();

public:
	unsigned long GetMajorVersion()	{ return m_os_info.dwMajorVersion; }
	unsigned long GetMinorVersion()	{ return m_os_info.dwMinorVersion; }
	unsigned long GetBuildNumber()	{ return (m_os_info.dwBuildNumber & 0xFFFF); } // needed for 9x
	unsigned long GetPlatformId()	{ return (m_os_info.dwPlatformId); }
	unsigned long GetProductInfo(unsigned long major=6, unsigned long minor=0);
	TCHAR*	GetServicePackString()	{ return m_os_info.szCSDVersion; }
	TCHAR*  GetProductString();
	TCHAR*	GetWinVersionString();
	int		GetServicePackNT();
	int		GetWinVersion();
	bool	Is64OS()				{ return m_is64os; }
	bool	IsWin95();
	bool	IsWin98();
	bool	IsWin2KorLater();
	bool	IsWin2003();
	bool	IsWinCE();
	bool	IsXP();
	bool	IsXPorLater();
	bool	IsXPHome();
	bool	IsXPPro();
	bool	IsXPSP2();
	bool	IsMediaCenter();
	bool	IsVista();
	bool	IsVistaHome();
	bool	IsVistaBusiness();
	bool	IsVistaEnterprise();
	bool	IsVistaUltimate();

// Implementation
private:
	void Init();

	struct OSVERSIONINFOEXX
	{  
		unsigned long dwOSVersionInfoSize;  
		unsigned long dwMajorVersion;  
		unsigned long dwMinorVersion;  
		unsigned long dwBuildNumber;  
		unsigned long dwPlatformId;  
		TCHAR szCSDVersion[128];  
		unsigned short  wServicePackMajor;  
		unsigned short  wServicePackMinor;  
		unsigned short  wSuiteMask;  
		unsigned char  wProductType;  
		unsigned char  wReserved;
	};
	OSVERSIONINFOEXX m_os_info;
	unsigned long m_product_info;
	TCHAR m_product_info_str[32];
	TCHAR m_version_str[32];
	bool  m_initialized;
	bool  m_is64os;
};


///////////////////////////////////////////////////////////////////////////////
//
// CXWinVersion instance
//
#ifndef XWINVER_CPP
// include an instance in each file;  the namespace insures uniqueness
namespace { CXWinVersion WinVersion; }
#endif


#endif //XWINVER_H
