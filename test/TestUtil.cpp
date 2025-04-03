#include "Util.h"

using namespace my_muduo;

int main()
{
    std::string buf;
    bool ret = Util::ReadFile("../TCPClient.cpp", &buf);
    if (ret == false)
        LOGE("error!");

    std::cout << buf << std::endl;
    ret = Util::WriteFile("../mimecopy.txt", buf);
    if (ret == false)
        LOGE("error!");

    std::string str = "C++";
    std::string res = Util::UrlEncode(str, false);
    std::cout << res << std::endl;
    std::string ans = Util::UrlDecode(res, false);
    std::cout << ans << std::endl;

    std::cout << Util::IsRegular("../proto") << std::endl;
    std::cout << Util::IsDirectory("../proto") << std::endl;
    return 0;
}
