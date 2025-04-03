#pragma once

#include "TCPServer.h"
#include <regex>

namespace my_muduo
{
    class HTTPResponse
    {
    public:
        int _statu;
        bool _redirect_flag;
        std::string _body;
        std::string _redirect_url;
        std::unordered_map<std::string, std::string> _headers;

    public:
        HTTPResponse() : _redirect_flag(false), _statu(200) {}
        HTTPResponse(int statu) : _redirect_flag(false), _statu(statu) {}

        void ReSet()
        {
            _statu = 200;
            _redirect_flag = false;
            _body.clear();
            _redirect_url.clear();
            _headers.clear();
        }

        /**
         * @brief 插入HTTP响应头部字段
         * @param key[in]    键
         * @param val[in]    值
         * @return 空
         */
        void SetHeader(const std::string &key, const std::string &val)
        {
            _headers.insert({key, val});
        }

        /**
         * @brief 判断是否存在指定HTTP响应头部字段
         * @param key[in]        键
         * @return 判断结果
         */
        bool HasHeader(const std::string &key)
        {
            auto it = _headers.find(key);
            if (it == _headers.end())
                return false;

            return true;
        }

        /**
         * @brief 获取HTTP响应头部字段的值
         * @param key[in]        键
         * @return 头部字段的值
         */
        std::string GetHeader(const std::string &key)
        {
            auto it = _headers.find(key);
            if (it == _headers.end())
                return "";

            return it->second;
        }

        /**
         * @brief 设置HTTP响应正文类型
         * @param key[in]        键
         * @return 空
         */
        void SetContent(const std::string &body, const std::string &type = "text/html")
        {
            _body = body;
            SetHeader("Content-type", type);
        }

        /**
         * @brief 设置HTTP响应重定向
         * @param key[in]        键
         * @return 空
         */
        void SetRedirect(const std::string &url, int statu = 302)
        {
            _statu = statu;
            _redirect_flag = true;
            _redirect_url = url;
        }

        /**
         * @brief 判断是否是短连接
         * @param 空
         * @return 判断结果
         */
        bool Close() 
        {
            if (HasHeader("Connection") == true && GetHeader("Connection") == "keep-alive")
                return false;

            return true;
        }
    };
}