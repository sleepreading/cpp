/**
 * @file      z_string.h
 * @date      2013-05-30 17:04:46
 * @author    leizi email:847088355@qq.com
 * @copyright https://www.codeproject.com/articles/1146/cstring-clone-using-standard-c
 * @remark
    This template derives the Standard C++ Library basic_string<>
    1. write/read COM IStream interfaces, you should include afx.h or atlbase.h first!
    2. do not support <=VC5 : Q172398
 * @demo
   String s = String::from_format(_T("%d %s %d"), 1, _T("hello"), 2);
   s.replace2(" EE", u8"-0xf你好");  // 目前_T已经几乎被废弃了,直接使用C++11兼容的utf8即可.std::wstring都可以不用!!
   s.load(IDS_STRING).lower().triml().icompare(s2);
 */
#ifndef __Z_STRING_H__
#define __Z_STRING_H__

#include <string>

#if  _MSC_VER <= 1200
#pragma warning(disable: 4018)  // vc6's vector signed/unsigned compare, must be above the vector's hearder
#endif
#include <vector>


//////////////////////////////////////////////////////////////////////////
// TString template

template<typename T>
class TString : public std::basic_string<T>
{
#if _MSC_VER <= 1300
    typedef __int64            int64;
    typedef unsigned __int64   uint64;
#else
    typedef long long          int64;
    typedef unsigned long long uint64;
#endif

public:
#if _MSC_VER < 1600
	typedef typename std::basic_string<T>::iterator		      iterator;
#endif
    typedef typename std::basic_string<T>::const_pointer      const_pointer; // const char* or const wchar_t*
    typedef typename std::basic_string<T>::const_iterator     const_iterator;
    typedef typename std::basic_string<T>::size_type          size_type;
    typedef typename std::basic_string<T>::value_type         value_type;
    typedef typename std::basic_string<T>::allocator_type     allocator_type;

    TString() {}  // call base class default constructor
    TString(const TString<T> &str) : std::basic_string<T>(str) {}

    // number --> string : std::string std::to_string(int/long/double...) C++11
    TString(const char* pA);
    TString(const wchar_t* pW);
    TString(const std::string &str);
    TString(const std::wstring &str);
    TString(const_pointer pT, size_type n) : std::basic_string<T>(!pT ? TString<T>().c_str() : pT, n) {}
    TString(const_iterator first, const_iterator last) : std::basic_string<T>(first, last) {}
    TString(size_type nSize, value_type ch, const allocator_type &al=allocator_type()) : std::basic_string<T>(nSize, ch, al) {}

    TString<T>& operator=(const TString<T> &str);
    TString<T>& operator=(const std::string &str);
    TString<T>& operator=(const std::wstring &str);
    TString<T>& operator=(const char* pA);
    TString<T>& operator=(const wchar_t* pW);
    TString<T>& operator=(T t);
    TString<T>& operator+=(const TString<T> &str);
    TString<T>& operator+=(const std::string &str);
    TString<T>& operator+=(const std::wstring &str);
    TString<T>& operator+=(const char* pA);
    TString<T>& operator+=(const wchar_t* pW);
    TString<T>& operator+=(T t);

    bool begwith(const_pointer szThat, bool ignorecase=false, const_pointer ignore_begchars=0) const;
    bool endwith(const_pointer szThat, bool ignorecase=false, const_pointer ignore_endchars=0) const;

    int  collate(const_pointer szThat, bool ignorecase=false) const;
    int  compare2(const_pointer szThat, bool ignorecase=false) const;

    std::vector< TString<T> > spliter(const_pointer delim) const;

    TString<T>& upper(const std::locale &loc=std::locale());
    TString<T>  upper_copy(const std::locale &loc=std::locale()) const;
    TString<T>& lower(const std::locale &loc=std::locale());
    TString<T>  lower_copy(const std::locale &loc=std::locale()) const;

    TString<T>& remove(const_pointer delim);  // remove every chars in the delim
    TString<T>  remove_copy(const_pointer delim) const;
    TString<T>& replace2(const_pointer szOld, const_pointer szNew);  // overwrite
    TString<T>  replace_copy(const_pointer szOld, const_pointer szNew) const;
    TString<T>& reverse();
    TString<T>  reverse_copy() const;
	TString<T>& sort();
    TString<T>  sort_copy() const;
	TString<T>& unique();
    TString<T>  unique_copy() const;

    TString<T>& trim(const_pointer szTrimChars=0);  // 0: trim space tab..
    TString<T>  trim_copy(const_pointer szTrimChars=0) const;
    TString<T>& triml(const_pointer szTrimChars=0);
    TString<T>  triml_copy(const_pointer szTrimChars=0) const;
    TString<T>& trimr(const_pointer szTrimChars=0);
    TString<T>  trimr_copy(const_pointer szTrimChars=0) const;

    TString<T>& format(const T *szFmt, ...);
    TString<T>& format_append(const T *szFmt, ...);  // format_append
    static TString<T> from_format(const T *szFmt, ...);

    //////////////////////////////////////////////////////////////////////////
    // small functions
    bool isdigit() const;
    bool isalpha() const;
    bool isspace() const;
    bool islower() const;
    bool isupper() const;
    // string --> number: or use std::stoll(const std::string& s, std::size_t* pos=0, int base=10)
    int           to_int() const;
    long          to_long() const;
    float         to_float() const;
    double        to_double() const;
    int64         to_longlong() const;  // vc6 only supports cast to decimal
    unsigned int  to_uint() const;
    unsigned long to_ulong() const;
    uint64        to_ulonglong() const;
    // number --> string
    static TString<T> from_double(double v, int ndigits=2);
    static TString<T> from_number(int64 v, int radix=10);
    //////////////////////////////////////////////////////////////////////////
    // basic_string member function, but need to return a String type referenced
    // causion: overwrite them!
	// !!!!note!!!! :  vc6 will lose some functions that have param 'InputIterator'!
    // ---- append ----
// #if !(_MSC_VER < 1600)
//     using basic_string<T>::append;
// #endif
    TString<T>& iappend(const_iterator first, const_iterator last)                 { static_cast<std::basic_string<T>*>(this)->append(first, last); return *this; }
    TString<T>& iappend(const TString<T>& str, size_type subpos, size_type sublen) { static_cast<std::basic_string<T>*>(this)->append(str, subpos, sublen); return *this; }
    TString<T>& iappend(const TString<T>& str)        { static_cast<std::basic_string<T>*>(this)->append(str); return *this; }
    TString<T>& iappend(const_pointer s)              { static_cast<std::basic_string<T>*>(this)->append(s); return *this; }
    TString<T>& iappend(const_pointer s, size_type n) { static_cast<std::basic_string<T>*>(this)->append(s, n); return *this; }
    TString<T>& iappend(size_type n, value_type c)    { static_cast<std::basic_string<T>*>(this)->append(n, c); return *this; }

    // ---- assign ----
// #if !(_MSC_VER < 1600)
//     using basic_string<T>::assign;
// #endif
    TString<T>& iassign(const_iterator first, const_iterator last)                 { static_cast<std::basic_string<T>*>(this)->assign(first, last); return *this; }
    TString<T>& iassign(const TString<T>& str, size_type subpos, size_type sublen) { static_cast<std::basic_string<T>*>(this)->assign(str, subpos, sublen); return *this; }
    TString<T>& iassign(const TString<T>& str)        { static_cast<std::basic_string<T>*>(this)->assign(str); return *this; }
    TString<T>& iassign(const_pointer s)              { static_cast<std::basic_string<T>*>(this)->assign(s); return *this; }
    TString<T>& iassign(const_pointer s, size_type n) { static_cast<std::basic_string<T>*>(this)->assign(s, n); return *this; }
    TString<T>& iassign(size_type n, value_type c)    { static_cast<std::basic_string<T>*>(this)->assign(n, c); return *this; }

    // ---- compare ----
// #if (_MSC_VER < 1600)
// 	int compare(const TString<T>& str) const { return static_cast<const std::basic_string<T>*>(this)->compare(str); }
// 	int compare(const_pointer s) const       { return static_cast<const std::basic_string<T>*>(this)->compare(s); }
// 	int compare(size_type pos, size_type len, const TString<T>& str) const { return static_cast<const std::basic_string<T>*>(this)->compare(pos, n, str); }
// 	int compare(size_type pos, size_type len, const TString<T>& str, size_type subpos, size_type sublen) const { return static_cast<const std::basic_string<T>*>(this)->compare(pos, n, str, subpos, sublen); }
// 	int compare(size_type pos, size_type len, const_pointer s) const { return static_cast<const std::basic_string<T>*>(this)->compare(pos, len, s); }
// 	int compare(size_type pos, size_type len, const_pointer s, size_type n) const { return static_cast<const std::basic_string<T>*>(this)->compare(pos, len, s, n); }
// #else
//     using basic_string<T>::compare;
// #endif
    int icompare(size_type pos, const TString<T>& str, bool ignorecase=false) const { return this->substr(pos,str.size()).compare(str.c_str(), ignorecase); }
    int icompare(size_type pos, const_pointer s, bool ignorecase=false) const { return this->substr(pos,std::basic_string<T>::traits_type::length(s)).compare(s, ignorecase); }

    // ---- erase ----
// #if (_MSC_VER < 1600)
// 	iterator erase(iterator p) { return static_cast<std::basic_string<T>*>(this)->erase(p); }
// 	iterator erase(iterator first, iterator last) { return static_cast<std::basic_string<T>*>(this)->erase(first, last); }
// #else
//     using basic_string<T>::erase;
// #endif
    TString<T>& ierase(size_type pos, size_type len) { static_cast<std::basic_string<T>*>(this)->erase(pos, len); return *this; }

    // ---- insert ----
// #if (_MSC_VER < 1600)
// 	void insert(iterator p, size_type n, value_type c) { static_cast<std::basic_string<T>*>(this)->insert(p, n, c); }
// 	iterator insert(iterator p, value_type c)          { return static_cast<std::basic_string<T>*>(this)->insert(p, c); }
// #else
//     using basic_string<T>::insert;
// #endif
    TString<T>& iinsert(size_type pos, const TString<T>& str, size_type subpos, size_type sublen) { static_cast<std::basic_string<T>*>(this)->insert(pos, str, subpos, sublen); return *this; }
    TString<T>& iinsert(size_type pos, const TString<T>& str)        { static_cast<std::basic_string<T>*>(this)->insert(pos, str); return *this; }
    TString<T>& iinsert(size_type pos, const_pointer* s)             { static_cast<std::basic_string<T>*>(this)->insert(pos, s); return *this; }
    TString<T>& iinsert(size_type pos, const_pointer* s, size_type n){ static_cast<std::basic_string<T>*>(this)->insert(pos, s, n); return *this; }
    TString<T>& iinsert(size_type pos, size_type n, value_type c)    { static_cast<std::basic_string<T>*>(this)->insert(pos, n, c); return *this; }

    // ---- replace ----
// #if (_MSC_VER < 1600)
// 	TString<T>& replace(iterator i1, iterator i2, const TString<T>& str)           { static_cast<std::basic_string<T>*>(this)->replace(i1, i2, str); return *this; }
// 	TString<T>& replace(iterator i1, iterator i2, const_pointer s)                 { static_cast<std::basic_string<T>*>(this)->replace(i1, i2, s); return *this; }
// 	TString<T>& replace(iterator i1, iterator i2, const_pointer s, size_type n)    { static_cast<std::basic_string<T>*>(this)->replace(i1, i2, s, n); return *this; }
// 	TString<T>& replace(iterator i1, iterator i2, size_type n, value_type c)       { static_cast<std::basic_string<T>*>(this)->replace(i1, i2, n, c); return *this; }
// #else
//     using basic_string<T>::replace;
// #endif
    TString<T>& ireplace(size_type pos, size_type len, const TString<T>& str, size_type subpos, size_type sublen) { static_cast<std::basic_string<T>*>(this)->replace(pos, len, str, subpos, sublen); return *this; }
    TString<T>& ireplace(size_type pos, size_type len, const TString<T>& str)       { static_cast<std::basic_string<T>*>(this)->replace(pos, len, str); return *this; }
    TString<T>& ireplace(size_type pos, size_type len, const_pointer s)             { static_cast<std::basic_string<T>*>(this)->replace(pos, len, s); return *this; }
    TString<T>& ireplace(size_type pos, size_type len, const_pointer s, size_type n){ static_cast<std::basic_string<T>*>(this)->replace(pos, len, s, n); return *this; }
    TString<T>& ireplace(size_type pos, size_type len, size_type n, value_type c)   { static_cast<std::basic_string<T>*>(this)->replace(pos, len, n, c); return *this; }

    // ---- substr ----
    TString<T> substr(size_type pos = 0, size_type len = npos) const { if (pos >= this->size()) return TString<T>(); return TString<T>(static_cast<const std::basic_string<T>*>(this)->substr(pos, len)); }

    //////////////////////////////////////////////////////////////////////////
    // nearly use
    #ifdef  _WIN32
    bool load(unsigned int nId);
    TString<T>& format(unsigned int nId, ...);
    static TString<T> get_last_errmsg();
    #endif

    void free_extra();
    T*   GetBuffer(int nMinLen = -1);
    void ReleaseBuffer(int nNewLen = -1);

private:
    void _try_load(const void *pT);

};   // class TString

typedef TString<char>       StringA;    // a better std::string
typedef TString<wchar_t>    StringW;    // a better std::wstring
#if defined(_UNICODE) || defined(UNICODE)
typedef StringW             String;
#else
typedef StringA             String;
#endif

StringA operator+(const StringA &s1, const StringA &s2);
StringA operator+(const StringA &s1, StringA::value_type t);
StringA operator+(const StringA &s1, const char* pA);
StringA operator+(const char* pA, const StringA &sA);
StringA operator+(const StringA &s1, const StringW &s2);
StringW operator+(const StringW &s1, const StringW &s2);
StringA operator+(const StringA &s1, const wchar_t* pW);
#if defined(_UNICODE) || defined(UNICODE)
StringW operator+(const wchar_t* pW, const StringA &sA);
StringW operator+(const char* pA, const StringW &sW);
#else
StringA operator+(const wchar_t* pW, const StringA &sA);
StringA operator+(const char* pA, const StringW &sW);
#endif
StringW operator+(const StringW &s1, StringW::value_type t);
StringW operator+(const StringW &s1, const wchar_t *pW);
StringW operator+(const wchar_t *pW, const StringW &sW);
StringW operator+(const StringW &s1, const StringA &s2);
StringW operator+(const StringW &s1, const char* pA);


#if  _MSC_VER <= 1200
#pragma warning(default: 4018)  // vc6's vector signed/unsigned compare
#endif

#include "zstring_impl.h"
#endif
