#include "Buffer.h"

using namespace my_muduo;

void testbuffer1()
{
    Buffer buf;
    for (int i = 0; i < 300; i++)
    {
        std::string str = "hello!!" + std::to_string(i) + '\n';
        buf.WriteStringAndPush(str);
    }

    while (buf.ReadAbleSize() > 0)
    {
        std::string line = buf.GetLineAndPop();
        std::cout << line << std::endl;
    }
}

void testbuffer2()
{
    Buffer buf;
    std::string str = "hello!!";
    buf.WriteStringAndPush(str);

    Buffer buf1;
    buf1.WriteBufferAndPush(buf);

    std::string tmp = buf.ReadAsStringAndPop(buf.ReadAbleSize());

    std::cout << tmp << std::endl;
    std::cout << buf.ReadAbleSize() << std::endl;
    std::cout << buf.ReadAbleSize() << std::endl;
}