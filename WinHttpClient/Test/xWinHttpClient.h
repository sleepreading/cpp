/**
 * @file      WinHttpClient enhancement
 * @date      2014-2-12 11:9:42
 * @author    leizi
 * @copyright zhanglei. email:847088355@qq.com
 * @remark    
 */
#ifndef __WINHTTPCLIENT_H__
#define __WINHTTPCLIENT_H__

#include <windows.h>
#include <Winhttp.h>
#include <string>
#include <vector>
#include <tchar.h>
#include "atlrx.h"

using namespace std;
#pragma comment(lib, "Winhttp.lib")

#if defined(_UNICODE) || defined(UNICODE)
#define tstring wstring
#else
#define tstring string
#endif

//////////////////////////////////////////////////////////////////////////
// Global
/**
 * Parameters
 *  [in] regExp: Value of type string which is the input regular expression.
 *  [in] caseSensitive: Value of type bool which indicate whether the parse is case sensitive.
 *  [in] groupCount: Value of type int which is the group count of the regular expression.
 *  [in] source: Value of type string reference which is the source to parse.
 *  [out] result: Value of type vecotr of strings which is the output of the parse.
 *  [in] allowDuplicate: Value of type bool which indicates whether duplicate items are added to the output result.
 *
 * Return Value
 *  Returns true if the function succeeds, or false otherwise.
 *
 * Remarks
 *  The output result is devided into groups.  User should get the groups according to the group count.  For example:
 *  1. RegExp = _T("{ab}"), source = _T("abcabe"), then result = _T("ab"), _T("ab").
 *  2. RegExp = _T("{ab}{cd}"), source = _T("abcdeabecd"), then result = _T("ab"), L"cd", _T("ab"), _T("cd").
*/
inline bool ParseRegExp(const tstring &regExp, bool caseSensitive, int groupCount, const tstring &source, vector<tstring> &result, bool allowDuplicate = false)
{
    result.clear();
    if (regExp.size()<=0 || groupCount<=0 || source.size()<=0) return false;

	CAtlRegExp<> re;
    REParseError error = re.Parse(regExp.c_str(), caseSensitive);
    if (error != REPARSE_ERROR_OK) return false;

	TCHAR *pSource = new TCHAR[source.size()+1];
    TCHAR *pSourceEnd = pSource + source.size();
    if (pSource == NULL) return false;

	_tcscpy_s(pSource, source.size()+1, source.c_str());
    BOOL bSucceed = TRUE;
    CAtlREMatchContext<> mc;
    const TCHAR *pFrom = pSource;
    const TCHAR *pTo = NULL;
    while (bSucceed) {
        bSucceed = re.Match(pFrom, &mc, &pTo);
        if (!bSucceed) break;

		const TCHAR *pStart = NULL;
		const TCHAR *pEnd = NULL;
		vector<tstring> tempMatch;
		for (int i = 0; i < groupCount; i++) {
			mc.GetMatch(i, &pStart, &pEnd);
			if (pStart != NULL && pEnd != NULL) {
				tstring match(pStart, pEnd-pStart);
				tempMatch.push_back(match);
			} else {
				break;
			}
		}
		bool bAdd = true;
		if (!allowDuplicate) {
			// Check whether this match already exists in the vector.
			for (vector<tstring>::iterator it = result.begin(); it != result.end();) {
				bool bEqual = true;
				for (vector<tstring>::iterator tempMatchIt = tempMatch.begin(); tempMatchIt != tempMatch.end(); tempMatchIt++, it++) {
					bool bGroupEqual = true;
					if (caseSensitive) {
						bGroupEqual = (_tcscmp(it->c_str(), tempMatchIt->c_str()) == 0);
					} else {
						bGroupEqual = (_tcsicmp(it->c_str(), tempMatchIt->c_str()) == 0);
					}
					if (!bGroupEqual) {
						bEqual = false;
					}
				}
				if (bEqual) {
					bAdd = false;
					break;
				}
			}
		}
		if (bAdd) {
			for (vector<tstring>::iterator tempMatchIt = tempMatch.begin(); tempMatchIt != tempMatch.end(); tempMatchIt++) {
				result.push_back(*tempMatchIt);
			}
		}
		if (pTo < pSourceEnd) {
			pFrom = pTo;
		} else {
			break;
		}
	}

    delete[] pSource;

    return true;
}

//////////////////////////////////////////////////////////////////////////
// WinHttpClient
typedef bool (*PFHttpDownloadProc)(double);
static const unsigned int	INT_RETRYTIMES = 3;
static TCHAR*				SZ_AGENT = _T("WinHttpClient");
static const int			INT_BUFFERSIZE = 1024*80; // Initial 80 KB temporary buffer, double if it is not enough.

class WinHttpClient
{
public:
    inline WinHttpClient(const tstring &url, PFHttpDownloadProc progressProc = NULL);
    inline ~WinHttpClient(void);

    // It is a synchronized method and may take a long time to finish.
    inline bool SendHttpRequest(const tstring &httpVerb = _T("GET"), bool disableAutoRedirect = false); //this method must be first called
    inline tstring GetResponseHeader(void); //after SendHttpRequest() successfull, get The response header
    inline tstring GetResponseContent(void);
    inline tstring GetResponseCharset(void);
    inline tstring GetResponseStatusCode(void);
    inline tstring GetResponseLocation(void);
    inline tstring GetRequestHost(void);
    inline const BYTE*	GetRawResponseContent(void);
    inline unsigned int GetRawResponseContentLength(void);
    inline unsigned int GetRawResponseReceivedContentLength(void);
    inline DWORD	GetLastError(void);
    inline bool		UpdateUrl(const tstring &url); //同一个对象,多次使用;关闭上次的session,新建一个session
    inline tstring	GetResponseCookies(void);
    inline bool SaveResponseToFile(const tstring &filePath);
    inline bool SetAdditionalRequestCookies(const tstring &cookies);
    inline bool SetAdditionalDataToSend(BYTE *data, unsigned int dataSize); //添加报体部分
    inline bool SetAdditionalRequestHeaders(const tstring &additionalRequestHeaders); //添加出开始行外的报头部分

    inline bool SetTimeout(unsigned int resolveTimeout = 0, unsigned int connectTimeout = 60000, unsigned int sendTimeout = 30000, unsigned int receiveTimeout = 30000);
    inline bool SetProxy(const tstring &proxy);
    inline bool SetUserAgent(const tstring &userAgent);
    inline bool SetForceCharset(const tstring &charset);
    inline bool SetProxyUsername(const tstring &username);
    inline bool SetProxyPassword(const tstring &password);
	inline bool ResetAdditionalDataToSend(void); //UpdateUrl will call this function
    inline bool SetRequireValidSslCertificates(bool require);

private:
    inline WinHttpClient(const WinHttpClient &other);
    inline WinHttpClient &operator =(const WinHttpClient &other);
    inline bool SetProgress(unsigned int byteCountReceived);
	inline tstring Trim(const tstring &source, const tstring &targets);
	inline wchar_t* AnsiToUnicode(const char* str);
	inline char* UnicodeToANSI(const wchar_t *str);

    HINTERNET m_sessionHandle;
    bool m_requireValidSsl;
    bool m_bForceCharset;
    BYTE *m_pResponse;
    BYTE *m_pDataToSend;
    DWORD m_dwLastError;
    tstring m_proxy;
    tstring m_statusCode;
    tstring m_userAgent;
    tstring m_proxyUsername;
    tstring m_proxyPassword;
    tstring m_location;
    tstring m_requestURL;
    tstring m_requestHost;
    tstring m_responseHeader;
    tstring m_responseContent;
    tstring m_responseCharset;
    tstring m_responseCookies;
    tstring m_additionalRequestCookies;
    tstring m_additionalRequestHeaders;
    unsigned int m_responseByteCountReceived;   // Up to 4GB.
    unsigned int m_responseByteCount;
    unsigned int m_dataToSendSize;
	unsigned int m_resolveTimeout;
	unsigned int m_connectTimeout;
	unsigned int m_sendTimeout;
	unsigned int m_receiveTimeout;
    PFHttpDownloadProc m_pfProcessProc;
};

WinHttpClient::WinHttpClient(const tstring &url, PFHttpDownloadProc progressProc)
    : m_requestURL(url), m_sessionHandle(NULL), m_requireValidSsl(false), m_responseHeader(_T("")),
      m_responseContent(_T("")), m_responseCharset(_T("")), m_requestHost(_T("")), m_pResponse(NULL),
      m_responseByteCountReceived(0), m_pfProcessProc(progressProc), m_responseByteCount(0),
      m_responseCookies(_T("")), m_additionalRequestCookies(_T("")), m_pDataToSend(NULL), m_dataToSendSize(0),
      m_proxy(_T("")), m_dwLastError(0), m_statusCode(_T("")), m_userAgent(SZ_AGENT), m_bForceCharset(false),
      m_proxyUsername(_T("")), m_proxyPassword(_T("")), m_location(_T("")), m_resolveTimeout(0), m_connectTimeout(60000),
      m_sendTimeout(30000), m_receiveTimeout(30000)
{
}

WinHttpClient::~WinHttpClient(void)
{
    if (m_pResponse != NULL)
        delete[] m_pResponse;
    if (m_pDataToSend != NULL)
        delete[] m_pDataToSend;

    if (m_sessionHandle != NULL)
        ::WinHttpCloseHandle(m_sessionHandle);
}

bool WinHttpClient::SendHttpRequest(const tstring &httpVerb, bool disableAutoRedirect)
{
    if (m_requestURL.size() <= 0) {
        m_dwLastError = ERROR_PATH_NOT_FOUND;
        return false;
    }
    // Make verb uppercase.
    tstring verb = httpVerb;
    if (_tcsicmp(verb.c_str(), _T("GET")) == 0) {
        verb = _T("GET");
    } else if (_tcsicmp(verb.c_str(), _T("POST")) == 0) {
        verb = _T("POST");
    } else {
        m_dwLastError = ERROR_INVALID_PARAMETER;
        return false;
    }
	const wchar_t* pwszTmp = NULL;
	const char* pszTmp = NULL;

    if (m_sessionHandle == NULL) {
#if defined(_UNICODE) || defined(UNICODE)
        m_sessionHandle = ::WinHttpOpen(m_userAgent.c_str(), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
#else
		pwszTmp = AnsiToUnicode(m_userAgent.c_str()); if (!pwszTmp) return false;
		m_sessionHandle = ::WinHttpOpen(pwszTmp, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
		free((void*)pwszTmp); pwszTmp = NULL;
#endif
        if (m_sessionHandle == NULL) {
            m_dwLastError = ::GetLastError();
            return false;
        }
    }

    ::WinHttpSetTimeouts(m_sessionHandle, m_resolveTimeout, m_connectTimeout, m_sendTimeout, m_receiveTimeout);

    wchar_t szHostName[MAX_PATH] = {0};
	wchar_t szURLPath[MAX_PATH*5] = {0};
    URL_COMPONENTS urlComp;
    memset(&urlComp, 0, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);
    urlComp.lpszHostName = szHostName;
    urlComp.dwHostNameLength = MAX_PATH;
    urlComp.lpszUrlPath = szURLPath;
    urlComp.dwUrlPathLength = MAX_PATH * 5;
    urlComp.dwSchemeLength = 1; // None zero
    bool bRetVal = true;

#if defined(_UNICODE) || defined(UNICODE)
	if (!::WinHttpCrackUrl(m_requestURL.c_str(), m_requestURL.size(), 0, &urlComp))
		return false;
#else
	pwszTmp = AnsiToUnicode(m_requestURL.c_str()); if (!pwszTmp) return false;
	if (!::WinHttpCrackUrl(pwszTmp, m_requestURL.size(), 0, &urlComp)) {
		free((void*)pwszTmp); pwszTmp = NULL;
		return false;
	}
	free((void*)pwszTmp); pwszTmp = NULL;
#endif

#if defined(_UNICODE) || defined(UNICODE)
	m_requestHost = szHostName;
#else
	pszTmp = UnicodeToANSI(szHostName); if (!pszTmp) return false;
	m_requestHost = pszTmp;
	free((void*)pszTmp); pszTmp = NULL;
#endif
	HINTERNET hConnect = NULL;
	hConnect = ::WinHttpConnect(m_sessionHandle, szHostName, urlComp.nPort, 0);
	if (!hConnect) return false;

    DWORD dwOpenRequestFlag = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
	HINTERNET hRequest = NULL;
#if defined(_UNICODE) || defined(UNICODE)
    hRequest = ::WinHttpOpenRequest(hConnect, verb.c_str(), urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
#else
	pwszTmp = AnsiToUnicode(verb.c_str()); if (!pwszTmp) return false;
	hRequest = ::WinHttpOpenRequest(hConnect, pwszTmp, urlComp.lpszUrlPath, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, dwOpenRequestFlag);
	free((void*)pwszTmp); pwszTmp = NULL;
#endif
	if (!hRequest) return false;

    // If HTTPS, then client is very susceptable to invalid certificates
    // Easiest to accept anything for now
    if (!m_requireValidSsl && urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
        DWORD options = SECURITY_FLAG_IGNORE_CERT_CN_INVALID | SECURITY_FLAG_IGNORE_CERT_DATE_INVALID | SECURITY_FLAG_IGNORE_UNKNOWN_CA;
        ::WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, (LPVOID)&options, sizeof(DWORD));
    }

    bool bGetReponseSucceed = false;
    unsigned int iRetryTimes = 0;

    // Retry for several times if fails.
    while (!bGetReponseSucceed && iRetryTimes++ < INT_RETRYTIMES)
    {
        if (m_additionalRequestHeaders.size() > 0) {
#if defined(_UNICODE) || defined(UNICODE)
            if (!::WinHttpAddRequestHeaders(hRequest, m_additionalRequestHeaders.c_str(), m_additionalRequestHeaders.size(), WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON))
				m_dwLastError = ::GetLastError();
#else
			pwszTmp = AnsiToUnicode(m_additionalRequestHeaders.c_str()); if (!pwszTmp) break;
			if (!::WinHttpAddRequestHeaders(hRequest, pwszTmp, m_additionalRequestHeaders.size(), WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON)) {
				free((void*)pwszTmp); pwszTmp = NULL;
				m_dwLastError = ::GetLastError();
			}
			free((void*)pwszTmp); pwszTmp = NULL;
#endif
		}

        if (m_additionalRequestCookies.size() > 0) {
            tstring cookies = _T("Cookie: ");
            cookies += m_additionalRequestCookies;
#if defined(_UNICODE) || defined(UNICODE)
            if (!::WinHttpAddRequestHeaders(hRequest, cookies.c_str(), cookies.size(), WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON))
				m_dwLastError = ::GetLastError();
#else
			pwszTmp = AnsiToUnicode(cookies.c_str());
			if (!::WinHttpAddRequestHeaders(hRequest, pwszTmp, cookies.size(), WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON)) {
				free((void*)pwszTmp); pwszTmp = NULL;
				m_dwLastError = ::GetLastError();
			}
			free((void*)pwszTmp); pwszTmp = NULL;
#endif
        }

        if (m_proxy.size() > 0) {
            WINHTTP_PROXY_INFO proxyInfo;
            memset(&proxyInfo, 0, sizeof(proxyInfo));
            proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
            wchar_t szProxy[MAX_PATH] = {0};
#if defined(_UNICODE) || defined(UNICODE)
            _tcscpy_s(szProxy, MAX_PATH, m_proxy.c_str());
#else
			pwszTmp = AnsiToUnicode(m_proxy.c_str());
			wcscpy_s(szProxy,pwszTmp);
			free((void*)pwszTmp); pwszTmp = NULL;
#endif
			proxyInfo.lpszProxy = szProxy;

            if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
                m_dwLastError = ::GetLastError();

            if (m_proxyUsername.size() > 0) {
#if defined(_UNICODE) || defined(UNICODE)
				if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID)m_proxyUsername.c_str(), m_proxyUsername.size() * sizeof(TCHAR)))
					m_dwLastError = ::GetLastError();
#else
				pwszTmp = AnsiToUnicode(m_proxyUsername.c_str()); if (!pwszTmp) break; if (!pwszTmp) break;
				if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID)pwszTmp, m_proxyUsername.size() * sizeof(TCHAR))) {
                    free((void*)pwszTmp); pwszTmp = NULL;
					m_dwLastError = ::GetLastError();
				}
				free((void*)pwszTmp); pwszTmp = NULL;
#endif
                if (m_proxyPassword.size() > 0)
#if defined(_UNICODE) || defined(UNICODE)
					if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID)m_proxyPassword.c_str(), m_proxyPassword.size() * sizeof(TCHAR)))
						m_dwLastError = ::GetLastError();
#else
					pwszTmp = AnsiToUnicode(m_proxyPassword.c_str()); if (!pwszTmp) break;
					if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID)pwszTmp, m_proxyPassword.size() * sizeof(TCHAR))) {
						free((void*)pwszTmp); pwszTmp = NULL;
						m_dwLastError = ::GetLastError();
					}
					free((void*)pwszTmp); pwszTmp = NULL;
#endif
            }
        }

        if (disableAutoRedirect) {
            DWORD dwDisableFeature = WINHTTP_DISABLE_REDIRECTS;
            if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_DISABLE_FEATURE, &dwDisableFeature, sizeof(dwDisableFeature)))
                m_dwLastError = ::GetLastError();
        }
        bool bSendRequestSucceed = false;
        if (::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, NULL)) {
            bSendRequestSucceed = true;
        } else {
            // Query the proxy information from IE setting and set the proxy if any.
            WINHTTP_CURRENT_USER_IE_PROXY_CONFIG proxyConfig;
            memset(&proxyConfig, 0, sizeof(proxyConfig));
            if (::WinHttpGetIEProxyConfigForCurrentUser(&proxyConfig)) {
                if (proxyConfig.lpszAutoConfigUrl != NULL) {
                    WINHTTP_AUTOPROXY_OPTIONS autoProxyOptions;
                    memset(&autoProxyOptions, 0, sizeof(autoProxyOptions));
                    autoProxyOptions.dwFlags = WINHTTP_AUTOPROXY_AUTO_DETECT | WINHTTP_AUTOPROXY_CONFIG_URL;
                    autoProxyOptions.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP;
                    autoProxyOptions.lpszAutoConfigUrl = proxyConfig.lpszAutoConfigUrl;
                    autoProxyOptions.fAutoLogonIfChallenged = TRUE;
                    autoProxyOptions.dwReserved = 0;
                    autoProxyOptions.lpvReserved = NULL;

                    WINHTTP_PROXY_INFO proxyInfo;
                    memset(&proxyInfo, 0, sizeof(proxyInfo));

#if defined(_UNICODE) || defined(UNICODE)
					pwszTmp = m_requestURL.c_str();
#else
					pwszTmp = AnsiToUnicode(m_requestURL.c_str()); if (!pwszTmp) break;
#endif
					if (::WinHttpGetProxyForUrl(m_sessionHandle, pwszTmp, &autoProxyOptions, &proxyInfo)) {
                        if (::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo))) {
                            if (::WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, NULL))
                                bSendRequestSucceed = true;
                        }
                        if (proxyInfo.lpszProxy != NULL)
                            ::GlobalFree(proxyInfo.lpszProxy);
                        if (proxyInfo.lpszProxyBypass != NULL)
                            ::GlobalFree(proxyInfo.lpszProxyBypass);
						free((void*)pwszTmp); pwszTmp = NULL;
                    } else {
						free((void*)pwszTmp); pwszTmp = NULL;
                        m_dwLastError = ::GetLastError();
                    }
                } else if (proxyConfig.lpszProxy != NULL) {
                    WINHTTP_PROXY_INFO proxyInfo;
                    memset(&proxyInfo, 0, sizeof(proxyInfo));
                    proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
					wchar_t szProxy[MAX_PATH] = {0};
					proxyInfo.lpszProxy = szProxy;

                    if (proxyConfig.lpszProxyBypass != NULL) {
						wchar_t szProxyBypass[MAX_PATH] = {0};
                        wcscpy_s(szProxyBypass, MAX_PATH, proxyConfig.lpszProxyBypass);
                        proxyInfo.lpszProxyBypass = szProxyBypass;
                    }

                    if (!::WinHttpSetOption(hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
                        m_dwLastError = ::GetLastError();
                }

                if (proxyConfig.lpszAutoConfigUrl != NULL)
                    ::GlobalFree(proxyConfig.lpszAutoConfigUrl);
                if (proxyConfig.lpszProxy != NULL)
                    ::GlobalFree(proxyConfig.lpszProxy);
                if (proxyConfig.lpszProxyBypass != NULL)
                    ::GlobalFree(proxyConfig.lpszProxyBypass);
            } else {
                m_dwLastError = ::GetLastError();
            }
        }

        if (bSendRequestSucceed) {
            if (m_pDataToSend != NULL) {
                DWORD dwWritten = 0;
                if (!::WinHttpWriteData(hRequest, m_pDataToSend, m_dataToSendSize, &dwWritten))
                    m_dwLastError = ::GetLastError();
            }
            if (::WinHttpReceiveResponse(hRequest, NULL)) {
                DWORD dwSize = 0;
                BOOL bResult = FALSE;
                bResult = ::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
                if (bResult || (!bResult && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER))) {
                    wchar_t *szHeader = new wchar_t[dwSize];
                    if (szHeader != NULL) {
                        memset(szHeader, 0, dwSize* sizeof(wchar_t));
                        if (::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, szHeader, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
#if !defined(_UNICODE) && !defined(UNICODE)
							pszTmp = UnicodeToANSI(szHeader); if (!pszTmp) break;
                            m_responseHeader.assign(pszTmp);
							free((void*)pszTmp); pszTmp = NULL;
#endif
                            vector<tstring> result;
                            tstring regExp = _T("");
                            if (!m_bForceCharset) {
                                regExp = _T("charset={[A-Za-z0-9\\-_]+}");
                                if (ParseRegExp(regExp, false, 1, m_responseHeader, result) && result.size() > 0)
                                    m_responseCharset = result[0];
                            }
                            regExp = _T("Content-Length: {[0-9]+}");
                            if (ParseRegExp(regExp, false, 1, m_responseHeader, result) && result.size() > 0)
                                m_responseByteCount = (unsigned int)_ttoi(result[0].c_str());
                            regExp = _T("Location: {[0-9]+}");
                            if (ParseRegExp(regExp, false, 1, m_responseHeader, result) && result.size() > 0)
                                m_location = result[0];
                            regExp = _T("Set-Cookie:\\b*{.+?}\\n");
                            if (ParseRegExp(regExp, false, 1, m_responseHeader, result) && result.size() > 0) {
                                for (vector<tstring>::size_type i = 0; i < result.size(); i++) {
                                    m_responseCookies += result[i];
                                    if (i != result.size() - 1)
                                        m_responseCookies += _T("; ");
                                }
                                m_responseCookies = Trim(m_responseCookies, _T(" "));
                                if (m_responseCookies.size() > 0 && m_responseCookies[m_responseCookies.size() - 1] != _T(';'))
                                    m_responseCookies += _T(";");
                            }
                        }
                        delete[] szHeader;
                    }
                }
                            
                dwSize = 0;
                bResult = ::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);
                if (bResult || (!bResult && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER))) {
                    wchar_t *szStatusCode = new wchar_t[dwSize];
                    if (szStatusCode != NULL) {
                        memset(szStatusCode, 0, dwSize* sizeof(wchar_t));
                        if (::WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX, szStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX)) {
#if defined(_UNICODE) || defined(UNICODE)
							m_statusCode = szStatusCode;
#else
							pszTmp = UnicodeToANSI(szStatusCode); if (!pszTmp) break;
							m_statusCode = pszTmp;
							free((void*)pszTmp); pszTmp = NULL;
#endif
						}
                        delete[] szStatusCode;
                    }
                }

                unsigned int iMaxBufferSize = INT_BUFFERSIZE;
                unsigned int iCurrentBufferSize = 0;
                if (m_pResponse != NULL) {
                    delete[] m_pResponse;
                    m_pResponse = NULL;
                }
                m_pResponse = new BYTE[iMaxBufferSize];
                if (m_pResponse == NULL) {
                    bRetVal = false;
                    break;
                }
                memset(m_pResponse, 0, iMaxBufferSize);

                do {
                    dwSize = 0;
                    if (::WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                        SetProgress(iCurrentBufferSize);
                        BYTE *pResponse = new BYTE[dwSize + 1];
                        if (pResponse != NULL) {
                            memset(pResponse, 0, (dwSize + 1)*sizeof(BYTE));
                            DWORD dwRead = 0;
                            if (::WinHttpReadData(hRequest, pResponse, dwSize, &dwRead)) {
                                if (dwRead + iCurrentBufferSize > iMaxBufferSize) {
                                    BYTE *pOldBuffer = m_pResponse;
                                    m_pResponse = new BYTE[iMaxBufferSize * 2];
                                    if (m_pResponse == NULL) {
                                        m_pResponse = pOldBuffer;
                                        bRetVal = false;
                                        break;
                                    }
                                    iMaxBufferSize *= 2;
                                    memset(m_pResponse, 0, iMaxBufferSize);
                                    memcpy(m_pResponse, pOldBuffer, iCurrentBufferSize);
                                    delete[] pOldBuffer;
                                }
                                memcpy(m_pResponse + iCurrentBufferSize, pResponse, dwRead);
                                iCurrentBufferSize += dwRead;
                            }
                            delete[] pResponse;
                        }
                    } else {
                        m_dwLastError = ::GetLastError();
                    }
                } while (dwSize > 0);

                SetProgress(iCurrentBufferSize);
                m_responseByteCountReceived = iCurrentBufferSize;

                UINT codePage = CP_ACP;
                DWORD dwFlag = MB_PRECOMPOSED;
                if (_tcsnicmp(m_responseCharset.c_str(), _T("utf-8"), 5) == 0) {
                    codePage = CP_UTF8;
                    dwFlag = 0;
                }
#if defined(_UNICODE) || defined(UNICODE)
                int iLength = ::MultiByteToWideChar(codePage, dwFlag, (LPCSTR)m_pResponse, m_responseByteCountReceived + 1, NULL, 0);
                if (iLength <= 0) { // Use CP_ACP if UTF-8 fail
                    codePage = CP_ACP;
                    dwFlag = MB_PRECOMPOSED;
                    iLength = ::MultiByteToWideChar(codePage, dwFlag, (LPCSTR)m_pResponse, m_responseByteCountReceived + 1, NULL, 0);
                }
                if (iLength > 0) {
                    TCHAR *wideChar = new TCHAR[iLength];
                    if (wideChar != NULL) {
                        memset(wideChar, 0, iLength * sizeof(TCHAR));
                        iLength = ::MultiByteToWideChar(codePage, dwFlag, (LPCSTR)m_pResponse, m_responseByteCountReceived + 1, wideChar, iLength);
                        if (iLength > 0)
                            m_responseContent = wideChar;
                        delete[] wideChar;
                    }
                }
#else
				m_responseContent = (LPCSTR)m_pResponse;
#endif
                bGetReponseSucceed = true;

                // If the resposne html web page size is less than 200, retry.
                if (verb == _T("GET") && !disableAutoRedirect) {
                    tstring regExp = _T("{<html>}");
                    vector<tstring> result;
                    if (ParseRegExp(regExp, false, 1, m_responseContent, result) && result.size() > 0) {
                        regExp = _T("{</html>}");
                        if (!ParseRegExp(regExp, false, 1, m_responseContent, result) || result.size() <= 0) {
                            m_dwLastError = ERROR_INVALID_DATA;
                            bGetReponseSucceed = false;
                        }
                    }
                }
            } else {
                m_dwLastError = ::GetLastError();
            }
        }
    } // while

    if (!bGetReponseSucceed)
        bRetVal = false;

    ::WinHttpCloseHandle(hRequest);
    ::WinHttpCloseHandle(hConnect);

    return bRetVal;
}

tstring WinHttpClient::GetResponseHeader(void)
{
    return m_responseHeader;
}

tstring WinHttpClient::GetResponseContent(void)
{
    return m_responseContent;
}

tstring WinHttpClient::GetResponseCharset(void)
{
    return m_responseCharset;
}

tstring WinHttpClient::GetRequestHost(void)
{
    return m_requestHost;
}

bool WinHttpClient::SaveResponseToFile(const tstring &filePath)
{
    if (m_pResponse == NULL || m_responseByteCountReceived <= 0)
    {
        return false;
    }
    FILE *f = NULL;
    int iResult = _tfopen_s(&f, filePath.c_str(), _T("wb"));
    if (iResult == 0 && f != NULL)
    {
        fwrite(m_pResponse, m_responseByteCountReceived, 1, f);
        fclose(f);
        return true;
    }

    return false;
}

bool WinHttpClient::SetProgress(unsigned int byteCountReceived)
{
    bool bReturn = false;
    if (m_pfProcessProc != NULL && m_responseByteCount > 0)
    {
        double dProgress = (double)byteCountReceived * 100 / m_responseByteCount;
        m_pfProcessProc(dProgress);
        bReturn = true;
    }

    return bReturn;
}

tstring WinHttpClient::GetResponseCookies(void)
{
    return m_responseCookies;
}

bool WinHttpClient::SetAdditionalRequestCookies(const tstring &cookies)
{
    m_additionalRequestCookies = cookies;

    return true;
}

bool WinHttpClient::SetAdditionalDataToSend(BYTE *data, unsigned int dataSize)
{
    if (data == NULL || dataSize < 0)
    {
        return false;
    }

    if (m_pDataToSend != NULL)
    {
        delete[] m_pDataToSend;
    }
    m_pDataToSend = NULL;
    m_pDataToSend = new BYTE[dataSize];
    if (m_pDataToSend != NULL)
    {
        memcpy(m_pDataToSend, data, dataSize);
        m_dataToSendSize = dataSize;
        return true;
    }

    return false;
}

// Reset additional data fields
bool WinHttpClient::ResetAdditionalDataToSend(void)
{
    if (m_pDataToSend != NULL)
    {
        delete[] m_pDataToSend;
    }

    m_pDataToSend = NULL;
    m_dataToSendSize = 0;

    return true;
}

// Allow us to reset the url on subsequent requests
bool WinHttpClient::UpdateUrl(const tstring &url)
{
    m_requestURL = url;
    ResetAdditionalDataToSend();

    return true;
}

bool WinHttpClient::SetAdditionalRequestHeaders(const tstring &additionalRequestHeaders)
{
    m_additionalRequestHeaders = additionalRequestHeaders;

    return true;
}

bool WinHttpClient::SetProxy(const tstring &proxy)
{
    m_proxy = proxy;

    return true;
}

// If we don't require valid SSL Certs then accept any
// certificate on an SSL connection
bool WinHttpClient::SetRequireValidSslCertificates(bool require)
{
    m_requireValidSsl = require;

    return true;
}

const BYTE *WinHttpClient::GetRawResponseContent(void)
{
    return m_pResponse;
}

unsigned int WinHttpClient::GetRawResponseContentLength(void)
{
    return m_responseByteCount;
}

unsigned int WinHttpClient::GetRawResponseReceivedContentLength(void)
{
    return m_responseByteCountReceived;
}

DWORD WinHttpClient::GetLastError(void)
{
    return m_dwLastError;
}

tstring WinHttpClient::GetResponseStatusCode(void)
{
    return m_statusCode;
}

bool WinHttpClient::SetUserAgent(const tstring &userAgent)
{
    m_userAgent = userAgent;

    return true;
}

bool WinHttpClient::SetForceCharset(const tstring &charset)
{
    m_responseCharset = charset;

    return true;
}

bool WinHttpClient::SetProxyUsername(const tstring &username)
{
    m_proxyUsername = username;

    return true;
}

bool WinHttpClient::SetProxyPassword(const tstring &password)
{
    m_proxyPassword = password;

    return true;
}
    
tstring WinHttpClient::GetResponseLocation(void)
{
    return m_location;
}

bool WinHttpClient::SetTimeout(unsigned int resolveTimeout,unsigned int connectTimeout,unsigned int sendTimeout,unsigned int receiveTimeout)
{
    m_resolveTimeout = resolveTimeout;
    m_connectTimeout = connectTimeout;
    m_sendTimeout = sendTimeout;
    m_receiveTimeout = receiveTimeout;

    return true;
}

wchar_t* WinHttpClient::AnsiToUnicode(const char* str)
{
	wchar_t* result;
	int textlen = MultiByteToWideChar(CP_ACP,0,str,-1,NULL,0);
	result = (wchar_t*)malloc((textlen+1)*sizeof(wchar_t)); if (!result) return NULL;
	memset(result,0,(textlen+1)*sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP,0,str,-1,(LPWSTR)result,textlen);
	return result;
}

char* WinHttpClient::UnicodeToANSI(const wchar_t *str)
{
	char* result;
	int textlen = WideCharToMultiByte(CP_ACP,0,str,-1,NULL,0,NULL,NULL);
	result = (char*)malloc((textlen+1)*sizeof(char)); if (!result) return NULL;
	memset(result, 0, sizeof(char)*(textlen + 1));
	WideCharToMultiByte(CP_ACP,0,str,-1,result,textlen,NULL,NULL);
	return result;
}

tstring WinHttpClient::Trim(const tstring &source, const tstring &targets)
{
	tstring::size_type start = 0;
	tstring::size_type end = 0;
	for (start = 0; start < source.size(); start++)
	{
		bool bIsTarget = false;
		for (tstring::size_type i = 0; i < targets.size(); i++)
		{
			if (source[start] == targets[i])
			{
				bIsTarget = true;
				break;
			}
		}
		if (!bIsTarget)
		{
			break;
		}
	}
	for (end = source.size() - 1; (int)end >= 0; end--)
	{
		bool bIsTarget = false;
		for (tstring::size_type i = 0; i < targets.size(); i++)
		{
			if (source[end] == targets[i])
			{
				bIsTarget = true;
				break;
			}
		}
		if (!bIsTarget)
		{
			break;
		}
	}
	tstring result = _T("");
	if (end >= start && start < source.size() && end >= 0)
	{
		result = source.substr(start, end-start+1);
	}

	return result;
}

#endif // __WINHTTPCLIENT_H__
