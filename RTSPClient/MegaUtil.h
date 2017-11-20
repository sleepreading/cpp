#pragma once
#include <string>
#include <vector>
#include <map>
#include <deque>
class CSplitVector : public std::vector< std::string >
{
public:
    CSplitVector(const char *s, const char *sp)
    {
        std::string sTemp;
        for (long i = 0; s[i] != 0; i++) {
            if (0 == strncmp(s + i, sp, strlen(sp))) {
                if (sTemp.size() > 0)
                    push_back(sTemp);
                i += (long)strlen(sp) - 1;
                sTemp = "";
                continue;
            }
            sTemp += s[i];
        }

        if (sTemp != "")
            push_back(sTemp);
    }
};

class CSplitMap : public std::map< std::string, std::string >
{
public:
    CSplitMap(const char *s, const char *sp1, const char *sp2)
    {
        std::string sKey, sValue;
        bool bKey = true;
        long lNameIndex = 0;
        for (long i = 0; s[i] != 0; i++) {
            if (bKey && 0 == strncmp(s + i, sp1, strlen(sp1))) {
                i += (long)strlen(sp1) - 1;
                bKey = false;
                continue;
            } else if (0 == strncmp(s + i, sp2, strlen(sp2))) {
                i += (long)strlen(sp2) - 1;
                if (bKey) {
                    if (lNameIndex == 0) {
                        (*this)[ "Name" ] = sKey;
                    } else {
                        char sTemp[512] = {0};
#if _MSC_VER > 1300
                        sprintf_s(sTemp, 511, "Name%d", lNameIndex);
#else
                        sprintf(sTemp, "Name%d", lNameIndex);
#endif
                        (*this)[ sTemp ] = sKey;
                    }
                    lNameIndex++;
                } else {
                    (*this)[ sKey ] = sValue;
                }

                sKey = "";
                sValue = "";
                bKey = true;
                continue;
            }

            if (bKey)
                sKey += s[i];
            else
                sValue += s[i];
        }

        if (sKey != "") {
            if (bKey) {
                if (lNameIndex == 0) {
                    (*this)[ "Name" ] = sKey;
                } else {
                    char sTemp[512] = {0};
#if _MSC_VER > 1300
                    sprintf_s(sTemp, 511, "Name%d", lNameIndex);
#else
                    sprintf(sTemp, "Name%d", lNameIndex);
#endif
                    (*this)[ sTemp ] = sKey;
                }
                lNameIndex++;
            } else {
                (*this)[ sKey ] = sValue;
            }
        }
    }
};

class CSplitDeque : public std::deque< std::pair<std::string, std::string > >
{
public:
    CSplitDeque(const char *s, const char *sp1, const char *sp2)
    {
        std::string sKey, sValue;
        bool bKey = true;
        long lNameIndex = 0;
        for (long i = 0; s[i] != 0; i++) {
            if (bKey && 0 == strncmp(s + i, sp1, strlen(sp1))) {
                i += (long)strlen(sp1) - 1;
                bKey = false;
                continue;
            } else if (0 == strncmp(s + i, sp2, strlen(sp2))) {
                i += (long)strlen(sp2) - 1;
                if (bKey) {
                    if (lNameIndex == 0) {
                        push_back(std::make_pair("Name", sKey));
                    } else {
                        char sTemp[512] = {0};
#if _MSC_VER > 1300
                        sprintf_s(sTemp, 511, "Name%d", lNameIndex);
#else
                        sprintf(sTemp, "Name%d", lNameIndex);
#endif
                        push_back(std::make_pair(sTemp, sKey));
                    }
                    lNameIndex++;
                } else {
                    push_back(std::make_pair(sKey, sValue));
                }

                sKey = "";
                sValue = "";
                bKey = true;
                continue;
            }

            if (bKey)
                sKey += s[i];
            else
                sValue += s[i];
        }

        if (sKey != "") {
            if (bKey) {
                if (lNameIndex == 0) {
                    push_back(std::make_pair("Name", sKey));
                } else {
                    char sTemp[512] = {0};
#if _MSC_VER > 1300
                    sprintf_s(sTemp, 511, "Name%d", lNameIndex);
#else
                    sprintf(sTemp, "Name%d", lNameIndex);
#endif
                    push_back(std::make_pair(sTemp, sKey));
                }
                lNameIndex++;
            } else {
                push_back(std::make_pair(sKey, sValue));
            }
        }
    }

    std::deque< std::pair<std::string, std::string > >::iterator find(const char *sKey)
    {
        std::deque< std::pair<std::string, std::string > >::iterator pos;
        for (pos = begin(); pos != end(); pos++) {
            if (pos->first == sKey)
                return pos;
        }
        return end();
    }

    std::deque< std::pair<std::string, std::string > >::iterator find(const char *sKey, std::deque< std::pair<std::string, std::string > >::iterator pos)
    {
        for (; pos != end(); pos++) {
            if (pos->first == sKey)
                return pos;
        }
        return end();
    }
};

// naming://platformid:accessip:[ID1:]ID2:type
// ID:[PID:]AccessIP:PlatformID
// Naming=ID:[PID:]AccessIP:PlatformID;type=
class CNaming
{
    bool m_bValid;
    std::string m_sID, m_sPID, m_sAccessIP, m_sPlatformID, m_sType;
public:
    CNaming(const char *sNamingUrl_Or_FromTo) : m_bValid(false)
    {
        if (0 == strncmp(sNamingUrl_Or_FromTo, "Naming=", strlen("Naming="))) {
            CSplitMap spm(sNamingUrl_Or_FromTo, "=", ";");
            if (spm.size() != 2)
                throw "sNamingUrl_Or_FromTo格式不正确";

            CSplitMap::iterator pos = spm.begin();
            CSplitVector spv(pos->second.c_str(), "-");
            if (spv.size() == 3) {
                m_sID = spv[0];
                m_sAccessIP = spv[1];
                m_sPlatformID = spv[2];
            } else if (spv.size() == 4) {
                m_sID = spv[0];
                m_sPID = spv[1];
                m_sAccessIP = spv[2];
                m_sPlatformID = spv[3];
            } else {
                throw "sNamingUrl_Or_FromTo格式不正确";
            }
            pos++;
            m_sType = pos->second;
            m_bValid = true;
        } else if (0 == strncmp(sNamingUrl_Or_FromTo, "Naming://", strlen("Naming://"))) {
            CSplitVector spv(sNamingUrl_Or_FromTo, ":");
            if (spv.size() == 4) {
                m_sID = spv[2];
                m_sType = spv[3];
            } else if (spv.size() == 5) {
                m_sPID = spv[2];
                m_sID = spv[3];
                m_sType = spv[4];
            } else {
                throw "sNamingUrl格式不正确";
            }
            m_sPlatformID = spv[0];
            m_sAccessIP = spv[1];
            m_bValid = true;
        } else {
            throw "sNamingUrl格式不正确";
        }
    }
    CNaming(const char *sNaming, const char *sType) : m_bValid(false)
    {
        CSplitVector spv(sNaming, ":");
        if (spv.size() == 3) {
            m_sID = spv[0];
            m_sAccessIP = spv[1];
            m_sPlatformID = spv[2];
        } else if (spv.size() == 4) {
            m_sID = spv[0];
            m_sPID = spv[1];
            m_sAccessIP = spv[2];
            m_sPlatformID = spv[3];
        } else {
            throw "sNamingUrl格式不正确";
        }
        m_sType = sType;
        m_bValid = true;
    }
    std::string GetUrl()
    {
        return std::string("naming://") + m_sPlatformID + ":" + m_sAccessIP
               + ":" + (m_sPID != "" ? m_sPID + ":" : "") + m_sID + ":" + m_sType;
    }
    std::string GetFromTo()
    {
        return std::string("Naming=") + m_sID + "-" + (m_sPID != "" ? m_sPID + "-" : "") + m_sAccessIP + "-" + m_sPlatformID + ";type=" + m_sType;
    }
    std::string GetNaming()
    {
        return m_sID + ":" + (m_sPID != "" ? m_sPID + ":" : "") + m_sAccessIP + ":" + m_sPlatformID;
    }
    std::string GetType()
    {
        return m_sType;
    }
    std::string GetID()
    {
        return m_sID;
    }
    std::string GetIDAndType()
    {
        return std::string("ID=") + m_sID + "&Type=" + m_sType;
    }
    std::string GetPlatformID()
    {
        return m_sPlatformID;
    }
};

inline std::string TrimBlank(const char *s, bool bLeft = true, bool bRight = true)
{
    std::string sR;

    const char *sTrimText = " \t\r\n";

    if (bLeft) {
        while (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n')
            s++;
    }

    if (bRight) {
        for (; *s != '\0' && *s != ' ' && *s != '\t' && *s != '\r' && *s != '\n'; s++)
            sR += *s;

        return sR;
    } else
        return s;
}

inline std::pair< std::string, long> GetIPAndPortFromUrl(const char *sUrl)
{
    const char *sStart = strstr(sUrl, "://");
    if (sStart == NULL)
        return std::make_pair("", 0);

    std::string sIP;
    const char *sTemp = sStart + strlen("://");
    while (*sTemp != 0 && *sTemp != '?' && *sTemp != '/' && *sTemp != ':')
        sIP += *sTemp++;

    sStart += strlen("://");
    sStart = strstr(sStart, ":");
    if (sStart == NULL)
        return std::make_pair(sIP, 0);

    long lPort = 0;
    sStart += strlen(":");
    for (int i = 0; sStart[i] != '/' && sStart[i] != 0; i++)
        lPort = 10 * lPort + sStart[i] - '0';
    return std::make_pair(sIP, lPort);
}

inline bool IsAccessUrl(const char *sUrl)
{
    return (0 == strncmp(sUrl, "megaeyes://Method=", strlen("megaeyes://Method="))
            || 0 == strncmp(sUrl, "megaeyes://Answer=", strlen("megaeyes://Answer=")));
}

inline bool IsCenterUrl(const char *sUrl)
{
    return (!IsAccessUrl(sUrl) && 0 == strncmp(sUrl, "megaeyes://", strlen("megaeyes://")));
}

inline bool IsHttpUrl(const char *sUrl)
{
    return (0 == strncmp(sUrl, "http://", strlen("http://")));
}