#include <iostream>
#include <sstream>
#include <map>

int main(){
    class location{
        public:
        std::string a = "!!!";

        location():a(a){}
    };
    std::string test = "a";
    location location;
    std::cout << location.[test];
}