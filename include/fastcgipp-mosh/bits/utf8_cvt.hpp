#ifndef FASTCGIPP_UTF8_FACET
#define FASTCGIPP_UTF8_FACET

#include <locale>
#include <cwchar>
#include <cstddef>

//! A codecvt facet for UTF-8 encoding/decoding. RFC 3629 compliant.
class Utf8_cvt : public std::codecvt<wchar_t, char, mbstate_t> {
public:
	explicit Utf8_cvt(std::size_t no_manage_ref = 0)
	: std::codecvt<wchar_t, char, std::mbstate_t>(no_manage_ref)
	{ }
protected:
	virtual std::codecvt_base::result do_in(
			std::mbstate_t&, const char* from,
			const char* from_end, const char *& from_next,
			wchar_t* to, wchar_t* to_end, wchar_t*& to_next
		) const;
	virtual std::codecvt_base::result do_out(
			std::mbstate_t&, const wchar_t* from,
			const wchar_t* from_end, const wchar_t *& from_next,
			char* to, char* to_end, char *& to_next
		) const;
	virtual bool do_always_noconv() const throw() { return false; }	
	virtual std::codecvt_base::result do_unshift(std::mbstate_t&,
			char * from, char * /*to*/, char * & next) const {
		next = from;
		return ok;
	}
			
	virtual int do_encoding() const throw() { return 0; /* variable-width */ }
	virtual int do_length(std::mbstate_t&, const char* from, const char* from_end, std::size_t limit) const;
	virtual int do_max_length() const throw() { return 4; /* 4 bytes needed for U+10FFFF */ }
};

#endif
