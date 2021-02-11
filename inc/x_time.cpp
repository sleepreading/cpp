#include "x_time.h"
#include <assert.h>

#ifdef _DEBUG
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
Timer32::Timer32(const Timer32 &rst, int iTimeZone)
{
    assert(sizeof(Timer32) == sizeof(long));
    assert(sizeof(unsigned int) == sizeof(long));

    *this = rst;

    int iHour   = (int)m_uHour;
    iHour += iTimeZone;
    if (iHour >= 24) {
        NextDay();
        m_uHour = (unsigned int)(iHour - 24);
    } else if (iHour < 0) {
        PrevDay();
        m_uHour = (unsigned int)(iHour + 24);
    } else {
        m_uHour = (unsigned int)iHour;
    }
}

//////////////////////////////////////////////////////////////////////
unsigned int Timer32::GetWeekDay() const
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    int iYear   = (int)GetYear();
    int iMonth  = (int)GetMonth();
    int iDay    = (int)GetDay();

    if (iMonth >= 3) {
        iMonth -= 2;
    } else {
        iMonth += 10;
        -- iYear;
    }

    int iYearH  = iYear / 100;
    iYear %= 100;

    return (unsigned int)((((13 * iMonth - 1) / 5 + iDay + iYear + (iYear >> 2) + (iYearH >> 2) - iYearH - iYearH) % 7 + 7) % 7);
}

void Timer32::NextDay()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    unsigned int    uDaysOfMonth[12]    = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    unsigned int    uCurMonthDayCount   = uDaysOfMonth[m_uMonth - 1];

    unsigned int    uYear32             = GetYear();
    if (m_uMonth == 2U && ((uYear32 % 400U == 0U) || ((uYear32 % 100U != 0U) && (uYear32 % 4U == 0U)))) {
        ++ uCurMonthDayCount;
    }

    assert(m_uDay <= uCurMonthDayCount);

    if (m_uDay == uCurMonthDayCount) {
        m_uDay = 1U;
        if (m_uMonth == 12U) {
            ++ m_uYear;
            m_uMonth = 1U;
        } else {
            ++ m_uMonth;
        }
    } else {
        ++ m_uDay;
    }
}

void Timer32::PrevDay()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uDay == 1U) {
        if (m_uMonth == 1U) {
            -- m_uYear;
            m_uMonth = 12U;
            m_uDay = 31U;
        } else {
            -- m_uMonth;

            unsigned int    uDaysOfMonth[12]    = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
            unsigned int    uCurMonthDayCount   = uDaysOfMonth[m_uMonth - 1];
            unsigned int    uYear32             = GetYear();
            if (m_uMonth == 2U && ((uYear32 % 400U == 0U) || ((uYear32 % 100U != 0U) && (uYear32 % 4U == 0U)))) {
                ++ uCurMonthDayCount;
            }

            m_uDay = uCurMonthDayCount;
        }
    } else {
        -- m_uDay;
    }
}

void Timer32::NextSecond()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uSecond == 59U) {
        NextMinute();
        m_uSecond = 0;
    } else {
        ++ m_uSecond;
    }
}

void Timer32::PrevSecond()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uSecond == 0U) {
        PrevMinute();
        m_uSecond = 59U;
    } else {
        -- m_uSecond;
    }
}

void Timer32::NextMinute()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uMinute == 59U) {
        NextHour();
        m_uMinute = 0U;
    } else {
        ++ m_uMinute;
    }
}

void Timer32::PrevMinute()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uMinute == 0U) {
        PrevHour();
        m_uMinute = 59U;
    } else {
        -- m_uMinute;
    }
}

void Timer32::NextHour()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uHour == 23U) {
        NextDay();
        m_uHour = 0U;
    } else {
        ++ m_uHour;
    }
}

void Timer32::PrevHour()
{
    assert(m_uYear <= 63U && m_uYear >= 5U); // 2005 - 2063
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);

    if (m_uHour == 0U) {
        PrevDay();
        m_uHour = 23U;
    } else {
        -- m_uHour;
    }
}

bool Timer32::IsValid() const
{
    if (m_uMonth == 0U || m_uMonth > 12U || m_uDay == 0U || m_uDay > 31U || m_uHour > 23 || m_uMinute > 59 || m_uSecond > 59)
        return false;
    unsigned int    uDaysOfMonth[12]    = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    unsigned int    uCurMonthDayCount   = uDaysOfMonth[m_uMonth - 1U];
    if (m_uMonth == 2U && ((m_uYear % 400U == 0U) || ((m_uYear % 100U != 0U) && (m_uYear % 4U == 0U))))
        ++ uCurMonthDayCount;
    return (m_uDay <= uCurMonthDayCount);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
Timer64::Timer64()
{
    assert(sizeof(Timer64) == sizeof(__int64));
    *this = (__int64)0;
}

Timer64::Timer64(const __int64 &ri)
{
    assert(sizeof(Timer64) == sizeof(__int64));
    *this = ri;
}

Timer64::Timer64(const Timer32 &rt)
{
    assert(sizeof(Timer64) == sizeof(__int64));
    *this = rt;
}

Timer64::Timer64(const Timer64 &rt)
{
    assert(sizeof(Timer64) == sizeof(__int64));
    *this = rt;
}

Timer64::Timer64(const Timer64 &rst, int iTimeZone)
{
    assert(sizeof(Timer64) == sizeof(__int64));
    *this = rst;

    int iHour   = (int)m_uHour;
    iHour += iTimeZone;
    if (iHour >= 24) {
        NextDay();
        m_uHour = (BYTE)(iHour - 24);
    } else if (iHour < 0) {
        PrevDay();
        m_uHour = (BYTE)(iHour + 24);
    } else {
        m_uHour = (BYTE)iHour;
    }
}

Timer64::Timer64(unsigned int uY, unsigned int uMon, unsigned int uD) : m_uReserved(0U), m_uMilliseconds(0U), m_uSecond(0U), m_uMinute(0U), m_uHour(0U), m_uDay(uD), m_uMonth(uMon), m_uYear(uY)
{
    assert(sizeof(Timer64) == sizeof(__int64));
}

Timer64::Timer64(unsigned int uY, unsigned int uMon, unsigned int uD, unsigned int uH, unsigned int uMin, unsigned int uS, unsigned int uMs) : m_uReserved(0U), m_uMilliseconds(uMs), m_uSecond(uS), m_uMinute(uMin), m_uHour(uH), m_uDay(uD), m_uMonth(uMon), m_uYear(uY)
{
    assert(sizeof(Timer64) == sizeof(__int64));
}

Timer64 &Timer64::operator=(const Timer32 &rt)
{
    m_uReserved = 0U;
    m_uMilliseconds = 0U;
    m_uSecond = rt.GetSecond();
    m_uMinute = rt.GetMinute();
    m_uHour = rt.GetHour();
    m_uDay = rt.GetDay();
    m_uMonth = rt.GetMonth();
    m_uYear = rt.GetYear();
    return *this;
}

Timer64 &Timer64::operator=(const SYSTEMTIME &rst)
{
    m_uReserved = 0U;
    m_uMilliseconds = rst.wMilliseconds;
    m_uSecond = rst.wSecond;
    m_uMinute = rst.wMinute;
    m_uHour = rst.wHour;
    m_uDay = rst.wDay;
    m_uMonth = rst.wMonth;
    m_uYear = rst.wYear;
    return *this;
}

// 0, ÖÜÈÕ; 0-4095Äê
unsigned int Timer64::GetWeekDay() const
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    int iYear   = (int)GetYear();
    int iMonth  = (int)GetMonth();
    int iDay    = (int)GetDay();

    if (iMonth >= 3) {
        iMonth -= 2;
    } else {
        iMonth += 10;
        -- iYear;
    }

    int iYearH  = iYear / 100;
    iYear %= 100;

    return (unsigned int)((((13 * iMonth - 1) / 5 + iDay + iYear + (iYear >> 2) + (iYearH >> 2) - iYearH - iYearH) % 7 + 7) % 7);
}

void Timer64::NextDay()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    unsigned int    uDaysOfMonth[12]    = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    unsigned int    uCurMonthDayCount   = uDaysOfMonth[m_uMonth - 1];

    if (m_uMonth == 2U && ((m_uYear % 400U == 0U) || ((m_uYear % 100U != 0U) && (m_uYear % 4U == 0U)))) {
        ++ uCurMonthDayCount;
    }

    assert(m_uDay <= uCurMonthDayCount);

    if (m_uDay == uCurMonthDayCount) {
        m_uDay = 1U;
        if (m_uMonth == 12U) {
            ++ m_uYear;
            m_uMonth = 1U;
        } else {
            ++ m_uMonth;
        }
    } else {
        ++ m_uDay;
    }
}

void Timer64::PrevDay()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uDay == 1U) {
        if (m_uMonth == 1U) {
            -- m_uYear;
            m_uMonth = 12U;
            m_uDay = 31U;
        } else {
            -- m_uMonth;

            unsigned int    uDaysOfMonth[12]    = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
            unsigned int    uCurMonthDayCount   = uDaysOfMonth[m_uMonth - 1];
            if (m_uMonth == 2U && ((m_uYear % 400U == 0U) || ((m_uYear % 100U != 0U) && (m_uYear % 4U == 0U)))) {
                ++ uCurMonthDayCount;
            }

            m_uDay = uCurMonthDayCount;
        }
    } else {
        -- m_uDay;
    }
}

void Timer64::NextSecond()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uSecond == 59U) {
        NextMinute();
        m_uSecond = 0;
    } else {
        ++ m_uSecond;
    }
}

void Timer64::PrevSecond()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uSecond == 0U) {
        PrevMinute();
        m_uSecond = 59U;
    } else {
        -- m_uSecond;
    }
}

void Timer64::NextMinute()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uMinute == 59U) {
        NextHour();
        m_uMinute = 0U;
    } else {
        ++ m_uMinute;
    }
}

void Timer64::PrevMinute()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uMinute == 0U) {
        PrevHour();
        m_uMinute = 59U;
    } else {
        -- m_uMinute;
    }
}

void Timer64::NextHour()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uHour == 23U) {
        NextDay();
        m_uHour = 0U;
    } else {
        ++ m_uHour;
    }
}

void Timer64::PrevHour()
{
    assert(m_uMonth <= 12U && m_uMonth >= 1U);
    assert(m_uDay <= 31U && m_uDay >= 1U);
    assert(m_uHour <= 23U);
    assert(m_uMinute <= 59U);
    assert(m_uSecond <= 59U);
    assert(m_uMilliseconds <= 999U);

    if (m_uHour == 0U) {
        PrevDay();
        m_uHour = 23U;
    } else {
        -- m_uHour;
    }
}

bool Timer64::IsValid() const
{
    if (m_uMonth == 0U || m_uMonth > 12U || m_uDay == 0U || m_uDay > 31U || m_uHour > 23U || m_uMinute > 59U || m_uSecond > 59U || m_uMilliseconds > 999U) {
        return false;
    }

    unsigned int    uDaysOfMonth[12]    = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    unsigned int    uCurMonthDayCount   = uDaysOfMonth[m_uMonth - 1U];
    if (m_uMonth == 2U && ((m_uYear % 400U == 0U) || ((m_uYear % 100U != 0U) && (m_uYear % 4U == 0U)))) {
        ++ uCurMonthDayCount;
    }
    return (m_uDay <= uCurMonthDayCount);
}
