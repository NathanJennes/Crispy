#pragma once

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1

#ifdef DEBUG
# define LOG_DEBUG_ENABLED 1
# define LOG_TRACE_ENABLED 1
#else
# define LOG_DEBUG_ENABLED 0
# define LOG_TRACE_ENABLED 0
#endif

namespace Vulkan
{
void log_debug(const char *message, ...);
void log_trace(const char *message, ...);
void log_info(const char *message, ...);
void log_warn(const char *message, ...);
void log_error(const char *message, ...);
void log_fatal(const char *message, ...);
}


#if LOG_TRACE_ENABLED == 1
# define CORE_TRACE(message, ...) Vulkan::log_trace(message, ##__VA_ARGS__);
#else
# define CORE_TRACE(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
# define CORE_DEBUG(message, ...) Vulkan::log_debug(message, ##__VA_ARGS__);
#else
# define CORE_DEBUG(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
# define CORE_INFO(message, ...) Vulkan::log_info(message, ##__VA_ARGS__);
#else
# define CORE_INFO(message, ...)
#endif

#if LOG_WARN_ENABLED == 1
# define CORE_WARN(message, ...) Vulkan::log_warn(message, ##__VA_ARGS__);
#else
# define CORE_WARN(message, ...)
#endif

# define CORE_ERROR(message, ...) Vulkan::log_error(message, ##__VA_ARGS__);

# define CORE_FATAL(message, ...) Vulkan::log_fatal(message, ##__VA_ARGS__);

