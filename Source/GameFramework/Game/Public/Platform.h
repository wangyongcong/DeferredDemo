#pragma once

// Codes from EASTL library eaplatform.h

#if (defined(PLATFORM_WINDOWS) || (defined(_WIN32) || defined(__WIN32__) || defined(_WIN64))) && !defined(CS_UNDEFINED_STRING)
#undef  PLATFORM_WINDOWS
#define PLATFORM_WINDOWS 1
#define PLATFORM_NAME "Windows"
#ifdef _WIN64 // VC++ defines both _WIN32 and _WIN64 when compiling for Win64.
	#define PLATFORM_WIN64 1
#else
	#define PLATFORM_WIN32 1
#endif // _WIN64
#if defined(_M_AMD64) || defined(_AMD64_) || defined(__x86_64__)
	#define PROCESSOR_X86_64 1
	#define SYSTEM_LITTLE_ENDIAN 1
	#define PLATFORM_DESCRIPTION "Windows on x64"
#elif defined(_M_IX86) || defined(_X86_)
	#define PROCESSOR_X86 1
	#define SYSTEM_LITTLE_ENDIAN 1
	#define PLATFORM_DESCRIPTION "Windows on X86"
#elif defined(_M_IA64) || defined(_IA64_)
	#define PROCESSOR_IA64 1
	#define SYSTEM_LITTLE_ENDIAN 1
	#define PLATFORM_DESCRIPTION "Windows on IA-64"
#elif defined(_M_ARM)
	#define ABI_ARM_WINCE 1
	#define PROCESSOR_ARM32 1
	#define SYSTEM_LITTLE_ENDIAN 1
	#define PLATFORM_DESCRIPTION "Windows on ARM"
#else //Possibly other Windows CE variants
	#error Unknown processor
	#error Unknown endianness
#endif // Architecture
#if defined(WINAPI_FAMILY)
#if defined(_MSC_VER)
#pragma warning(push, 0)
#endif
#include <winapifamily.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#endif // WINAPI_FAMILY
#endif // PLATFORM_WINDOWS

// PLATFORM_PTR_SIZE
// Platform pointer size; same as sizeof(void*).
// This is not the same as sizeof(int), as int is usually 32 bits on
// even 64 bit platforms.
//
// _WIN64 is defined by Win64 compilers, such as VC++.
// _M_IA64 is defined by VC++ and Intel compilers for IA64 processors.
// __LP64__ is defined by HP compilers for the LP64 standard.
// _LP64 is defined by the GCC and Sun compilers for the LP64 standard.
// __ia64__ is defined by the GCC compiler for IA64 processors.
// __arch64__ is defined by the Sparc compiler for 64 bit processors.
// __mips64__ is defined by the GCC compiler for MIPS processors.
// __powerpc64__ is defined by the GCC compiler for PowerPC processors.
// __64BIT__ is defined by the AIX compiler for 64 bit processors.
// __sizeof_ptr is defined by the ARM compiler (armcc, armcpp).
//
#ifndef PLATFORM_PTR_SIZE
#if defined(__WORDSIZE) // Defined by some variations of GCC.
#define PLATFORM_PTR_SIZE ((__WORDSIZE) / 8)
#elif defined(_WIN64) || defined(__LP64__) || defined(_LP64) || defined(_M_IA64) || defined(__ia64__) || defined(__arch64__) || defined(__aarch64__) || defined(__mips64__) || defined(__64BIT__) || defined(__Ptr_Is_64)
#define PLATFORM_PTR_SIZE 8
#elif defined(__CC_ARM) && (__sizeof_ptr == 8)
#define PLATFORM_PTR_SIZE 8
#else
#define PLATFORM_PTR_SIZE 4
#endif
#endif

// PLATFORM_WORD_SIZE
// This defines the size of a machine word. This will be the same as
// the size of registers on the machine but not necessarily the same
// as the size of pointers on the machine. A number of 64 bit platforms
// have 64 bit registers but 32 bit pointers.
//
#ifndef PLATFORM_WORD_SIZE
#define PLATFORM_WORD_SIZE PLATFORM_PTR_SIZE
#endif

// PLATFORM_MIN_MALLOC_ALIGNMENT
// This defines the minimal alignment that the platform's malloc 
// implementation will return. This should be used when writing custom
// allocators to ensure that the alignment matches that of malloc
#ifndef PLATFORM_MIN_MALLOC_ALIGNMENT
#if defined(PLATFORM_APPLE)
#define PLATFORM_MIN_MALLOC_ALIGNMENT 16
#elif defined(PLATFORM_ANDROID) && defined(PROCESSOR_ARM)
#define PLATFORM_MIN_MALLOC_ALIGNMENT 8
#elif defined(PLATFORM_NINTENDO) && defined(PROCESSOR_ARM)
#define PLATFORM_MIN_MALLOC_ALIGNMENT 8
#elif defined(PLATFORM_ANDROID) && defined(PROCESSOR_X86_64)
#define PLATFORM_MIN_MALLOC_ALIGNMENT 8
#else
#define PLATFORM_MIN_MALLOC_ALIGNMENT (PLATFORM_PTR_SIZE * 2)
#endif
#endif

// CACHE_LINE_SIZE
// Specifies the cache line size broken down by compile target.
// This the expected best guess values for the targets that we can make at compilation time.
#ifndef CACHE_LINE_SIZE
#if   defined(PROCESSOR_X86)      
#define CACHE_LINE_SIZE 32    // This is the minimum possible value.
#elif defined(PROCESSOR_X86_64)  
#define CACHE_LINE_SIZE 64    // This is the minimum possible value
#elif defined(PROCESSOR_ARM32)
#define CACHE_LINE_SIZE 32    // This varies between implementations and is usually 32 or 64. 
#elif defined(PROCESSOR_ARM64)
#define CACHE_LINE_SIZE 64    // Cache line Cortex-A8  (64 bytes) http://shervinemami.info/armAssembly.html however this remains to be mostly an assumption at this stage
#elif (PLATFORM_WORD_SIZE == 4)
#define CACHE_LINE_SIZE 32    // This is the minimum possible value
#else
#define CACHE_LINE_SIZE 64    // This is the minimum possible value
#endif
#endif

// Detecting the availability of SSE at compile-time is a bit more involving with Visual Studio...
#if defined(_MSC_VER) && !defined(NN_NINTENDO_SDK)
#if (defined(__AVX__) || defined(__AVX2__) || defined(_M_AMD64) || defined(_M_X64) || (_M_IX86_FP == 1) || (_M_IX86_FP == 2))
#define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 1
#else // SSE support
#define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 0
#endif // SSE support
#else // !_MSC_VER
#if defined(__SSE__)
#define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 1
#elif defined(__ANDROID__)
#if defined(ANDROID_ARM_NEON)
#define VECTORMATH_CPU_HAS_NEON 1
#else
#define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 0
#define VECTORMATH_CPU_HAS_NEON 0
#endif
#elif defined(__arm64) || defined(__aarch64__) || defined(__arm__ )
#define VECTORMATH_CPU_HAS_NEON 1
#else // !__SSE__
#define VECTORMATH_CPU_HAS_SSE1_OR_BETTER 0
#define VECTORMATH_CPU_HAS_NEON 0
#endif // __SSE__
#endif // _MSC_VER

#define VECTORMATH_FORCE_SCALAR_MODE 0

#if defined(ORBIS) || defined(PROSPERO)
#define VECTORMATH_MODE_SCE 1
#endif

// Vectormath mode selection
#if VECTORMATH_MODE_SCE
#define VECTORMATH_MODE_SCALAR 0
#define VECTORMATH_MODE_SSE    1
#define VECTORMATH_MODE_NEON   0
#define VECTORMATH_MIN_ALIGN   16
#elif (VECTORMATH_CPU_HAS_SSE1_OR_BETTER && !VECTORMATH_FORCE_SCALAR_MODE) // SSE
#define VECTORMATH_MODE_SCALAR 0
#define VECTORMATH_MODE_SSE    1
#define VECTORMATH_MODE_NEON   0
#define VECTORMATH_MIN_ALIGN   16
#elif (VECTORMATH_CPU_HAS_NEON && !VECTORMATH_FORCE_SCALAR_MODE) // NEON
#define VECTORMATH_MODE_SCALAR 0
#define VECTORMATH_MODE_SSE    0
#define VECTORMATH_MODE_NEON   1
#define VECTORMATH_MIN_ALIGN   16
#else // !SSE
#define VECTORMATH_MODE_SCALAR 1
#define VECTORMATH_MODE_SSE    0
#define VECTORMATH_MODE_NEON   0
#define VECTORMATH_MIN_ALIGN   0
#endif // Vectormath mode selection
