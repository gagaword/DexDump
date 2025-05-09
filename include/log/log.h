//
// Created by GaGa on 25-5-8.
//

#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

// 日志级别枚举
typedef enum {
    LOG_LEVEL_VERBOSE = 0,  // 详细信息
    LOG_LEVEL_DEBUG,        // 调试信息
    LOG_LEVEL_INFO,         // 一般信息
    LOG_LEVEL_WARNING,      // 警告信息
    LOG_LEVEL_ERROR,        // 错误信息
    LOG_LEVEL_FATAL         // 严重错误
} LogLevel;

// 日志颜色（ANSI转义序列）
#define LOG_COLOR_RESET   "\033[0m"
#define LOG_COLOR_BLACK   "\033[30m"
#define LOG_COLOR_RED     "\033[31m"
#define LOG_COLOR_GREEN   "\033[32m"
#define LOG_COLOR_YELLOW  "\033[33m"
#define LOG_COLOR_BLUE    "\033[34m"
#define LOG_COLOR_MAGENTA "\033[35m"
#define LOG_COLOR_CYAN    "\033[36m"
#define LOG_COLOR_WHITE   "\033[37m"

// 全局日志级别，可以在运行时更改
extern LogLevel g_logLevel;

// 是否启用颜色（默认不启用，避免乱码）
extern bool g_logColorEnabled;

// 初始化日志系统
void log_init();

// 日志级别设置函数
 void log_set_level(LogLevel level);

// 获取当前日志级别
 LogLevel log_get_level();

// 设置是否启用颜色
void log_enable_color(bool enable);

#ifdef _WIN32
// 将Windows错误代码转换为错误消息
 char* log_win_error_msg(DWORD error_code);
#endif

// 提取文件名（不包含路径）
 const char* log_extract_filename(const char* path);

// 日志输出函数
void log_print(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...);

// 日志宏定义
#define LOGV(...) log_print(LOG_LEVEL_VERBOSE, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOGD(...) log_print(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOGI(...) log_print(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOGW(...) log_print(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOGE(...) log_print(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)
#define LOGF(...) log_print(LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

// 条件日志宏定义
#define LOG_IF(condition, level, ...) do { if (condition) { log_print(level, __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__); } } while(0)
#define LOGV_IF(condition, ...) LOG_IF(condition, LOG_LEVEL_VERBOSE, __VA_ARGS__)
#define LOGD_IF(condition, ...) LOG_IF(condition, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define LOGI_IF(condition, ...) LOG_IF(condition, LOG_LEVEL_INFO, __VA_ARGS__)
#define LOGW_IF(condition, ...) LOG_IF(condition, LOG_LEVEL_WARNING, __VA_ARGS__)
#define LOGE_IF(condition, ...) LOG_IF(condition, LOG_LEVEL_ERROR, __VA_ARGS__)
#define LOGF_IF(condition, ...) LOG_IF(condition, LOG_LEVEL_FATAL, __VA_ARGS__)


// Windows错误日志宏定义
#ifdef _WIN32
#define LOGE_WIN(message, error_code) log_print(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, "%s: %s (code: %lu)", message, log_win_error_msg(error_code), error_code)
#define LOGF_WIN(message, error_code) log_print(LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, "%s: %s (code: %lu)", message, log_win_error_msg(error_code), error_code)
#define LOGE_WIN_LAST(message) LOGE_WIN(message, GetLastError())
#define LOGF_WIN_LAST(message) LOGF_WIN(message, GetLastError())
#endif

// 断言宏定义
#define LOG_ASSERT(condition, ...) \
    do { \
        if (!(condition)) { \
            LOGF("Assertion failed: %s", #condition); \
            LOGF(__VA_ARGS__); \
            abort(); \
        } \
    } while(0)

#endif //LOG_H