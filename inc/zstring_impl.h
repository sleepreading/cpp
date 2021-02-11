#include <algorithm>
#include <functional>   // bind
#include <locale>
#include <ctype.h>      // isalpha
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>     // va_arg
#if defined(_WIN32) || defined(__BORLANDC__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

// When using VC, turn off browser references & unavoidable compiler warnings
#if defined(_MSC_VER)
#pragma component(browser, off, references, "String")
#pragma warning (disable : 4290) // C++ Exception Specification ignored
#pragma warning (disable : 4127) // Conditional expression is constant
#pragma warning (disable : 4097) // typedef name used as synonym for class name
#pragma warning (disable : 4996 4786)
#elif defined(__BORLANDC__)
#pragma option push -w-inl  // Turn off inline function warnings
#endif


//////////////////////////////////////////////////////////////////////////
// global functions

// bind cann't use a template function as his parameter, so we define one
// global function such as ::isspace not support wchar_t&char at the same time, so we do define these shits!

//#if __cplusplus >= 201703L
template <class Arg, class Result>
struct munary_function
{
    typedef Arg argument_type;
    typedef Result result_type;
};
template <class Arg1, class Arg2, class Result>
struct mbinary_function
{
    typedef Arg1 first_argument_type;
    typedef Arg2 second_argument_type;
    typedef Result result_type;
};
//#endif // __cplusplus >= 201703L

template<typename T>
struct ssnotspace : public munary_function<T, bool> {
    bool operator()(T t) const { return !std::isspace(t, std::locale()); }
};
template<typename T>
struct ssisspace : public munary_function<T, bool> {
    bool operator()(T t) const { return std::isspace(t, std::locale()); }
};
template<typename T>
struct ssisdigit : public munary_function<T, bool> {
    bool operator()(T t) const { return std::isdigit(t, std::locale()); }
};
template<typename T>
struct ssisalpha : public munary_function<T, bool> {
    bool operator()(T t) const { return std::isalpha(t, std::locale()); }
};
template<typename T>
struct ssislower : public munary_function<T, bool> {
    bool operator()(T t) const { return std::islower(t, std::locale()); }
};
template<typename T>
struct ssisupper : public munary_function<T, bool> {
    bool operator()(T t) const { return std::isupper(t, std::locale()); }
};
template<typename T>
struct sstoupper : public mbinary_function<T, std::locale, T> {
    inline T operator()(const T& t, const std::locale& loc) const { return std::toupper<T>(t, loc); }
};
template<typename T>
struct sstolower : public mbinary_function<T, std::locale, T> {
    inline T operator()(const T& t, const std::locale& loc) const { return std::tolower<T>(t, loc); }
};

// -----------------------------------------------------------------------------
// StdCodeCvt - made to look like Win32 functions WideCharToMultiByte
// -----------------------------------------------------------------------------
inline wchar_t* StdCodeCvt(wchar_t* pDstW, size_t nDst, const char* pSrcA, size_t nSrc, const std::locale &loc=std::locale())
{
    if (!pSrcA || !pDstW) return 0;

    pDstW[0]                    = '\0';
    if (nSrc > 0) {
        const char* pNextSrcA   = pSrcA;
        wchar_t* pNextDstW      = pDstW;
#if _MSC_VER < 1600
        typedef std::codecvt<wchar_t, char, mbstate_t> SSCodeCvt;
        const SSCodeCvt &conv    = std::_USE(loc, SSCodeCvt);
#else
        const std::codecvt<wchar_t, char, mbstate_t> &conv    = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc);
#endif
        std::codecvt<wchar_t, char, mbstate_t>::state_type st = { 0 };
        std::codecvt<wchar_t, char, mbstate_t>::result res    = conv.in(st, pSrcA, pSrcA + nSrc, pNextSrcA, pDstW, pDstW + nDst, pNextDstW);
        //ASSERT(SSCodeCvt::ok == res);
        //ASSERT(SSCodeCvt::error != res);
        //ASSERT(pNextDstW >= pDstW);
        //ASSERT(pNextSrcA >= pSrcA);

        if (pNextDstW - pDstW > (long)nDst) *(pDstW + nDst) = '\0';
        else *pNextDstW = '\0';
    }
    return pDstW;
}
inline char* StdCodeCvt(char* pDstA, size_t nDst, const wchar_t* pSrcW, size_t nSrc, const std::locale &loc=std::locale())
{
    if (!pSrcW || !pDstA) return 0;

    pDstA[0]                     = '\0';
    if (nSrc > 0) {
        char* pNextDstA          = pDstA;
        const wchar_t* pNextSrcW = pSrcW;
#if _MSC_VER < 1600
        typedef std::codecvt<wchar_t, char, mbstate_t> SSCodeCvt;
        const SSCodeCvt &conv    = std::_USE(loc, SSCodeCvt);
#else
        const std::codecvt<wchar_t, char, mbstate_t> &conv    = std::use_facet<std::codecvt<wchar_t, char, mbstate_t>>(loc);
#endif
        std::codecvt<wchar_t, char, mbstate_t>::state_type st = { 0 };
        std::codecvt<wchar_t, char, mbstate_t>::result res    = conv.out(st, pSrcW, pSrcW + nSrc, pNextSrcW, pDstA, pDstA + nDst, pNextDstA);

        if (pNextDstA - pDstA > (long)nDst) *(pDstA + nDst) = '\0';
        else *pNextDstA = '\0';
    }
    return pDstA;
}

// -----------------------------------------------------------------------------
// vsnprintf : GNU is supposed to have vsnprintf and vsnwprintf.  But only the newer distributions do.
// -----------------------------------------------------------------------------
inline int  ssvsprintf(char* pA, size_t nCount, const char* pFmtA, va_list vl)
{
#ifdef _WIN32
    return _vsnprintf(pA, nCount, pFmtA, vl);
#else
    return vsnprintf(pA, nCount, pFmtA, vl);
#endif
}
inline int  ssvsprintf(wchar_t* pW, size_t nCount, const wchar_t* pFmtW, va_list vl)
{
#ifdef _WIN32
    return _vsnwprintf(pW, nCount, pFmtW, vl);
#else
    return vsnwprintf(pW, nCount, pFmtW, vl);  // vswprintf
#endif
}

// -----------------------------------------------------------------------------
// assign
// -----------------------------------------------------------------------------
inline void ssassign(std::string &sDst, const std::string &sSrc)
{
    if (sDst.c_str() != sSrc.c_str()) {
        sDst.erase();
        sDst.assign(sSrc);
    }
}
inline void ssassign(std::wstring &sDst, const std::wstring &sSrc)
{
    if (sDst.c_str() != sSrc.c_str()) {
        sDst.erase();
        sDst.assign(sSrc);
    }
}
inline void ssassign(std::string &sDst, const char* pA)
{
    // Watch out for NULLs, as always.
    if (0 == pA) {
        sDst.erase();

    // If pA actually points to part of sDst, we must NOT erase(), but rather take a substring
    } else if (pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.size()) {
        sDst = sDst.substr(static_cast<std::string::size_type>(pA - sDst.c_str()));

    // Otherwise (most cases) apply the assignment bug fix, if applicable and do the assignment
    } else {
        sDst.assign(pA);
    }
}
inline void ssassign(std::string &sDst, const std::wstring &sSrc)
{
    if (sSrc.empty()) {
        sDst.erase();
    } else {
        int nDst = static_cast<int>(sSrc.size());

        // In MBCS builds, pad the buffer to account for the possibility of some 3 byte characters.  Not perfect but should get most cases.
        // For other platforms you may define _MBCS manually!. The only effect it currently has is to cause the allocation of more space for wchar_t --> char conversions.

#ifdef _MBCS
        nDst = static_cast<int>(static_cast<double>(nDst) * 1.3);
#endif
        sDst.resize(nDst + 1);
        const char* szCvt = StdCodeCvt(const_cast<std::string::pointer>(sDst.data()), nDst, sSrc.c_str(), static_cast<int>(sSrc.size()));

        // In MBCS builds, we don't know how long the destination string will be.
#ifdef _MBCS
        sDst.resize(strlen(szCvt));
#else
        szCvt;
        sDst.resize(sSrc.size());
#endif
    }
}
inline void ssassign(std::string &sDst, const wchar_t* pW)
{
    size_t nSrc = wcslen(pW);
    if (nSrc > 0) {
        size_t nDst = nSrc;

        // In MBCS builds, pad the buffer to account for the possibility of
        // some 3 byte characters.  Not perfect but should get most cases.
#ifdef _MBCS
        nDst = static_cast<int>(static_cast<double>(nDst) * 1.3);
#endif
        sDst.resize(nDst + 1);
        const char* szCvt = StdCodeCvt(const_cast<std::string::pointer>(sDst.data()), nDst, pW, nSrc);

        // In MBCS builds, we don't know how long the destination string will be.
#ifdef _MBCS
        sDst.resize(strlen(szCvt));
#else
        sDst.resize(nDst);
        szCvt;
#endif
    } else {
        sDst.erase();
    }
}
inline void ssassign(std::wstring &sDst, const wchar_t* pW)
{
    // Watch out for NULLs, as always.
    if (0 == pW) {
        sDst.erase();

    // If pW actually points to part of sDst, we must NOT erase(), but rather take a substring
    } else if (pW >= sDst.c_str() && pW <= sDst.c_str() + sDst.size()) {
        sDst = sDst.substr(static_cast<std::wstring::size_type>(pW - sDst.c_str()));

    // Otherwise (most cases) apply the assignment bug fix, if applicable and do the assignment
    } else {
        sDst.assign(pW);
    }
}
inline void ssassign(std::wstring &sDst, const std::string &sSrc)
{
    if (sSrc.empty()) {
        sDst.erase();
    } else {
        int nSrc = static_cast<int>(sSrc.size());
        int nDst = nSrc;
        sDst.resize(nSrc + 1);
        const wchar_t* szCvt = StdCodeCvt(const_cast<std::wstring::pointer>(sDst.data()), nDst, sSrc.c_str(), nSrc);
        sDst.resize(wcslen(szCvt));
    }
}
inline void ssassign(std::wstring &sDst, const char* pA)
{
    size_t nSrc = strlen(pA);
    if (0 == nSrc) {
        sDst.erase();
    } else {
        size_t nDst = nSrc;
        sDst.resize(nDst + 1);
        const wchar_t* szCvt = StdCodeCvt(const_cast<std::wstring::pointer>(sDst.data()), nDst, pA, nSrc);
        sDst.resize(wcslen(szCvt));
    }
}

// -----------------------------------------------------------------------------
// ssadd: string object concatenation -- add second argument to first
// -----------------------------------------------------------------------------
inline void ssadd(std::string &sDst, const std::wstring &sSrc)
{
    int nSrc = static_cast<int>(sSrc.size());
    if (nSrc > 0) {
        int nDst = static_cast<int>(sDst.size());
        int nAdd = nSrc;

        // In MBCS builds, pad the buffer to account for the possibility of
        // some 3 byte characters.  Not perfect but should get most cases.

#ifdef _MBCS
        nAdd = static_cast<int>(static_cast<double>(nAdd) * 1.3);
#endif

        sDst.resize(nDst + nAdd + 1);
        const char* szCvt = StdCodeCvt(const_cast<std::string::pointer>(sDst.data() + nDst), nAdd, sSrc.c_str(), nSrc);

#ifdef _MBCS
        sDst.resize(nDst + strlen(szCvt));
#else
        sDst.resize(nDst + nAdd);
        szCvt;
#endif
    }
}
inline void ssadd(std::string &sDst, const wchar_t* pW)
{
    size_t nSrc = wcslen(pW);
    if (nSrc > 0) {
        size_t nDst = static_cast<size_t>(sDst.size());
        size_t nAdd = nSrc;

#ifdef _MBCS
        nAdd = static_cast<size_t>(static_cast<double>(nAdd) * 1.3);
#endif

        sDst.resize(nDst + nAdd + 1);
        const char* szCvt = StdCodeCvt(const_cast<std::string::pointer>(sDst.data() + nDst), nAdd, pW, nSrc);

#ifdef _MBCS
        sDst.resize(nDst + strlen(szCvt));
#else
        sDst.resize(nDst + nSrc);
        szCvt;
#endif
    }
}
inline void ssadd(std::string &sDst, const char* pA)
{
    if (pA) {
        // If the string being added is our internal string or a part of our
        // internal string, then we must NOT do any reallocation without
        // first copying that string to another object (since we're using a
        // direct pointer)
        if (pA >= sDst.c_str() && pA <= sDst.c_str() + sDst.length()) {
            if (sDst.capacity() <= sDst.size() + strlen(pA))
                sDst.append(std::string(pA));
            else
                sDst.append(pA);
        } else {
            sDst.append(pA);
        }
    }
}
inline void ssadd(std::string &sDst, const std::string &sSrc)
{
    sDst += sSrc;
}
inline void ssadd(std::wstring &sDst, const std::wstring &sSrc)
{
    sDst += sSrc;
}
inline void ssadd(std::wstring &sDst, const std::string &sSrc)
{
    if (!sSrc.empty()) {
        int nSrc = static_cast<int>(sSrc.size());
        int nDst = static_cast<int>(sDst.size());
        sDst.resize(nDst + nSrc + 1);
        const wchar_t* szCvt = StdCodeCvt(const_cast<std::wstring::pointer>(sDst.data() + nDst), nSrc, sSrc.c_str(), nSrc + 1);
#ifdef _MBCS
        sDst.resize(nDst + wcslen(szCvt));
#else
        sDst.resize(nDst + nSrc);
        szCvt;
#endif
    }
}
inline void ssadd(std::wstring &sDst, const char* pA)
{
    size_t nSrc = strlen(pA);
    if (nSrc > 0) {
        size_t nDst = static_cast<size_t>(sDst.size());
        sDst.resize(nDst + nSrc + 1);
        const wchar_t* szCvt = StdCodeCvt(const_cast<std::wstring::pointer>(sDst.data() + nDst), nSrc, pA, nSrc + 1);
#ifdef _MBCS
        sDst.resize(nDst + wcslen(szCvt));
#else
        sDst.resize(nDst + nSrc);
        szCvt;
#endif
    }
}
inline void ssadd(std::wstring &sDst, const wchar_t* pW)
{
    if (pW) {
        // If the string being added is our internal string or a part of our
        // internal string, then we must NOT do any reallocation without
        // first copying that string to another object (since we're using a
        // direct pointer)

        if (pW >= sDst.c_str() && pW <= sDst.c_str() + sDst.length()) {
            if (sDst.capacity() <= sDst.size() + wcslen(pW))
                sDst.append(std::wstring(pW));
            else
                sDst.append(pW);
        } else {
            sDst.append(pW);
        }
    }
}


//////////////////////////////////////////////////////////////////////////
// TString Template

template<typename T>
TString<T>::TString(const char* pA)
{
    _try_load(pA);
}

template<typename T>
TString<T>::TString(const wchar_t* pW)
{
    _try_load(pW);
}

template<typename T>
TString<T>::TString(const std::string &str)
{
    ssassign(*this, str);
}

template<typename T>
TString<T>::TString(const std::wstring &str)
{
    ssassign(*this, str);
}

template<typename T>
TString<T>& TString<T>::operator=(const TString<T> &str)
{
    ssassign(*this, str); return *this;
}

template<typename T>
TString<T>& TString<T>::operator=(const std::string &str)
{
    ssassign(*this, str); return *this;
}

template<typename T>
TString<T>& TString<T>::operator=(const std::wstring &str)
{
    ssassign(*this, str); return *this;
}

template<typename T>
TString<T>& TString<T>::operator=(const char* pA)
{
    ssassign(*this, pA);  return *this;
}

template<typename T>
TString<T>& TString<T>::operator=(const wchar_t* pW)
{
    ssassign(*this, pW);  return *this;
}

template<typename T>
TString<T>& TString<T>::operator=(T t)
{
    return assign(1, t);
}

template<typename T>
TString<T>& TString<T>::operator+=(const TString<T> &str)
{
    ssadd(*this, str);    return *this;
}

template<typename T>
TString<T>& TString<T>::operator+=(const std::string &str)
{
    ssadd(*this, str);    return *this;
}

template<typename T>
TString<T>& TString<T>::operator+=(const std::wstring &str)
{
    ssadd(*this, str);    return *this;
}

template<typename T>
TString<T>& TString<T>::operator+=(const char* pA)
{
    ssadd(*this, pA);     return *this;
}

template<typename T>
TString<T>& TString<T>::operator+=(const wchar_t* pW)
{
    ssadd(*this, pW);     return *this;
}

template<typename T>
TString<T>& TString<T>::operator+=(T t)
{
    return append(1, t);
}

#if _MSC_VER < 1600
char* ssntoa(__int64 value, char* result, int base)
#else
char* ssntoa(long long value, char* result, int base)
#endif
{
    if (base < 2 || base > 36 || !result) return result;
    char* ptr = result, *ptr1 = result, tmp_char;
    #if _MSC_VER < 1600
    __int64 tmp_value;
    #else
    long long tmp_value;
    #endif

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while (value);

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--   = *ptr1;
        *ptr1++  = tmp_char;
    }
    return result;
}
template<typename T>
TString<T> TString<T>::from_number(int64 v, int radix/*=10*/)
{
    TString<T> s;
    char buf[520] = {0};
    ssntoa(v, buf, radix);
    s = buf;
    return s;
}
template<typename T>
TString<T> TString<T>::from_double(double v, int ndigits/*=2*/)
{
    TString<T> s;
    char buf[520] = {0};
    gcvt(v, ndigits, buf);
    s = buf;  // operator = will convert it for me
    return s;
}

template<typename T>
bool TString<T>::begwith(const_pointer szThat, bool ignorecase/*=false*/, const_pointer ignore_begchars/*=0*/) const
{
    // if this string is empty whatever you search return false
    // if the search string szThat is empty, reaturn false, this feature is just like string::find_first_of()
    size_type len = !szThat ? 0 : std::basic_string<T>::traits_type::length(szThat);
    if (this->empty() || !len) return false;

    // exclude the 'ignored begin chars', this string may be less than szThat, in that case the substr() call crash
    size_type pos = !ignore_begchars ? 0 : this->find_first_not_of(ignore_begchars);
    if (pos<this->size() && pos+len <= this->size()) {  // here this->size() means this->end(). note len>0
        return this->substr(pos, len).compare2(szThat, ignorecase) == 0;
    }

    return false;
}

template<typename T>
bool TString<T>::endwith(const_pointer szThat, bool ignorecase/*=false*/, const_pointer ignore_endchars/*=0*/) const
{
    size_type len = !szThat ? 0 : std::basic_string<T>::traits_type::length(szThat);
    if (this->empty() || !len) return false;

    size_type pos = !ignore_endchars ? this->size()-1 : this->find_last_not_of(ignore_endchars);  // otherwise last position
    if (pos<this->size() && pos>=len) {
        return this->substr(pos-len, len).compare(szThat, ignorecase)==0;
    }

    return false;
}

template<typename T>
int TString<T>::compare2(const_pointer szThat, bool ignorecase/*=false*/) const
{
    if (!szThat) {
        return 1;
    }
    size_type thatlen = std::basic_string<T>::traits_type::length(szThat);
    size_type thislen = this->size();
    if (thatlen < 1) {
        return thislen < 1 ? 0 : 1;
    }
    if (thislen < 1) {
        return thatlen < 1 ? 0 : -1;
    }

    if (!ignorecase) {
        return this->compare(szThat);
    }
    // Using the "C" locale = "not affected by locale"
    std::locale loc = std::locale::classic();
#if _MSC_VER < 1600
    const std::ctype<T> &ct = std::_USE(loc, std::ctype<T>);
#else
    const std::ctype<T> &ct = std::use_facet<std::ctype<T>>(loc);
#endif
    T f;
    T l;
    const_pointer szThis = this->c_str();
    do {
        f = ct.tolower(*(szThis++));
        l = ct.tolower(*(szThat++));
    } while ((f) && (f == l));

    return (int)(f - l);
}

/*
-1	The first sequence is less than the second (i.e., the first goes before the second in alphabetical order).
0	Both sequences are considered equal.
1	The first sequence is greater than the second (i.e., the first goes after the second in alphabetical order).
*/
template<typename T>
int TString<T>::collate(const_pointer szThat, bool ignorecase/*=false*/) const
{
    if (!szThat) {
        return 1;
    }
    size_type thatlen = std::basic_string<T>::traits_type::length(szThat);
    size_type thislen = this->size();
    if (thatlen < 1) {
        return thislen < 1 ? 0 : 1;
    }
    if (thislen < 1) {
        return thatlen < 1 ? 0 : -1;
    }

    if (ignorecase) {
        const std::locale loc;  // 和程序当前的全局 locale 属性相同
    #if _MSC_VER < 1600
        const std::collate<T> &coll = std::_USE(loc, std::collate<T>);
    #else
        const std::collate<T> &coll = std::use_facet<std::collate<T>>(loc);
    #endif
        // Some implementations seem to have trouble using the collate<> facet typedefs so we'll just default to basic_string and hope that's what the collate facet uses (which it generally should)
        // std::collate<T>::string_type s1(this->c_str());
        // std::collate<T>::string_type s2(szThat);
        const std::basic_string<T> sEmpty;
        std::basic_string<T> s1(this->c_str() ? this->c_str() : sEmpty.c_str());
        std::basic_string<T> s2(szThat);
    #if _MSC_VER < 1600
        std::_USE(loc, std::ctype<T>).tolower(const_cast<T*>(s1.c_str()), const_cast<T*>(s1.c_str()) + nLen1);
        std::_USE(loc, std::ctype<T>).tolower(const_cast<T*>(s2.c_str()), const_cast<T*>(s2.c_str()) + nLen2);
    #else
        std::use_facet<std::ctype<T>>(loc).tolower(const_cast<T*>(s1.c_str()), const_cast<T*>(s1.c_str()) + thislen);
        std::use_facet<std::ctype<T>>(loc).tolower(const_cast<T*>(s2.c_str()), const_cast<T*>(s2.c_str()) + thatlen);
    #endif
        return coll.compare(s1.c_str(), s1.c_str() + thislen, s2.c_str(), s2.c_str() + thatlen);

    } else {
    #if _MSC_VER < 1600
        const std::collate<T> &coll = std::_USE(std::locale(), std::collate<T>);
    #else
        const std::collate<T> &coll = std::use_facet<std::collate<T>>(std::locale());
    #endif
        return coll.compare(this->c_str(), this->c_str() + this->size(), szThat, szThat + thatlen);
    }
}

template<typename T>
std::vector< TString<T> > TString<T>::spliter(const_pointer delim) const
{
    std::vector< TString<T> > out;  if (!delim) return out;

    size_type b = 0, i = this->find_first_of(delim, b);
    while (i != std::string::npos) {
        out.push_back(this->substr(b, i-b));
        b = i + 1;
        i = this->find_first_of(delim, b);
    }
    if (i - b > 0) {  // the last element before separator
        ret.push_back(this->substr(b));
    }
    return out;
}

template<typename T>
TString<T>& TString<T>::upper(const std::locale &loc/*=std::locale()*/)
{
    // Note -- if there are any MBCS character sets in which the lowercase
    // form a character takes up a different number of bytes than the uppercase form, this would probably not work...
    // < c++11: std::bind2nd(sstoupper<T>(), loc));
	std::transform(this->begin(), this->end(), this->begin(), std::bind(sstoupper<T>(), std::placeholders::_1, loc));
    return *this;
}
template<typename T>
TString<T> TString<T>::upper_copy(const std::locale &loc/*=std::locale()*/) const
{
    TString<T> s(*this);
	std::transform(s.begin(), s.end(), s.begin(), std::bind(sstoupper<T>(), std::placeholders::_1, loc));
    return s;
}

template<typename T>
TString<T>& TString<T>::lower(const std::locale &loc/*=std::locale()*/)
{
	std::transform(this->begin(), this->end(), this->begin(), std::bind(sstolower<T>(), std::placeholders::_1, loc));
    return *this;
}
template<typename T>
TString<T> TString<T>::lower_copy(const std::locale &loc/*=std::locale()*/) const
{
    TString<T> s(*this);
	std::transform(s.begin(), s.end(), s.begin(), std::bind(sstolower<T>(), std::placeholders::_1, loc));
    return s;
}

template<typename T>
TString<T>& TString<T>::remove(const_pointer delim)
{
    size_type len = !delim ? 0 : std::basic_string<T>::traits_type::length(delim); if (0 == len) return *this;
    for (size_type i=0; i<len; ++i) {
        this->erase(std::remove(this->begin(), this->end(), delim[i]), this->end());
    }
    return *this;
}
template<typename T>
TString<T> TString<T>::remove_copy(const_pointer delim) const
{
    TString<T> s(*this);
    size_type len = !delim ? 0 : std::basic_string<T>::traits_type::length(delim); if (0 == len) return *this;
    for (size_type i=0; i<len; ++i) {
        s.erase(std::remove(s.begin(), s.end(), delim[i]), s.end());
    }
    return s;
}

// 标准的std::string::replace()最少是3个参数,主要用来替换(或删除)前2个参数所标识的(位置,个数);
// 本函数用来替换(或删除)所有匹配的子串!
// 单个字符的全部替换: std::replace(ForwardIterator first, ForwardIterator last,const T& old_value, const T& new_value).
template<typename T>
TString<T>& TString<T>::replace2(const_pointer szOld, const_pointer szNew/*为0时表示删除*/)
{
    size_type nIdx    = 0;
    size_type nOldLen = !szOld ? 0 : std::basic_string<T>::traits_type::length(szOld); if (0 == nOldLen) return *this;
    size_type nNewLen = !szNew ? 0 : std::basic_string<T>::traits_type::length(szNew);

    // expand if replace string is larger
    if (nNewLen > nOldLen) {
        int nFound = 0;
        while (nIdx < this->length() && (nIdx = this->find(szOld, nIdx)) != std::basic_string<T>::npos) {
            nFound++;
            nIdx += nOldLen;
        }
        this->reserve(this->size() + nFound * (nNewLen - nOldLen));
    }
    // do replace
    static const T ch = T(0);
    const_pointer szRealNew = szNew == 0 ? &ch : szNew;  // 0 means delete
    nIdx = 0;
    while (nIdx < this->length() && (nIdx = this->find(szOld, nIdx)) != std::basic_string<T>::npos) {
        this->replace(this->begin() + nIdx, this->begin() + nIdx + nOldLen, szRealNew);
        nIdx += nNewLen;
    }

    return *this;
}
template<typename T>
TString<T> TString<T>::replace_copy(const_pointer szOld, const_pointer szNew) const
{
    size_type nIdx    = 0;
    size_type nOldLen = !szOld ? 0 : std::basic_string<T>::traits_type::length(szOld); if (0 == nOldLen) return *this;
    size_type nNewLen = !szNew ? 0 : std::basic_string<T>::traits_type::length(szNew);
    TString<T> s(*this);

    // expand if replace string is larger
    if (nNewLen > nOldLen) {
        int nFound = 0;
        while (nIdx < s.length() && (nIdx = s.find(szOld, nIdx)) != std::basic_string<T>::npos) {
            nFound++;
            nIdx += nOldLen;
        }
        s.reserve(s.size() + nFound * (nNewLen - nOldLen));
    }
    // do replace
    static const T ch = T(0);
    const_pointer szRealNew = szNew == 0 ? &ch : szNew;  // 0 means delete
    nIdx = 0;
    while (nIdx < s.length() && (nIdx = s.find(szOld, nIdx)) != std::basic_string<T>::npos) {
        s.replace(s.begin() + nIdx, s.begin() + nIdx + nOldLen, szRealNew);
        nIdx += nNewLen;
    }

    return s;
}

template<typename T>
TString<T>& TString<T>::reverse()
{
    std::reverse(this->begin(), this->end());
    return *this;
}
template<typename T>
TString<T> TString<T>::reverse_copy() const
{
    TString<T> s(*this);
    std::reverse(s.begin(), s.end());
    return s;
}

template<typename T>
TString<T>& TString<T>::sort()
{
    std::sort(this->begin(), this->end());
    return *this;
}
template<typename T>
TString<T> TString<T>::sort_copy() const
{
    TString<T> s(*this);
    std::sort(s.begin(), s.end());
    return s;
}

// sort --> delete repeat elements
template<typename T>
TString<T>& TString<T>::unique()
{
    s.erase(std::unique(this->begin(), this->end()), this->end());
    return *this;
}
template<typename T>
TString<T> TString<T>::unique_copy() const
{
    TString<T> s(*this);
    s.erase(std::unique(s.begin(), s.end()), s.end());
    return s;
}

template<typename T>
TString<T>& TString<T>::trim(const_pointer szTrimChars/*=0*/)
{
    return triml(szTrimChars).trimr(szTrimChars);
}
template<typename T>
TString<T> TString<T>::trim_copy(const_pointer szTrimChars/*=0*/) const
{
    TString<T> s(*this);
    return s.triml(szTrimChars).trimr(szTrimChars);
}

template<typename T>
TString<T>& TString<T>::triml(const_pointer szTrimChars/*=0*/)
{
    if (!szTrimChars || !std::basic_string<T>::traits_type::length(szTrimChars)) {
	#if _MSC_VER < 1600
        this->erase(this->begin(), std::find_if(this->begin(), this->end(), ssnotspace<T>()));  // find_if_not C++11
	#else
        this->erase(this->begin(), std::find_if_not(this->begin(), this->end(), ssisspace<T>()));
	#endif
    } else {
        this->erase(0, this->find_first_not_of(szTrimChars));
    }
    return *this;
}
template<typename T>
TString<T> TString<T>::triml_copy(const_pointer szTrimChars/*=0*/) const
{
    TString<T> s(*this);
    if (!szTrimChars || !std::basic_string<T>::traits_type::length(szTrimChars)) {
    #if _MSC_VER < 1600
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), ssnotspace<T>()));  // find_if_not C++11
	#else
        s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), ssisspace<T>()));
	#endif
    } else {
        s.erase(0, s.find_first_not_of(szTrimChars));
    }
    return s;
}

template<typename T>
TString<T>& TString<T>::trimr(const_pointer szTrimChars/*=0*/)
{
    if (!szTrimChars || !std::basic_string<T>::traits_type::length(szTrimChars)) {
        std::basic_string<T>::reverse_iterator it = std::find_if(this->rbegin(), this->rend(), ssnotspace<T>());
        if (this->rend() != it) { // global operator!= already defined for reverse_iterator in the header <utility>.
            this->erase(it.base());
        }
        this->erase(!(it == this->rend()) ? this->find_last_of(*it) + 1 : 0);

    } else {
        size_type nIdx = this->find_last_not_of(szTrimChars);
        this->erase(std::basic_string<T>::npos == nIdx ? 0 : ++nIdx);
    }
    return *this;
}
template<typename T>
TString<T> TString<T>::trimr_copy(const_pointer szTrimChars/*=0*/) const
{
    TString<T> s(*this);
    if (!szTrimChars || !std::basic_string<T>::traits_type::length(szTrimChars)) {
        std::basic_string<T>::reverse_iterator it = std::find_if(s.rbegin(), s.rend(), ssnotspace<T>());
        if (!(s.rend() == it))  // global operator!= already defined for reverse_iterator in the header <utility>.
            s.erase(this->rend() - it);
        s.erase(!(it == s.rend()) ? s.find_last_of(*it) + 1 : 0);

    } else {
        size_type nIdx = s.find_last_not_of(szTrimChars);
        s.erase(std::basic_string<T>::npos == nIdx ? 0 : ++nIdx);
    }
    return s;
}

template<typename T>
bool TString<T>::isdigit() const
{
#if _MSC_VER < 1600
	return this->size() == std::count_if(this->begin(), this->end(), ssisdigit<T>());
#else
    return std::all_of(this->begin(), this->end(), ssisdigit<T>()); // C++11
#endif
}

template<typename T>
bool TString<T>::isalpha() const
{
#if _MSC_VER < 1600
	return this->size() == std::count_if(this->begin(), this->end(), ssisalpha<T>());
#else
    return std::all_of(this->begin(), this->end(), ssisalpha<T>());
#endif
}

template<typename T>
bool TString<T>::isspace() const
{
#if _MSC_VER < 1600
	return this->size() == std::count_if(this->begin(), this->end(), ssisspace<T>());
#else
    return std::all_of(this->begin(), this->end(), ssisspace<T>());
#endif
}

template<typename T>
bool TString<T>::islower() const
{
#if _MSC_VER < 1600
	return this->size() == std::count_if(this->begin(), this->end(), ssislower<T>());
#else
    return std::all_of(this->begin(), this->end(), ssislower<T>());
#endif
}

template<typename T>
bool TString<T>::isupper() const
{
#if _MSC_VER < 1600
	return this->size() == std::count_if(this->begin(), this->end(), ssisupper<T>());
#else
    return std::all_of(this->begin(), this->end(), ssisupper<T>());
#endif
}

// use typeid will introduce metadata to object, su we use template specialization
// can't use String's isdigit, because leading space can also be valued
// because std::stol can cause exception, so we do not use it!
template<typename T>
long TString<T>::to_long() const
{
    TString<char> t = this->triml_copy();  // ssassign will convert it for me
    int radix = 10;
    if (t.begwith("0x", true, "+-")) {
        radix = 16;
    } else if (t.begwith("0o", true, "+-")) {
        radix = 8;
    } else if (t.begwith("0b", true, "+-")) {
        radix = 2;
    }
    return ::strtol(t.c_str(), 0, radix);
}

// 这几个函数都很灵活,会忽略前导空格以及后续的无法转换字符
template<typename T>
unsigned long TString<T>::to_ulong() const
{
    TString<char> t = this->triml_copy();
    int radix = 10;
    if (t.begwith("0x", true, "+-")) {
        radix = 16;
    } else if (t.begwith("0o", true, "+-")) {
        radix = 8;
    } else if (t.begwith("0b", true, "+-")) {
        radix = 2;
    }
    return ::strtoul(t.c_str(), 0, radix);
}
template<typename T>
int TString<T>::to_int() const
{
    return (int)this->to_long();
}
template<typename T>
unsigned int TString<T>::to_uint() const
{
    return (unsigned int)this->to_ulong();
}

template<typename T>
double TString<T>::to_double() const
{
    TString<char> t = this->triml_copy();
    return ::strtod(t.c_str(), 0);
}
template<typename T>
float TString<T>::to_float() const
{
    return (float)this->to_double();
}

template<typename T>
int64 TString<T>::to_longlong() const
{
    TString<char> t = this->triml_copy();
    int radix = 10;
    if (t.begwith("0x", true, "+-")) {
        radix = 16;
    } else if (t.begwith("0o", true, "+-")) {
        radix = 8;
    } else if (t.begwith("0b", true, "+-")) {
        radix = 2;
    }
#if _MSC_VER < 1600  // below vs2010, only supports decimal convert
    return ::_atoi64(t.c_str());
#else
	return ::_strtoi64(t.c_str(), 0, radix);  // return std::stoll(t, 0, radix);
#endif
}
template<typename T>
uint64 TString<T>::to_ulonglong() const
{
    TString<char> t = this->triml_copy();
    int radix = 10;
    if (t.begwith("0x", true, "+-")) {
        radix = 16;
    } else if (t.begwith("0o", true, "+-")) {
        radix = 8;
    } else if (t.begwith("0b", true, "+-")) {
        radix = 2;
    }
#if _MSC_VER < 1600  // !!! bug !!!
    return (uint64)::_atoi64(t.c_str());
#else
    return ::_strtoui64(t.c_str(), 0, radix);  // std::stoull(t, 0, radix);
#endif
}

#ifdef  _WIN32
template<typename T>
bool TString<T>::load(unsigned int nId)
{
    bool loaded = false;   // set to true of we succeed.
    static HMODULE hModule = GetModuleHandle(0);

    #if defined(_UNICODE) || defined(UNICODE)
    const wchar_t* szName = MAKEINTRESOURCE((nId >> 4) + 1); // lifted
    #else
    const char*    szName = MAKEINTRESOURCE((nId >> 4) + 1); // lifted
    #endif

    unsigned int dwSize = 0;
    HRSRC hrsrc = ::FindResource(hModule, szName, RT_STRING);
    if (0 == hrsrc) { // No sense continuing if we can't find the resource
        //TRACE(_T("Cannot find resource %d: 0x%X"), nId, ::GetLastError());
    } else if (0 == (dwSize = ::SizeofResource(hModule, hrsrc) / sizeof(T))) {
        //TRACE(_T("Cant get size of resource %d 0x%X\n"), nId, GetLastError());
    } else {
    #if defined(_UNICODE) || defined(UNICODE)
        loaded = 0 != ::LoadString(hModule, nId, (wchar_t*)GetBuffer(dwSize), dwSize);
    #else
        loaded = 0 != ::LoadString(hModule, nId, (char*)GetBuffer(dwSize), dwSize);
    #endif
        ReleaseBuffer();
    }

    //if (!bLoaded)
    //TRACE(_T("String not loaded 0x%X\n"), ::GetLastError());
    return loaded;
}

template<typename T>
TString<T>& TString<T>::format(unsigned int nId, ...)
{
    va_list argList;
    va_start(argList, nId);
    TString<T> szFmt;
    if (szFmt.load(nId)) {
        //format_v(szFmt, argList);
        T* szBuf = new T[0x800000];  // 8M
        if (szBuf) {
            int nLen = ssvsprintf(szBuf, 0x800000 - 1, szFmt, argList);
            if (0 < nLen) {
                *this = szBuf;
            }
            delete[] szBuf;
        }
    }
    va_end(argList);
    return *this;
}

template<typename T>
TString<T> TString<T>::get_last_errmsg()
{
    TString<char> s;
    char szBuf[520] = "Unknown error :(";
    if (0 != ::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, szBuf, 511, 0)) {
        s = szBuf;
        s.trimr("\r\n");  // attention: windows break line form
    }
    return s;
}

#endif  // _WIN32

template<typename T>
TString<T>& TString<T>::format(const T *szFmt, ...)
{
    va_list argList;
    va_start(argList, szFmt);
    T* szBuf = new T[0x800000];  // 8M
    if (szBuf) {
        int nLen = ssvsprintf(szBuf, 0x800000 - 1, szFmt, argList);
        if (0 < nLen) {
            *this = szBuf;
        }
        delete[] szBuf;
    }
    va_end(argList);
    return *this;
}
template<typename T>
TString<T> TString<T>::from_format(const T *szFmt, ...)
{
    TString<T> s;
    va_list argList;
    va_start(argList, szFmt);
    T* szBuf = new T[0x800000];  // 8M
    if (szBuf) {
        int nLen = ssvsprintf(szBuf, 0x800000 - 1, szFmt, argList);
        if (0 < nLen) {
            s = szBuf;
        }
        delete[] szBuf;
    }
    va_end(argList);
    return s;
}

template<typename T>
TString<T>& TString<T>::format_append(const T *szFmt, ...)
{
    va_list argList;
    va_start(argList, szFmt);

    T* szBuf = new T[0x800000];  // 8M
    if (szBuf) {
        int nLen = ssvsprintf(szBuf, 0x800000 - 1, szFmt, argList);
        if (0 < nLen) {
            this->append(szBuf, nLen);
        }
        delete[] szBuf;
    }

    va_end(argList);
    return *this;
}

template<typename T>
void TString<T>::free_extra()
{
    TString<T> mt;
    this->swap(mt);
    if (!mt.empty())
        this->assign(mt.c_str(), mt.size());
}

template<typename T>
T* TString<T>::GetBuffer(int nMinLen /*= -1*/)
{
    if (static_cast<int>(this->size()) < nMinLen)
        this->resize(static_cast<size_type>(nMinLen));
    return this->empty() ? const_cast<T *>(this->data()) : &(this->at(0));
}

template<typename T>
void TString<T>::ReleaseBuffer(int nNewLen /*= -1*/)
{
    this->resize(static_cast<size_type>(nNewLen > -1 ? nNewLen : std::basic_string<T>::traits_type::length(this->c_str())));
}

// shorthand conversion from const TCHAR* to string resource ID
template<typename T>
void TString<T>::_try_load(const void *pT)
{
#ifdef  _WIN32
    #ifndef IS_INTRESOURCE
    #ifdef _WIN64
        #define IS_INTRESOURCE(_r) (((unsigned __int64)(_r) >> 16) == 0)
    #else
        #define IS_INTRESOURCE(_r) (((unsigned long)(_r) >> 16) == 0)
    #endif
    #endif
    if (pT && IS_INTRESOURCE(pT)) {
        load(LOWORD(reinterpret_cast<unsigned long>(pT)));
    } else {
    #if defined(_UNICODE) || defined(UNICODE)
        *this = (const wchar_t*)pT;
    #else
        *this = (const char*)pT;
    #endif
    }
#else
    *this = pW;
#endif
}  // TString


//////////////////////////////////////////////////////////////////////////
// global operators

StringA operator+(const StringA &s1, const StringA &s2)
{
    StringA sRet(s1);
    sRet.append(s2);
    return sRet;
}

StringA operator+(const StringA &s1, StringA::value_type t)
{
    StringA sRet(s1);
    sRet.append(1, t);
    return sRet;
}

StringA operator+(const StringA &s1, const char* pA)
{
    StringA sRet(s1);
    sRet.append(pA);
    return sRet;
}

StringA operator+(const char* pA, const StringA &sA)
{
    StringA sRet;
    StringA::size_type nObjSize = sA.size();
    StringA::size_type nLitSize = static_cast<StringA::size_type>(strlen(pA));

    sRet.reserve(nLitSize + nObjSize);
    sRet.assign(pA);
    sRet.append(sA);
    return sRet;
}

StringA operator+(const StringA &s1, const StringW &s2)
{
    return s1 + StringA(s2);
}

StringW operator+(const StringW &s1, const StringW &s2)
{
    StringW sRet(s1);
    sRet.append(s2);
    return sRet;
}

StringA operator+(const StringA &s1, const wchar_t* pW)
{
    return s1 + StringA(pW);
}

#if defined(_UNICODE) || defined(UNICODE)  // 类型是一样的,返回值不同.故要区分下
StringW operator+(const wchar_t* pW, const StringA &sA)
{
    return StringW(pW) + StringW(sA);
}

StringW operator+(const char* pA, const StringW &sW)
{
    return StringW(pA) + sW;
}
#else
StringA operator+(const wchar_t* pW, const StringA &sA)
{
    return StringA(pW) + sA;
}

StringA operator+(const char* pA, const StringW &sW)
{
    return pA + StringA(sW);
}
#endif  // _UNICODE

StringW operator+(const StringW &s1, StringW::value_type t)
{
    StringW sRet(s1);
    sRet.append(1, t);
    return sRet;
}

StringW operator+(const StringW &s1, const wchar_t* pW)
{
    StringW sRet(s1);
    sRet.append(pW);
    return sRet;
}

StringW operator+(const wchar_t* pW, const StringW &sW)
{
    StringW sRet;
    StringW::size_type nObjSize = sW.size();
    StringA::size_type nLitSize = static_cast<StringW::size_type>(wcslen(pW));

    sRet.reserve(nLitSize + nObjSize);
    sRet.assign(pW);
    sRet.append(sW);
    return sRet;
}

StringW operator+(const StringW &s1, const StringA &s2)
{
    return s1 + StringW(s2);
}

StringW operator+(const StringW &s1, const char* pA)
{
    return s1 + StringW(pA);
}

//////////////////////////////////////////////////////////////////////////
//  format
template<typename T>
TString<T> sformat_v(const T *szFmt, va_list argList)
{
    TString<T> r;
    T* szBuf = new T[0x800000];  // 8M
    if (szBuf) {
        int nLen = ssvsprintf(szBuf, 0x800000 - 1, szFmt, argList);
        if (0 < nLen) {
            r = szBuf;
        }
        delete[] szBuf;
    }
    return r;
}

// When using VC, turn back compiler warnings
#if defined(_MSC_VER) && (_MSC_VER > 1100)
#pragma warning (default : 4290) // C++ Exception Specification ignored
#pragma warning (default : 4127) // Conditional expression is constant
#pragma warning (default : 4097) // typedef name used as synonym for class name
#pragma warning (default : 4996 4786)
#endif

#ifdef __BORLANDC__
#pragma option pop  // Turn back on inline function warnings
#endif
