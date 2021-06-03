#pragma once

#ifndef LOG_SEVERITY_LEVEL
#define LOG_SEVERITY_LEVEL 10
#endif

#include "stb/stb_log.h"

#define Log(fmt, ...) log_info(fmt, ##__VA_ARGS__)
#define LogDebug(fmt, ...) log_debug(fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...) log_info(fmt, ##__VA_ARGS__)
#define LogWarning(fmt, ...) log_warning(fmt, ##__VA_ARGS__)
#define LogError(fmt, ...) log_error(fmt, ##__VA_ARGS__)
#define LogCritical(fmt, ...) log_critical(fmt, ##__VA_ARGS__)
