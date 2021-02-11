#ifndef __X_TIME_H__
#define __X_TIME_H__

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#pragma pack(push, 1)

class Timer32 //2063年
{
    unsigned int m_uSecond  : 6;
    unsigned int m_uMinute  : 6;
    unsigned int m_uHour    : 5;
    unsigned int m_uDay     : 5;
    unsigned int m_uMonth   : 4;
    unsigned int m_uYear    : 6;

public:
    Timer32()
    {
        *this = 0U;
    }
    Timer32(const unsigned int &ru)
    {
        *this = ru;
    }
    Timer32(const Timer32 &ru)
    {
        *this = ru;
    }
    Timer32(const Timer32 &rst, int iTimeZone);  // 根据格林威治时间 rst 及时区 iTimeZone 构造 CTime32
    Timer32(unsigned int uY, unsigned int uMon, unsigned int uD) :
        m_uSecond(0), m_uMinute(0), m_uHour(0),
        m_uDay(uD), m_uMonth(uMon), m_uYear(uY - 2000U) {}
    Timer32(unsigned int uY, unsigned int uMon, unsigned int uD, unsigned int uH, unsigned int uMin, unsigned int uS) :
        m_uSecond(uS), m_uMinute(uMin), m_uHour(uH),
        m_uDay(uD), m_uMonth(uMon), m_uYear(uY - 2000U) {}
    ~Timer32() {};

    operator unsigned int() const
    {
        return *((unsigned int *)this);
    };

    Timer32 &operator = (const Timer32 &rt)
    {
        (*((unsigned int *)this)) = (*((unsigned int *)&rt));
        return *this;
    };
    Timer32 &operator = (const unsigned int &ru)
    {
        (*((unsigned int *)this)) = ru;
        return *this;
    };
    Timer32 &operator = (const SYSTEMTIME &rst)
    {
        m_uYear = rst.wYear - 2000U;
        m_uMonth = rst.wMonth;
        m_uDay = rst.wDay;
        m_uHour = rst.wHour;
        m_uMinute = rst.wMinute;
        m_uSecond = rst.wSecond;
        return *this;
    }

    bool operator == (const Timer32 &rt) const
    {
        return (*((unsigned int *)this)) == (*((unsigned int *)&rt));
    };
    bool operator == (const unsigned int ut) const
    {
        return (*((unsigned int *)this)) == ut;
    };
    bool operator != (const Timer32 &rt) const
    {
        return (*((unsigned int *)this)) != (*((unsigned int *)&rt));
    };
    bool operator != (const unsigned int ut) const
    {
        return (*((unsigned int *)this)) != ut;
    };

    bool operator > (const Timer32 &rt) const
    {
        return (*((unsigned int *)this)) > (*((unsigned int *)&rt));
    };
    bool operator < (const Timer32 &rt) const
    {
        return (*((unsigned int *)this)) < (*((unsigned int *)&rt));
    };
    bool operator >= (const Timer32 &rt) const
    {
        return (*((unsigned int *)this)) >= (*((unsigned int *)&rt));
    };
    bool operator <= (const Timer32 &rt) const
    {
        return (*((unsigned int *)this)) <= (*((unsigned int *)&rt));
    };

    void GetSystemTime()
    {
        SYSTEMTIME st;
        ::GetSystemTime(&st);
        *this = st;
    };
    void GetLocalTime()
    {
        SYSTEMTIME lt;
        ::GetLocalTime(&lt);
        *this = lt;
    };

    unsigned int GetWeekDay() const; // 0, 周日; 1, 周一; ...; 6, 周六
    bool IsWeekend() const
    {
        unsigned int uWeekDay = GetWeekDay();
        return (uWeekDay == 0U || uWeekDay == 6U);
    };
    bool IsSameDay(Timer32 &rt) const
    {
        return ((*((unsigned int *)this)) & 0xFFFE0000U) == ((*((unsigned int *)&rt)) & 0xFFFE0000U);
    };

    void NextDay();     // 后一天
    void PrevDay();     // 前一天
    void NextHour();    // 后一小时
    void PrevHour();    // 前一小时
    void NextSecond();  // 后一秒
    void PrevSecond();  // 前一秒
    void NextMinute();  // 后一分
    void PrevMinute();  // 前一分

    unsigned int GetYear() const
    {
        return m_uYear + 2000U;
    };
    unsigned int GetMonth() const
    {
        return m_uMonth;
    };
    unsigned int GetDay() const
    {
        return m_uDay;
    };
    unsigned int GetHour() const
    {
        return m_uHour;
    };
    unsigned int GetMinute() const
    {
        return m_uMinute;
    };
    unsigned int GetSecond() const
    {
        return m_uSecond;
    };

    void SetYear(unsigned int u)
    {
        if (u >= 2000U && u < 2064U) m_uYear = u - 2000U;
    };
    void SetMonth(unsigned int u)
    {
        m_uMonth = u;
    };
    void SetDay(unsigned int u)
    {
        m_uDay = u;
    };
    void SetHour(unsigned int u)
    {
        m_uHour = u;
    };
    void SetMinute(unsigned int u)
    {
        m_uMinute = u;
    };
    void SetSecond(unsigned int u)
    {
        m_uSecond = u;
    };

    bool IsValid() const;
    bool IsLeapYear() const
    {
        return ((GetYear() % 400U == 0U) || ((GetYear() % 100U != 0U) && (GetYear() % 4U == 0U)));
    };
};

class Timer64
{
    unsigned int m_uReserved    : 16;     // must be zero.
    unsigned int m_uMilliseconds: 10;
    unsigned int m_uSecond      : 6;
    unsigned int m_uMinute      : 6;
    unsigned int m_uHour        : 5;
    unsigned int m_uDay         : 5;
    unsigned int m_uMonth       : 4;
    unsigned int m_uYear        : 12;

public:
    Timer64();
    Timer64(const __int64 &ri);
    Timer64(const Timer32 &rt);
    Timer64(const Timer64 &rt);
    Timer64(const Timer64 &rst, int iTimeZone);  // 根据格林威治时间 rst 及时区 iTimeZone 构造 Time64
    Timer64(unsigned int uY, unsigned int uMon, unsigned int uD);
    Timer64(unsigned int uY, unsigned int uMon, unsigned int uD, unsigned int uH, unsigned int uMin, unsigned int uS, unsigned int uMs = 0U);
    ~Timer64() {};

    operator __int64() const
    {
        return *((__int64 *)this);
    };

    Timer64 &operator = (const Timer32 &rt);
    Timer64 &operator = (const Timer64 &rt)
    {
        (*((__int64 *)this)) = (*((__int64 *)&rt));
        return *this;
    };
    Timer64 &operator = (const __int64 &ri)
    {
        (*((__int64 *)this)) = ri;
        return *this;
    };
    Timer64 &operator = (const SYSTEMTIME &rst);

    bool operator == (const Timer64 &rt) const
    {
        return (*((__int64 *)this)) == (*((__int64 *)&rt));
    };
    bool operator == (const __int64 &i) const
    {
        return (*((__int64 *)this)) == i;
    };
    bool operator != (const Timer64 &rt) const
    {
        return (*((__int64 *)this)) != (*((__int64 *)&rt));
    };
    bool operator != (const __int64 &i) const
    {
        return (*((__int64 *)this)) != i;
    };

    bool operator > (const Timer64 &rt) const
    {
        return (*((__int64 *)this)) > (*((__int64 *)&rt));
    };
    bool operator < (const Timer64 &rt) const
    {
        return (*((__int64 *)this)) < (*((__int64 *)&rt));
    };
    bool operator >= (const Timer64 &rt) const
    {
        return (*((__int64 *)this)) >= (*((__int64 *)&rt));
    };
    bool operator <= (const Timer64 &rt) const
    {
        return (*((__int64 *)this)) <= (*((__int64 *)&rt));
    };

    void GetSystemTime()
    {
        SYSTEMTIME st;
        ::GetSystemTime(&st);
        *this = st;
    };
    void GetLocalTime()
    {
        SYSTEMTIME lt;
        ::GetLocalTime(&lt);
        *this = lt;
    };

    unsigned int GetWeekDay() const; // 0, 周日; 1, 周一; ...; 6, 周六
    bool IsWeekend() const
    {
        unsigned int uWeekDay = GetWeekDay();
        return (uWeekDay == 0U || uWeekDay == 6U);
    };
    bool IsSameDay(Timer64 &rt) const
    {
        return (GetDay() == rt.GetDay() && GetMonth() == rt.GetMonth() && GetYear() == rt.GetYear());
    };

    void NextDay();     // 后一天
    void PrevDay();     // 前一天
    void NextHour();    // 后一小时
    void PrevHour();    // 前一小时
    void NextSecond();  // 后一秒
    void PrevSecond();  // 前一秒
    void NextMinute();  // 后一分
    void PrevMinute();  // 前一分

    unsigned int GetYear() const
    {
        return m_uYear;
    };
    unsigned int GetMonth() const
    {
        return m_uMonth;
    };
    unsigned int GetDay() const
    {
        return m_uDay;
    };
    unsigned int GetHour() const
    {
        return m_uHour;
    };
    unsigned int GetMinute() const
    {
        return m_uMinute;
    };
    unsigned int GetSecond() const
    {
        return m_uSecond;
    };
    unsigned int GetMilliseconds() const
    {
        return m_uMilliseconds;
    };

    void SetYear(unsigned int u)
    {
        m_uYear = u;
    };
    void SetMonth(unsigned int u)
    {
        m_uMonth = u;
    };
    void SetDay(unsigned int u)
    {
        m_uDay = u;
    };
    void SetHour(unsigned int u)
    {
        m_uHour = u;
    };
    void SetMinute(unsigned int u)
    {
        m_uMinute = u;
    };
    void SetSecond(unsigned int u)
    {
        m_uSecond = u;
    };
    void SetMilliseconds(unsigned int u)
    {
        m_uMilliseconds = u;
    };

    bool IsValid() const;
    bool IsLeapYear() const
    {
        return ((GetYear() % 400U == 0U) || ((GetYear() % 100U != 0U) && (GetYear() % 4U == 0U)));
    };
};

#pragma pack(pop)

#endif

