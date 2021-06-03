#pragma once

#define CheckAndReturnFalse(RESULT) if(FAILED((RESULT))) { return false; }

#define ASSERT assert

#define CHECK_HRESULT(exp)                                                     \
{                                                                              \
	HRESULT hres = (exp);                                                      \
	if (!SUCCEEDED(hres))                                                      \
	{                                                                          \
		LogError("%s: FAILED with HRESULT: %u", #exp, (uint32_t)hres);		   \
		ASSERT(false);                                                         \
	}                                                                          \
}
