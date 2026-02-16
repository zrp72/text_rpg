#include <iostream>
#include <sstream>
#include <map>

int main(){
    std::string current_item = " +5 Potion";
    int operation = current_item.find('+');
    std::cout << "op" << operation << '\n';
    int item_pos = current_item.find(' ', operation) + 1;
    std::cout << item_pos << '\n';
}