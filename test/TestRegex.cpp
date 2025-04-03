#include <regex>
#include <iostream>

int main()
{
    std::string str = "GET /baidu/numbers/login?user=xiaoming&pass=123231 HTTP/1.1";
    std::regex e("(GET|HEAD|POST|PUT|DELETE) ([^?]*)(?:\\?(.*))? (HTTP/1\\.[01])(?:\n|\r\n)");
    // (GET|HEAD|POST|PUT|DELETE) 表示匹配并提取其中任意一个字符串
    // [^?]     匹配非？字符
    // *        零次或多次
    // \\?(.*)  \\? 表示原始的?字符 (.*)表示提取？之后的任意字符零次或多次，直到遇到空格
    //HTTP/1\\.[01] 表示匹配以HTTP/1.开始，后面有个0或1的字符串
    // (?:\n|\r\n)  (?...)表示匹配某个字符串，但是不提取
    std::smatch matches;

    bool ret = std::regex_match(str, matches, e);
    if(ret == false)
        return -1;

    for(auto &s : matches)
        std::cout << s << std::endl;

    return 0;
}
