#pragma once

#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <ctime>
#include <stdarg.h>
#include <fstream>
#include <cstring>
#include <mutex>

namespace my_muduo
{
#define LOG_COLOR_E "\x1b[31m"
#define LOG_COLOR_W "\x1b[33m"
#define LOG_COLOR_I "\x1b[32m"
#define LOG_COLOR_D "\x1b[30m"
#define LOG_RESET_COLOR "\x1b[0m"

#define SCREEN_TYPE 1
#define FILE_TYPE 2

    typedef enum
    {
        LOG_ERROR, /*!< 严重错误，软件模块无法自行恢复 */
        LOG_WARN,  /*!< 已采取恢复措施的错误条件 */
        LOG_INFO,  /*!< Information描述正常事件流程的消息 */
        LOG_DEBUG, /*!< 普通使用不需要的额外信息(值、指针、大小等)。 */
    } LOG_LEVEL_T;

    static const std::string DefaultProjectPath = "/home/user/my_muduo/My_mutuo/";

    static std::string LogTimestamp()
    {
        time_t now = time(nullptr);
        struct tm *curr_time = localtime(&now);
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
                    curr_time->tm_hour, curr_time->tm_min, curr_time->tm_sec);

        return buffer;
    }

    static const std::string GetRelativePath(const std::string &fullPath, const std::string &projectpath = DefaultProjectPath)
    {
        auto pos = fullPath.find(projectpath);
        if (pos == std::string::npos)
            return fullPath;

        return fullPath.substr(pos + projectpath.length());
    }

    static const std::string glogfile = "../log/log.txt";
    std::mutex _mutex;
    class Log
    {
    public:
        Log(const std::string &logfile = glogfile) : _type(SCREEN_TYPE), _logfile(logfile) {}

        void Enable(int type) { _type = type; }

        void FlushLogToScreen(const std::string& log)
        {   
            std::cout << log;
        }

        void FlushLogToFile(const std::string& log)
        {
            std::ofstream out(_logfile, std::ios::app);
            if (!out.is_open())
                return;
            out.write(log.c_str(), log.size());
            out.close();
        }

        void Flushlog(const std::string& log)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            switch (_type)
            {
            case SCREEN_TYPE:
                FlushLogToScreen(log);
                break;
            case FILE_TYPE:
                FlushLogToFile(log);
                break;
            }
        }

        void LogWrite(const char *format, ...)
        {
            va_list ap;
            va_start(ap, format);
            char log_info[1024];
            memset(&log_info, 0, sizeof(log_info));
            vsnprintf(log_info, sizeof(log_info), format, ap);
            va_end(ap);
            Flushlog(log_info);
        }

        ~Log()  {}

    private:
        int _type;
        std::string _logfile;
    };

    Log lg;

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ##letter #letter " [%s][%s](%d)(%p): " format LOG_RESET_COLOR "\n"

#if defined(__cplusplus) && (__cplusplus >  201703L)
#define LOG_LEVEL(level, format, ...) do {                     \
        if (level == LOG_ERROR )          { lg.LogWrite(LOG_FORMAT(E, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, getpid(), __VA_OPT__(,) __VA_ARGS__); } \
        else if (level == LOG_WARN )      { lg.LogWrite(LOG_FORMAT(W, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, getpid(), __VA_OPT__(,) __VA_ARGS__); } \
        else if (level == LOG_DEBUG )     { lg.LogWrite(LOG_FORMAT(D, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, getpid(), __VA_OPT__(,) __VA_ARGS__); } \
        else                              { lg.LogWrite(LOG_FORMAT(I, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, getpid(), __VA_OPT__(,) __VA_ARGS__); } \
    } while(0)
#else // !(defined(__cplusplus) && (__cplusplus >  201703L))
#define LOG_LEVEL(level, format, ...) do {                     \
        if (level == LOG_ERROR )          { lg.LogWrite(LOG_FORMAT(E, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, (void*)pthread_self(), ##__VA_ARGS__); } \
        else if (level == LOG_WARN )      { lg.LogWrite(LOG_FORMAT(W, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, (void*)pthread_self(), ##__VA_ARGS__); } \
        else if (level == LOG_DEBUG )     { lg.LogWrite(LOG_FORMAT(D, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, (void*)pthread_self(), ##__VA_ARGS__); } \
        else                              { lg.LogWrite(LOG_FORMAT(I, format), LogTimestamp().c_str(), GetRelativePath(__FILE__).c_str(), __LINE__, (void*)pthread_self(), ##__VA_ARGS__); } \
    } while(0)
#endif

#define LOG_LEVEL_LOCAL(level, format, ...) do {   \
        LOG_LEVEL(level, format, ##__VA_ARGS__);   \
    } while(0)

#if defined(__cplusplus) && (__cplusplus >  201703L)
#define LOGE(format, ... ) LOG_LEVEL_LOCAL(LOG_ERROR,   format __VA_OPT__(,) __VA_ARGS__)
#define LOGW(format, ... ) LOG_LEVEL_LOCAL(LOG_WARN ,   format __VA_OPT__(,) __VA_ARGS__)
#define LOGI(format, ... ) LOG_LEVEL_LOCAL(LOG_INFO ,   format __VA_OPT__(,) __VA_ARGS__)
#define LOGD(format, ... ) LOG_LEVEL_LOCAL(LOG_DEBUG,   format __VA_OPT__(,) __VA_ARGS__)
#else // !(defined(__cplusplus) && (__cplusplus >  201703L))
#define LOGE(format, ... ) LOG_LEVEL_LOCAL(LOG_ERROR,   format, ##__VA_ARGS__)
#define LOGW(format, ... ) LOG_LEVEL_LOCAL(LOG_WARN ,   format, ##__VA_ARGS__)
#define LOGI(format, ... ) LOG_LEVEL_LOCAL(LOG_INFO ,   format, ##__VA_ARGS__)
#define LOGD(format, ... ) LOG_LEVEL_LOCAL(LOG_DEBUG,   format, ##__VA_ARGS__)
#endif // !(defined(__cplusplus) && (__cplusplus >  201703L))

#define EnableScreen()          \
    do                          \
    {                           \
        lg.Enable(SCREEN_TYPE); \
    } while (0)
#define EnableFILE()          \
    do                        \
    {                         \
        lg.Enable(FILE_TYPE); \
    } while (0)
}