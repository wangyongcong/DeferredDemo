#pragma once

#include <cstdint>
#include <string>

#ifndef DISALLOW_COPY_MOVE_AND_ASSIGN
#define DISALLOW_COPY_MOVE_AND_ASSIGN(TypeName) \
	TypeName(const TypeName&) = delete;			\
	void operator=(const TypeName&) = delete;	\
	TypeName(TypeName&&) = delete;				\
	void operator=(TypeName&&) = delete;
#endif

namespace wyc
{

	typedef float float32_t;

	inline bool have_state(unsigned st, unsigned flag) {
		return flag == (st & flag);
	}
	inline void add_state(unsigned &st, unsigned flag) {
		st |= flag;
	}
	inline void set_state(unsigned &st, unsigned mask, unsigned flag) {
		st &= ~mask;
		st |= flag;
	}
	inline void remove_state(unsigned &st, unsigned flag) {
		st &= ~flag;
	}

	inline bool is_power2(unsigned val)
	{
		return (val&(val - 1)) == 0;
	}

	// return the minimal r, which (r = 2^a and r >= val)
	unsigned minimal_power2(unsigned val);

	// return log2(val), if val = 2^a (a >= 0)
	uint32_t log2p2(uint32_t val);

	// wstring to UTF-8 string
	bool wstr_to_utf8(const std::wstring& in_str, std::string& out_str);
	// UTF-8 string to wstring
	bool utf8_to_wstr(const std::string in_str, std::wstring& out_dst);
	// wstring to ANSI string (CP936)
	bool wstr_to_ansi(const std::wstring& in_str, std::string& out_str);
	// ANSI string (CP936) to wstring
	bool ansi_to_wstr(const std::string in_str, std::wstring& out_str);

} // end of namespace wyc

