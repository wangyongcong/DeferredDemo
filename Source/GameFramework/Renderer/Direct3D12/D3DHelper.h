#pragma once

#define SAFE_RELEASE(ptr) do { if(ptr) { (ptr)->Release(); (ptr) = NULL; } } while(false)

#define SAFE_CLOSE_HANDLE(handle) do { if((handle) != NULL) { CloseHandle((handle)); (handle) = NULL; } } while(false)
