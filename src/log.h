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
void log_message(bool newline, const char *error_level, const char *message, ...);
}


#if LOG_TRACE_ENABLED == 1
# define CORE_TRACE(message, ...) {Vulkan::log_message(true, "TRACE", message, ##__VA_ARGS__);}
#else
# define CORE_TRACE(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
# define CORE_DEBUG(message, ...) {Vulkan::log_message(true, "DEBUG", message, ##__VA_ARGS__);}
#else
# define CORE_DEBUG(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
# define CORE_INFO(message, ...) {Vulkan::log_message(true, "INFO", message, ##__VA_ARGS__);}
#else
# define CORE_INFO(message, ...)
#endif

#if LOG_WARN_ENABLED == 1
# ifdef DEBUG
#  define CORE_WARN(message, ...) {Vulkan::log_message(false, "WARN", "[%s:%d %s()]\t", __FILE__, __LINE__, __func__); Vulkan::log_message(true, nullptr, message, ##__VA_ARGS__);}
# else
#  define CORE_WARN(message, ...) {Vulkan::log_message(true, "WARN", message, ##__VA_ARGS__);}
# endif
#else
# define CORE_WARN(message, ...)
#endif

#ifdef DEBUG
# define CORE_ERROR(message, ...) {Vulkan::log_message(false, "ERROR", "[%s:%d %s()]\t", __FILE__, __LINE__, __func__); Vulkan::log_message(true, nullptr, message, ##__VA_ARGS__);}
#else
# define CORE_ERROR(message, ...) {Vulkan::log_message(true, "ERROR", message, ##__VA_ARGS__);}
#endif

# define CORE_FATAL(message, ...) {Vulkan::log_message(true, "FATAL", message, ##__VA_ARGS__);}

#define TODO_PROPAGATE_ERRORS CORE_DEBUG("TODO: this function should propagate errors!");

