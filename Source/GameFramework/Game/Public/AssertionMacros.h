#pragma once

#define CheckAndReturnFalse(RESULT) if(FAILED((RESULT))) { return false; }

#define Ensure(expr) if(!(expr)) {LogError("%s(%s): Ensure fail: %s", __FILE__, __LINE__, #expr); __debugbreak();}

#define EnsureMsg(expr, msg) if(!(expr)) {LogError("%s(%s): Ensure fail: %s", __FILE__, __LINE__, (msg)); __debugbreak(); }

#define EnsureHResult(exp) Ensure(SUCCEEDED(exp))
