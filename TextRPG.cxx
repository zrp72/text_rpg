#include <iostream>
#include <random>
#include <vector>
#include <string>

std::string lineage = "";

void randomize_lineage(){
    static const std::vector<std::string> options = {
        "Human",
        "Elf",
        "Dwarf",
        "Orc",
        "Halfling",
        "Tiefling",
        "Gnome"
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, options.size() - 1);
    lineage = options[dist(gen)];
}

int main(){
    for(int i = 0; i < 1000; i++){
    randomize_lineage();
    std::cout << "Lineage: " << lineage << '\n';
    }
    return 0;
}