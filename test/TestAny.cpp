#include "Any.h"

using namespace my_muduo;

class Test
{
public:
    Test() { std::cout << "construct" << std::endl; }
    Test(const Test &t) { std::cout << "copy construct" << std::endl; }
    ~Test() { std::cout << "disconstruct" << std::endl; }
};

int main()
{
    Any a;
    {
        Test t;
        a = t;
    }
    a = 10;
    int *pa = a.get<int>();
    std::cout << *pa << std::endl;
    a = std::string("nihao");
    std::string *ps = a.get<std::string>();
    std::cout << *ps << std::endl;
}