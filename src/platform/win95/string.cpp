#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "string.hpp"

String::String()
: rep(0)
, len(0)
, cstring(0)
{
}

String::String(char const * str, size_t maxlen)
: rep(0)
, len(strlen(str))
, cstring(0)
{
	if (len > maxlen) len = maxlen;
	if (!len) return;
	rep = (char *)malloc(len*sizeof(char));
	memcpy(rep,str,len*sizeof(char));
}

String::String(String const & str, size_t start, size_t leng)
: rep(0)
, len(leng)
, cstring(0)
{
	if (start>str.len) return;
	if (start+len>str.len) len=str.len-start;
	if (!len) return;
	rep = (char *)malloc(len*sizeof(char));
	memcpy(rep,&str.rep[start],len*sizeof(char));
}

String::String(char c, size_t n)
: rep(0)
, len(n)
, cstring(0)
{
	if (len)
	{
		rep = (char *)malloc(len*sizeof(char));
		for (size_t p=0; p<len; ++p)
			rep[p]=c;
	}
}

String::~String()
{
	if (cstring) free(cstring);
	if (rep) free(rep);
}

char const * String::c_str() const
{
	if (!len) return "";
	if (cstring)
	{
		if (strlen(cstring)==len && !strncmp(cstring,rep,len))
			return cstring;
		free(cstring);
	}
	char * newcstr = (char *)malloc(len+1);
	memcpy(newcstr,rep,len*sizeof(char));
	newcstr[len]=0;
	*(char * *)&cstring = newcstr;
	return newcstr;
}

#if 0
String::operator char () const
{
	return len ? rep[0] : 0;
}
#endif

String & String::operator = (String const & str)
{
	if (&str != this)
	{
		if (rep) free(rep);
		rep = 0;
		len = str.len;
		if (len)
		{
			rep = (char *)malloc(len*sizeof(char));
			memcpy(rep,str.rep,len*sizeof(char));
		}
	}
	return *this;
}

String & String::operator = (char const * str)
{
	if (rep) free(rep);
	rep = 0;
	len = strlen(str);
	if (len)
	{
		rep = (char *)malloc(len*sizeof(char));
		memcpy(rep,str,len*sizeof(char));
	}
	return *this;
}

String & String::operator += (String const & str)
{
	if (&str != this)
	{
		if (str.len)
		{
			rep = (char *)realloc(rep,(len+str.len)*sizeof(char));
			memcpy(&rep[len],str.rep,str.len*sizeof(char));
			len += str.len;
		}
	}
	else if (len)
	{
		rep = (char *)realloc(rep,len*2*sizeof(char));
		memcpy(&rep[len],rep,len*sizeof(char));
		len *= 2;
	}
	return *this;
}

String & String::operator += (char const * str)
{
	size_t clen = strlen(str);
	if (clen)
	{
		rep = (char *)realloc(rep,(len+clen)*sizeof(char));
		memcpy(&rep[len],str,clen*sizeof(char));
		len += clen;
	}
	return *this;
}

#define STRING_COMPARES(op) \
int operator op (String const & str1, String const & str2) \
{ \
	return strcmp(str1.c_str(),str2.c_str()) op 0; \
} \
int operator op (String const & str1, char const * str2) \
{ \
	return strcmp(str1.c_str(),str2) op 0; \
} \
int operator op (char const * str1, String const & str2) \
{ \
	return strcmp(str1,str2.c_str()) op 0; \
} \
int operator op (String const & str1, char c) \
{ \
	char buf[] = { c, 0 }; \
	return strcmp(str1.c_str(),buf) op 0; \
} \
int operator op (char c, String const & str2) \
{ \
	char buf[] = { c, 0 }; \
	return strcmp(buf,str2.c_str()) op 0; \
}
STRING_COMPARES(==)
STRING_COMPARES(!=)
STRING_COMPARES(<=)
STRING_COMPARES(>=)
STRING_COMPARES(<)
STRING_COMPARES(>)

void String::put_at(size_t pos, char c)
{
	if (pos<len)
		rep[pos]=c;
	else
		operator += (c);
}

int String::match(String const & str) const
{
	for (size_t pos = 0; pos<len || pos<str.len; ++pos)
	{
		if (pos>=len || pos>=str.len || rep[pos] != str.rep[pos]) return (int)pos;
	}
	return -1;
}

int String::match(char const * str) const
{
	for (size_t pos = 0; pos<len || *str; ++pos, ++str)
	{
		if (pos>=len || !*str || rep[pos] != *str) return (int)pos;
	}
	return -1;
}

int String::index(String const & str, size_t pos) const
{
	if (!str.len) return (int)pos;
	for (size_t spos = 0, rpos = pos; pos<len; ++pos)
	{
		if (rep[pos]==str.rep[spos])
		{
			if (!spos) rpos = pos;
			++spos;
			if (spos >= str.len) return (int)rpos;
		}
		else spos = 0;
	}
	return -1;
}

int String::index(char const * str, size_t pos) const
{
	if (!*str) return (int)pos;
	char const * strP = str;
	for (size_t rpos = pos; pos<len; ++pos)
	{
		if (rep[pos]==*strP)
		{
			if (strP==str) rpos = pos;
			++strP;
			if (!*strP) return (int)rpos;
		}
		else strP = str;
	}
	return -1;
}

String String::upper() const
{
	String rstr(*this);
	for (size_t pos=0; pos<rstr.len; ++pos)
	{
		rstr.rep[pos] = (char)toupper(rstr.rep[pos]);
	}
	return rstr;
}

String String::lower() const
{
	String rstr(*this);
	for (size_t pos=0; pos<rstr.len; ++pos)
	{
		rstr.rep[pos] = (char)tolower(rstr.rep[pos]);
	}
	return rstr;
}

String::String(String const & str1, String const & str2)
: rep(0)
, len(str1.len + str2.len)
, cstring(0)
{
	if (!len) return;
	rep = (char *)malloc(len*sizeof(char));
	if (str1.len) memcpy(rep,str1.rep,str1.len*sizeof(char));
	if (str2.len) memcpy(&rep[str1.len],str2.rep,str2.len*sizeof(char));
}

String::String(char const * str1, String const & str2)
: rep(0)
, cstring(0)
{
	size_t clen = strlen(str1);
	len = clen + str2.len;
	if (!len) return;
	rep = (char *)malloc(len*sizeof(char));
	if (clen) memcpy(rep,str1,clen*sizeof(char));
	if (str2.len) memcpy(&rep[clen],str2.rep,str2.len*sizeof(char));
}

String::String(String const & str1, char const * str2)
: rep(0)
, cstring(0)
{
	size_t clen = strlen(str2);
	len = str1.len + clen;
	if (!len) return;
	rep = (char *)malloc(len*sizeof(char));
	if (str1.len) memcpy(rep,str1.rep,str1.len*sizeof(char));
	if (clen) memcpy(&rep[str1.len],str2,clen*sizeof(char));
}

String::String(char c, String const & str2)
: rep(0)
, len(1 + str2.len)
, cstring(0)
{
	rep = (char *)malloc(len*sizeof(char));
	rep[0]=c;
	if (str2.len) memcpy(&rep[1],str2.rep,str2.len*sizeof(char));
}

String::String(String const & str1, char c)
: rep(0)
, len(str1.len + 1)
, cstring(0)
{
	rep = (char *)malloc(len*sizeof(char));
	if (str1.len) memcpy(rep,str1.rep,str1.len*sizeof(char));
	rep[str1.len]=c;
}




