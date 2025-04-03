#pragma once

#include "TCPServer.h"
#include <sys/stat.h>

namespace my_muduo
{
    std::unordered_map<int, std::string> _statu_msg =
        {
            {100, "Continue"},
            {101, "Switching Protocol"},
            {102, "Processing"},
            {103, "Early Hints"},
            {200, "OK"},
            {201, "Created"},
            {202, "Accepted"},
            {203, "Non-Authoritative Information"},
            {204, "No Content"},
            {205, "Reset Content"},
            {206, "Partial Content"},
            {207, "Multi-Status"},
            {208, "Already Reported"},
            {226, "IM Used"},
            {300, "Multiple Choice"},
            {301, "Moved Permanently"},
            {302, "Found"},
            {303, "See Other"},
            {304, "Not Modified"},
            {305, "Use Proxy"},
            {306, "unused"},
            {307, "Temporary Redirect"},
            {308, "Permanent Redirect"},
            {400, "Bad Request"},
            {401, "Unauthorized"},
            {402, "Payment Required"},
            {403, "Forbidden"},
            {404, "Not Found"},
            {405, "Method Not Allowed"},
            {406, "Not Acceptable"},
            {407, "Proxy Authentication Required"},
            {408, "Request Timeout"},
            {409, "Conflict"},
            {410, "Gone"},
            {411, "Length Required"},
            {412, "Precondition Failed"},
            {413, "Payload Too Large"},
            {414, "URI Too Long"},
            {415, "Unsupported Media Type"},
            {416, "Range Not Satisfiable"},
            {417, "Expectation Failed"},
            {418, "I'm a teapot"},
            {421, "Misdirected Request"},
            {422, "Unprocessable Entity"},
            {423, "Locked"},
            {424, "Failed Dependency"},
            {425, "Too Early"},
            {426, "Upgrade Required"},
            {428, "Precondition Required"},
            {429, "Too Many Requests"},
            {431, "Request Header Fields Too Large"},
            {451, "Unavailable For Legal Reasons"},
            {501, "Not Implemented"},
            {502, "Bad Gateway"},
            {503, "Service Unavailable"},
            {504, "Gateway Timeout"},
            {505, "HTTP Version Not Supported"},
            {506, "Variant Also Negotiates"},
            {507, "Insufficient Storage"},
            {508, "Loop Detected"},
            {510, "Not Extended"},
            {511, "Network Authentication Required"}};

    std::unordered_map<std::string, std::string> _mime_msg =
        {
            {".aac", "audio/aac"},
            {".abw", "application/x-abiword"},
            {".arc", "application/x-freearc"},
            {".avi", "video/x-msvideo"},
            {".azw", "application/vnd.amazon.ebook"},
            {".bin", "application/octet-stream"},
            {".bmp", "image/bmp"},
            {".bz", "application/x-bzip"},
            {".bz2", "application/x-bzip2"},
            {".csh", "application/x-csh"},
            {".css", "text/css"},
            {".csv", "text/csv"},
            {".doc", "application/msword"},
            {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
            {".eot", "application/vnd.ms-fontobject"},
            {".epub", "application/epub+zip"},
            {".gif", "image/gif"},
            {".htm", "text/html"},
            {".html", "text/html"},
            {".ico", "image/vnd.microsoft.icon"},
            {".ics", "text/calendar"},
            {".jar", "application/java-archive"},
            {".jpeg", "image/jpeg"},
            {".jpg", "image/jpeg"},
            {".js", "text/javascript"},
            {".json", "application/json"},
            {".jsonld", "application/ld+json"},
            {".mid", "audio/midi"},
            {".midi", "audio/x-midi"},
            {".mjs", "text/javascript"},
            {".mp3", "audio/mpeg"},
            {".mpeg", "video/mpeg"},
            {".mpkg", "application/vnd.apple.installer+xml"},
            {".odp", "application/vnd.oasis.opendocument.presentation"},
            {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
            {".odt", "application/vnd.oasis.opendocument.text"},
            {".oga", "audio/ogg"},
            {".ogv", "video/ogg"},
            {".ogx", "application/ogg"},
            {".otf", "font/otf"},
            {".png", "image/png"},
            {".pdf", "application/pdf"},
            {".ppt", "application/vnd.ms-powerpoint"},
            {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
            {".rar", "application/x-rar-compressed"},
            {".rtf", "application/rtf"},
            {".sh", "application/x-sh"},
            {".svg", "image/svg+xml"},
            {".swf", "application/x-shockwave-flash"},
            {".tar", "application/x-tar"},
            {".tif", "image/tiff"},
            {".tiff", "image/tiff"},
            {".ttf", "font/ttf"},
            {".txt", "text/plain"},
            {".vsd", "application/vnd.visio"},
            {".wav", "audio/wav"},
            {".weba", "audio/webm"},
            {".webm", "video/webm"},
            {".webp", "image/webp"},
            {".woff", "font/woff"},
            {".woff2", "font/woff2"},
            {".xhtml", "application/xhtml+xml"},
            {".xls", "application/vnd.ms-excel"},
            {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
            {".xml", "application/xml"},
            {".xul", "application/vnd.mozilla.xul+xml"},
            {".zip", "application/zip"},
            {".3gp", "video/3gpp"},
            {".3g2", "video/3gpp2"},
            {".7z", "application/x-7z-compressed"}};

    class Util
    {
    public:
        /**
         * @brief 字符串分割函数
         * @param src[in]        源字符串
         * @param seq[in]        根据seq进行分割
         * @param arry[out]      将分割出来的子串放到arry中
         * @return 返回分割出的子串数量
         */
        static size_t Split(const std::string &src, const std::string &seq, std::vector<std::string> *arry)
        {
            int offset = 0;
            // abc,bcd,def
            // 有10个字符，offset是查找其实位置，范围应该是0-9，offset == 10表示已经越界了
            while (offset < src.size())
            {
                size_t pos = src.find(seq, offset);
                // 没有找到特定字符，将剩余部分当做一个子串，放入array中。
                if (pos == std::string::npos)
                {
                    arry->push_back(src.substr(offset));
                    return arry->size();
                }
                // 当前子串是一个空串
                if (pos == offset)
                {
                    offset = pos + seq.size();
                    continue;
                }
                arry->push_back(src.substr(offset, pos - offset));
                offset = pos + seq.size();
            }
            return arry->size();
        }

        /**
         * @brief 读取文件所有内容
         * @param filename[in]   文件名
         * @param buf[in]        读取缓冲区
         * @return 读取文件是否成功
         */
        static bool ReadFile(const std::string &filename, std::string *buf)
        {
            std::ifstream ifs(filename, std::ios::binary);
            if (ifs.is_open() == false)
            {
                LOGE("open %s failed!", filename.c_str());
                return false;
            }
            size_t fsize = 0;
            ifs.seekg(0, ifs.end);
            fsize = ifs.tellg();
            ifs.seekg(0, ifs.beg);

            buf->resize(fsize);
            ifs.read(&(*buf)[0], fsize);
            if (ifs.good() == false)
            {
                LOGE("read %s failed!", filename.c_str());
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        /**
         * @brief 向文件写入数据
         * @param filename[in]   文件名
         * @param buf[in]        读取缓冲区
         * @return 读取文件是否成功
         */
        static bool WriteFile(const std::string &filename, const std::string &buf)
        {
            std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
            if (ofs.is_open() == false)
            {
                LOGE("open %s failed!", filename.c_str());
                return false;
            }
            ofs.write(buf.c_str(), buf.size());
            if (ofs.good() == false)
            {
                LOGE("write %s failed!", filename.c_str());
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        /**
         * @brief URL 编码，将特殊的accsii的值，转换为两个16进制字符，前缀% C++ -> C%2B%2B
         * RFC3986规定不编码的特殊字符：. - _ ~ 字母和数字
         * W3C文档中规定，查询字符串中的空格，需要被编码为 +，解码则是 + 转空格
         * @param url[in]        源URL
         * @param seconvert_space_to_plusq[in]    判断字符是否是 +
         * @return 返回编码后的 URL
         */
        static std::string UrlEncode(const std::string url, bool convert_space_to_plus)
        {
            std::string res;
            for (auto &c : url)
            {
                if (c == '.' || c == '-' || c == '_' || c == '~' || isalnum(c))
                {
                    res += c;
                    continue;
                }

                if (c == ' ' && convert_space_to_plus == true)
                {
                    res += '+';
                    continue;
                }

                // 剩下的字符都要编码成为 %HH 格式
                char tmp[4] = {0};
                snprintf(tmp, 4, "%%%02X", c);
                res += tmp;
            }
            return res;
        }

        /**
         * @brief 16进制字符转数字
         * @param c[in]       字符
         * @return 数字
         */
        static char HexToI(char c)
        {
            if (c >= '0' && c <= '9')
                return c - '0';
            else if (c >= 'a' && c <= 'z')
                return c - 'a' + 10;
            else if (c >= 'A' && c <= 'Z')
                return c - 'A' + 10;

            return -1;
        }

        /**
         * @brief URL 解码
         * @param url[in]       源URL
         * @return 返回解码后的 URL
         */
        static std::string UrlDecode(const std::string url, bool convert_space_to_plus)
        {
            std::string res;
            for (int i = 0; i < url.size(); i++)
            {
                if (url[i] == ' ' && convert_space_to_plus)
                    res += '+';

                if (url[i] == '%' && i + 2 < url.size())
                {
                    char v1 = HexToI(url[i + 1]);
                    char v2 = HexToI(url[i + 2]);
                    char v = (v1 << 4) + v2;
                    res += v;
                    i += 2;
                    continue;
                }
                res += url[i];
            }
            return res;
        }

        /**
         * @brief                响应状态码描述信息获取
         * @param statu[in]      状态码
         * @return 返回描述信息
         */
        static std::string StatuDesc(int statu)
        {
            auto it = _statu_msg.find(statu);
            if (it != _statu_msg.end())
                return it->second;
            else
                return "UnKnown";
        }

        /**
         * @brief                根据文件后缀名获取文件mime
         * @param filename[in]   文件名
         * @return 文件mime
         */
        static std::string ExtMime(const std::string &filename)
        {
            size_t pos = filename.find_last_of('.');
            if (pos == std::string::npos)
                // 文件类型是二进制文件
                return "application/octet-stream";

            // 根据扩展名获取mime
            std::string ext = filename.substr(pos);
            auto it = _mime_msg.find(ext);
            if (it == _mime_msg.end())
                return "application/octet-stream";

            return it->second;
        }

        /**
         * @brief                判断一个文件是否是一个目录
         * @param filename[in]   文件名
         * @return 判断结果
         */
        static bool IsDirectory(const std::string &filename)
        {
            struct stat st;
            int ret = stat(filename.c_str(), &st);
            if (ret < 0)
            {
                return false;
            }
            return S_ISDIR(st.st_mode);
        }

        /**
         * @brief                判断一个文件是否是一个普通文件
         * @param filename[in]   文件名
         * @return 判断结果
         */
        static bool IsRegular(const std::string &filename)
        {
            struct stat st;
            int ret = stat(filename.c_str(), &st);
            if (ret < 0)
            {
                return false;
            }
            return S_ISREG(st.st_mode);
        }

        /** http请求的资源路径有效性判断
         * @brief                字符串分割函数
         * @param src[in]        源字符串
         * @param seq[in]        根据seq进行分割
         * @param arry[out]      将分割出来的子串放到arry中
         * @return 返回分割出的子串数量
         */
        static bool ValidPath(const std::string &path)
        {
            // 思想：按照/进行路径分隔，根据有多少字目录，计算目录深度，有多少层，深度不小于0
            std::vector<std::string> subdir;
            Split(path, "/", &subdir);
            int level = 0;
            for (auto &dir : subdir)
            {
                if (dir == "..")
                {
                    level--;
                    //任意一层走出跟目录，就认为有问题。
                    if (level < 0)
                        return false;
                    
                    continue;
                }
                level++;
            }
            return true;
        }
    };
}
