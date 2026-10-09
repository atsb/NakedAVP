#ifndef _included_string_hpp_
#define _included_string_hpp_

#ifndef __cplusplus
#error "string.hpp requires C++ compilation"
#endif

#include <stddef.h>

//const size_t NPOS = (size_t) -1;
#define _BIGSIZET ((size_t)-1)

class String
{
public:
	String();
	String(char const *, size_t = _BIGSIZET);
	String(String const &, size_t = 0, size_t = _BIGSIZET);
	String(char, size_t = 1);
	
	~String();
	
	inline operator char const * () const;
	char const * c_str() const;
	//operator char () const;
	
    String & operator = (String const &);
    String & operator = (char const *);

    String & operator += (String const &);
    String & operator += (char const *);

    inline String operator () (size_t, size_t) const;
    inline char & operator () (size_t);
    inline char const & operator () (size_t) const;
    inline char & operator [] (size_t);
    inline char const & operator [] (size_t) const;

    friend int operator == (String const &, String const &);
    friend int operator == (String const &, char const *);
    friend int operator == (char const *, String const &);
    friend int operator == (String const &, char);
    friend int operator == (char, String const &);

    friend int operator != (String const &, String const &);
    friend int operator != (String const &, char const *);
    friend int operator != (char const *, String const &);
    friend int operator != (String const &, char);
    friend int operator != (char, String const &);

    friend int operator <= (String const &, String const &);
    friend int operator <= (String const &, char const *);
    friend int operator <= (char const *, String const &);
    friend int operator <= (String const &, char);
    friend int operator <= (char, String const &);

    friend int operator >= (String const &, String const &);
    friend int operator >= (String const &, char const *);
    friend int operator >= (char const *, String const &);
    friend int operator >= (String const &, char);
    friend int operator >= (char, String const &);

    friend int operator < (String const &, String const &);
    friend int operator < (String const &, char const *);
    friend int operator < (char const *, String const &);
    friend int operator < (String const &, char);
    friend int operator < (char, String const &);

    friend int operator > (String const &, String const &);
    friend int operator > (String const &, char const *);
    friend int operator > (char const *, String const &);
    friend int operator > (String const &, char);
    friend int operator > (char, String const &);

    friend inline String operator + (String const &, String const &);
    friend inline String operator + (String const &, char const *);
    friend inline String operator + (char const *, String const &);
    friend inline String operator + (String const &, char);
    friend inline String operator + (char, String const &);

    inline size_t length() const;

    inline char const & get_at(size_t) const;
    void put_at(size_t, char);

    int match(String const &) const;
    int match(char const *) const;

    int index(String const &, size_t = 0) const;
    int index(char const *, size_t = 0 ) const;

    String upper() const;
    String lower() const;

    inline int operator ! () const;
    inline int valid() const;
    friend inline int valid(String const &);

private:
	String(String const &, String const &);
	String(char const *, String const &);
	String(String const &, char const *);
	String(char, String const &);
	String(String const &, char);
	
	char * rep;
	size_t len;
	char * cstring;
};

inline String::operator char const * () const
{
	return c_str();
}

inline String String::operator () (size_t start, size_t leng) const
{
	return String(*this,start,leng);
}

inline char & String::operator () (size_t pos)
{
	return operator [] (pos);
}
	
inline char const & String::operator () (size_t pos) const
{
	return operator [] (pos);
}
	
inline char & String::operator [] (size_t pos)
{
	return rep[pos];
}
	
inline char const & String::operator [] (size_t pos) const
{
	return rep[pos];
}

#define STRING_CONSTRCAT(arg1,arg2) \
inline String operator + (arg1 a1,arg2 a2) \
{ \
	return String(a1,a2); \
}
STRING_CONSTRCAT(String const &, String const &)
STRING_CONSTRCAT(char const *, String const &)
STRING_CONSTRCAT(String const &, char const *)
STRING_CONSTRCAT(char , String const &)
STRING_CONSTRCAT(String const &, char)

inline size_t String::length() const
{
	return len;
}

inline char const & String::get_at(size_t pos) const
{
	return operator [] (pos);
}

inline int String::operator ! () const
{
	return !valid();
}

inline int String::valid() const
{
	return 1;
}
	
inline int valid(String const & str)
{
	return str.valid();
}

#endif
