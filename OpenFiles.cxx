#include <cstring>
#include <fstream>
#include <filesystem>

#include "functions.h"

namespace fs = std::filesystem;

std::string
open_file(fs::path path){
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Error opening file: "
			<< std::strerror(errno);
	}
	std::stringstream read_file;
	read_file << file.rdbuf();
	return read_file.str();
}

files::files(){
	this->story = open_file(fs::path("Story.txt"));
	this->items = open_file(fs::path("ItemCatalog.txt"));
}