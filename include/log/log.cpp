#include "log.h"


LogLevel g_logLevel = LOG_LEVEL_INFO;
bool g_logColorEnabled = false;

// 日志级别文本表示
static const char* LOG_LEVEL_NAMES[] = {
    "VERBOSE", "DEBUG", "INFO", "WARNING", "ERROR", "FATAL"
};

// 日志级别颜色表示
static const char* LOG_LEVEL_COLORS[] = {
    LOG_COLOR_WHITE, LOG_COLOR_BLUE, LOG_COLOR_GREEN,
    LOG_COLOR_YELLOW, LOG_COLOR_RED, LOG_COLOR_MAGENTA
};

void log_init()
{
#ifdef _WIN32
    // 在Windows上启用ANSI转义序列支持
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE)
    {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode))
        {
            // ENABLE_VIRTUAL_TERMINAL_PROCESSING 是 Windows 10+ 支持的标志
            dwMode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void log_set_level(LogLevel level)
{
    g_logLevel = level;
}

LogLevel log_get_level()
{
    return g_logLevel;
}

void log_enable_color(bool enable)
{
    g_logColorEnabled = enable;
    if (enable)
    {
        log_init(); // 如果启用颜色，确保控制台支持
    }
}

char* log_win_error_msg(DWORD error_code)
{
    static char error_msg[1024]; // 静态缓冲区，避免内存泄漏

    // 将错误代码转换为错误消息
    if (FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        error_msg,
        sizeof(error_msg) - 1,
        nullptr) == 0)
    {
        // 如果FormatMessage失败，提供一个默认消息
        sprintf(error_msg, "Unknown error (0x%08X)", error_code);
    }
    else
    {
        // 删除可能的尾随换行符
        size_t len = strlen(error_msg);
        while (len > 0 && (error_msg[len - 1] == '\r' || error_msg[len - 1] == '\n'))
        {
            error_msg[--len] = '\0';
        }
    }

    return error_msg;
}

const char* log_extract_filename(const char* path)
{
    const char* filename = strrchr(path, '/');
    if (filename)
    {
        return filename + 1;
    }

    filename = strrchr(path, '\\');
    if (filename)
    {
        return filename + 1;
    }

    return path;
}

void log_print(LogLevel level, const char* file, int line, const char* func, const char* fmt, ...)
{
    if (level < g_logLevel)
    {
        return;
    }

    // 获取当前时间
    time_t now = time(nullptr);
    if (now == static_cast<time_t>(-1))
    {
        fprintf(stderr, "[ERROR] Failed to get current time!\n");
        return;
    }

    tm tm_info;
    if (localtime_s(&tm_info, &now) != 0)
    {
        fprintf(stderr, "[ERROR] Failed to convert time!\n");
        return;
    }

    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_info);

    // 输出日志前缀
    if (g_logColorEnabled)
    {
        fprintf(stderr, "%s%s [%s] %s:%d %s(): ",
                LOG_LEVEL_COLORS[level],
                time_str,
                LOG_LEVEL_NAMES[level],
                log_extract_filename(file),
                line,
                func);
    }
    else
    {
        fprintf(stderr, "%s [%s] %s:%d %s(): ",
                time_str,
                LOG_LEVEL_NAMES[level],
                log_extract_filename(file),
                line,
                func);
    }

    // 输出日志内容
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    // 重置颜色并添加换行
    if (g_logColorEnabled)
    {
        fprintf(stderr, "%s\n", LOG_COLOR_RESET);
    }
    else
    {
        fprintf(stderr, "\n");
    }
}
