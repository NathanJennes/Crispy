#include <cstdarg>
#include <iostream>
#include "log.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

namespace Vulkan
{

void log_debug(const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	printf("%s[DEBUG]: ", BLUE);
	vprintf(message, args);
	printf("\n%s", RESET);
	va_end(args);
}

void log_trace(const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	printf("%s[TRACE]: ", WHITE);
	vprintf(message, args);
	printf("\n%s", RESET);
	va_end(args);
}

void log_info(const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	printf("%s[INFO]: ", GREEN);
	vprintf(message, args);
	printf("\n%s", RESET);
	va_end(args);
}

void log_warn(const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	printf("%s[WARN]: ", YELLOW);
	vprintf(message, args);
	printf("\n%s", RESET);
	va_end(args);
}

void log_error(const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	printf("%s[ERROR]: ", MAGENTA);
	vprintf(message, args);
	printf("\n%s", RESET);
	va_end(args);
}

void log_fatal(const char *message, ...)
{
	__builtin_va_list args;
	va_start(args, message);
	printf("%s[FATAL]: ", RED);
	vprintf(message, args);
	printf("\n%s", RESET);
	va_end(args);
}

}

