#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
#include <cstring>
#include "basic_functions.h"

int randomize_number(int range){
	std::random_device random_device;
	std::mt19937 generator(random_device());
	std::uniform_int_distribution<size_t> distributor(0, range);
	return distributor(generator);
} 

void
lowercase_string(std::string& text){
	for(char& c : text){
		if(std::isupper(c)){
			c = std::tolower(c);
		}
	}
}

void
move_iterator_past_whitespace(std::string& source, int& c){
	while(std::isspace(source[c])){
		c += 1;
	}
}

std::string
open_txt_file(std::filesystem::path path){
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Error opening file: "
			<< std::strerror(errno);
	}
	std::stringstream read_file;
	read_file << file.rdbuf();
	return read_file.str();
}
