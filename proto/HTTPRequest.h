#pragma once

#include "TCPServer.h"
#include <regex>

namespace my_muduo
{
    class HTTPRequest
    {
    public:
        std::string _method;                                   // 请求方法
        std::string _path;                                     // 资源路径
        std::string _body;                                     // 协议版本
        std::string _version;                                  // 请求正文
        std::smatch _matches;                                  // 资源路径的正则提取数据
        std::unordered_map<std::string, std::string> _headers; // 头部字段
        std::unordered_map<std::string, std::string> _params;  // 查询字符串
    public:

        HTTPRequest()
            :_version("HTTP/1.1")
        {}

        /**
         * @brief 重置
         * @param 空
         * @return 空
         */
        void ReSet()
        {
            _method.clear();
            _path.clear();
            _body.clear();
            _version = "HTTP/1.1";
            std::smatch match;
            _matches.swap(match);
            _headers.clear();
            _params.clear();
        }

        /**
         * @brief 插入HTTP请求头字段
         * @param key[in]    键
         * @param val[in]    值
         * @return 空
         */
        void SetHeader(const std::string &key, const std::string &val)
        {
            _headers.insert({key, val});
        }

        /**
         * @brief 判断是否存在指定HTTP请求头部字段
         * @param key[in]        键
         * @return 判断结果
         */
        bool HasHeader(const std::string &key) const
        {
            auto it = _headers.find(key);
            if (it == _headers.end())
                return false;

            return true;
        }

        /**
         * @brief 获取HTTP请求头部字段的值
         * @param key[in]        键
         * @return 头部字段的值
         */
        std::string GetHeader(const std::string &key) const
        {
            auto it = _headers.find(key);
            if (it == _headers.end())
                return "";

            return it->second;
        }

        /**
         * @brief 插入查询字符串
         * @param key[in]    键
         * @param val[in]    值
         * @return 空
         */
        void SetParam(std::string &key, std::string &val)
        {
            _params.insert({key, val});
        }

        /**
         * @brief 判断是否有某个指定的查询字符串
         * @param key[in]    键
         * @return 判断结果
         */
        bool HasParam(std::string &key) const
        {
            auto it = _params.find(key);
            if (it == _params.end())
                return false;

            return true;
        }

        /**
         * @brief 获取指定的查询字符串
         * @param key[in]    键
         * @return 查询字符串
         */
        std::string GetParam(std::string &key) const 
        {
            auto it = _params.find(key);
            if (it == _params.end())
                return "";

            return it->second;
        }

        /**
         * @brief 获取正文长度
         * @param url[in]        源URL
         * @param seconvert_space_to_plusq[in]    判断字符是否是 +
         * @return 正文长度
         */
        size_t ContentLength() const 
        {
            bool ret = HasHeader("Content-Length");
            if(ret == false)
                return 0;
            
            std::string clen = GetHeader("Content-Length");
            return std::stoi(clen);
        }

        /**
         * @brief 判断是否是短连接
         * @param 空
         * @return 判断结果
         */
        bool Close() const
        {
            if(HasHeader("Connection") == true && GetHeader("Connection") == "keep-alive")
                return false;
            
            return true;
        }
    };

}
