#include <locale>
#include <codecvt>
#include "Utility.h"

namespace wyc
{

unsigned minimal_power2(unsigned val)
{
	// val maybe power of 2
	--val;
	// set the bits right of MSB to 1
	val |= (val >>  1);
	val |= (val >>  2);
	val |= (val >>  4);
	val |= (val >>  8);		/* Ok, since int >= 16 bits */
#if (UINT_MAX != 0xffff)
	val |= (val >> 16);		/* For 32 bit int systems */
#if (UINT_MAX > 0xffffffffUL)
	val |= (val >> 32);		/* For 64 bit int systems */
#endif // UINT_MAX != 0xffff
#endif // UINT_MAX > 0xffffffffUL
	++val;
	return val;
}

uint32_t log2p2(uint32_t val)
{
	// 32bit De Bruijn series
	static const int ls_DeBruijn32[32] =
	{
		0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
		31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9
	};
	return ls_DeBruijn32[(uint32_t)(val * 0x077CB531U) >> 27];
}

bool wcs_to_mbcs(uint32_t charset, const std::wstring& wcs, std::string& out_mbcs)
{
	int len = WideCharToMultiByte(CP_ACP, 0, wcs.c_str(), int(wcs.size()), NULL, 0, NULL, NULL);
	if (len <= 0)
	{
		return false;
	}
	auto buff = std::make_unique<char[]>(len);
	char* cstr = buff.get();
	WideCharToMultiByte(charset, 0, wcs.c_str(), int(wcs.size()), buff.get(), len, NULL, NULL);
	out_mbcs.assign(cstr, cstr + len);
	return true;
}

bool mbcs_to_wcs(uint32_t charset, const std::string& mbcs, std::wstring& out_wcs)
{
	int len = MultiByteToWideChar(CP_ACP, 0, mbcs.c_str(), int(mbcs.size()), NULL, 0);
	if (len <= 0)
	{
		return false;
	}
	auto buff = std::make_unique<wchar_t[]>(len);
	wchar_t* wcs = buff.get();
	MultiByteToWideChar(CP_ACP, 0, mbcs.c_str(), int(mbcs.size()), wcs, len);
	out_wcs.assign(wcs, wcs + len);
	return true;
}

bool wstr_to_utf8(const std::wstring& in_str, std::string& out_str)
{
	if (in_str.size() < 1)
	{
		out_str.resize(0);
		return true;
	}
	return wcs_to_mbcs(CP_UTF8, in_str, out_str);
}

bool utf8_to_wstr(const std::string in_str, std::wstring& out_dst)
{
	if (in_str.size() < 1)
	{
		out_dst.resize(0);
		return true;
	}
	return mbcs_to_wcs(CP_UTF8, in_str, out_dst);
}


bool wstr_to_ansi(const std::wstring& in_str, std::string& out_str)
{
	if (in_str.size() < 1)
	{
		out_str.resize(0);
		return true;
	}
	return wcs_to_mbcs(CP_ACP, in_str, out_str);
}

bool ansi_to_wstr(const std::string in_str, std::wstring& out_str)
{
	if (in_str.size() < 1)
	{
		out_str.resize(0);
		return true;
	}
	return mbcs_to_wcs(CP_ACP, in_str, out_str);
}

} // end of namespace wyc
