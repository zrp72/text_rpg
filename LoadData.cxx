#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <limits>
#include <random>
#include <filesystem>

#include <stdexcept> //may not be needed
#include <cerrno>
#include <cstring>

#include "classes.h"

#define debug(x)  std::cout << x

void
lowercase_string (std::string& text){
	std::transform(text.begin(), text.end(), text.begin(),
		[](unsigned char c){ return std::tolower(c); }
	);
}

/*

std::string randomize_lineage(){
	std::string lineage = "";
	const std::vector<std::string> options = {
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
	debug("Lineage: " << lineage << '\n');
	return lineage;
}

int
find_operation(std::string line){
	int operation = line.find_first_of("+-");
	if (operation == std::string::npos){
		debug("Mathematic operator not found: " << '\n');
	}
	return operation;
}

std::map<std::string, int>
create_item_map(std::string changes){
	std::stringstream i(changes);
	std::string current_item;
	std::map<std::string, int> item_map;
	while(std::getline(i, current_item, ',')){
		int operation = find_operation(current_item);
		int item_pos = current_item.find(' ', operation) + 1;
		int amount = std::stoi(current_item.substr(operation, item_pos - 1 - operation));
		std::string item = current_item.substr(item_pos);
		lowercase_string(item);
		item_map[item] = amount;
	}
	return item_map;
}

std::map<std::string, int>
create_stat_map(std::string changes){
	std::stringstream s(changes);
	std::string current_change;
	std::map<std::string, int> statchange_map;
	while(std::getline(s, current_change, ',')){
		int operation = find_operation(current_change);
		size_t line_start = current_change.find_first_not_of(" (");
		std::string affected_stat = current_change.substr(line_start, operation - 2);
		lowercase_string(affected_stat);
		int change = std::stoi(current_change.substr(operation));
		statchange_map[affected_stat] = change;
	}
	return statchange_map;
}

std::array<std::map<std::string, int>, 2>
initialize_stat_inv_changes(std::string changes){
	std::array<std::map<std::string, int>, 2> maps;
	int open_square = changes.find('[');

	if (changes[0] != '['){
		if (open_square == std::string::npos) {
			maps[0] = create_stat_map(changes);
		} else {
			std::string stat_changes = changes.substr(0, open_square - 1);
			std::string item_changes = changes.substr(open_square);
			maps[0] = create_stat_map(stat_changes);
			maps[1] = create_item_map(item_changes);
		}
	}else{
		maps[1] = create_item_map(changes);
	}
	return maps;
}

void
attach_change_maps(auto maps, std::string id, auto &story_map){
	if (id.substr(0,3) == "opt"){
		int open_paren = id.find('(');
		int close_paren = id.find(')');
		std::string source = id.substr(open_paren + 1, close_paren - open_paren - 1);
		int opt_num = std::stoi(id.substr(close_paren + 1));
		story_map[source].options[opt_num - 1].stat_change_map = maps[0];
		story_map[source].options[opt_num - 1].item_change_map = maps[1];
	}else{
		story_map[id].stat_change_map = maps[0];
		story_map[id].item_change_map = maps[1];
	}
}

void
parse_option(auto &story_map, std::string line){
	int colon = line.find(':');
	int close_bracket = line.find('}');
	std::string parent_paragraph = line.substr(1,colon - 1);
	std::string destination = line.substr(colon + 1,close_bracket - colon - 1);
	std::string prompt;
	std::string stat_inv_changes;

	switch (line[close_bracket + 1]){
	case ' ':{
		prompt = line.substr(close_bracket + 1);
	}
	break;
	case '(':{
		int open_paren = line.find('(', close_bracket);
		int close_paren = line.find(')', open_paren);
		if (line[close_paren + 1] != '['){
			prompt = line.substr(close_paren + 1);
			stat_inv_changes = line.substr(open_paren, close_paren - open_paren);
		}else{
			int close_square = line.find(']');
			prompt = line.substr(close_square + 1);
			stat_inv_changes = line.substr(open_paren, close_square - open_paren);
		}
	}
	break;
	case '[':{
		int open_square = line.find('[');
		int close_square = line.find(']');
		prompt = line.substr(close_square + 1);
		stat_inv_changes = line.substr(open_square, close_square - open_square);
	}
	}

	destination = lowercase_string(destination);
	parent_paragraph = lowercase_string(parent_paragraph);
	story_map[parent_paragraph].options.push_back(option(prompt, destination));

	if (!stat_inv_changes.empty()){
		int p = story_map[parent_paragraph].options.size();
		std::string option_id = "option(" + parent_paragraph + ")" + std::to_string(p);
		auto maps = initialize_stat_inv_changes(stat_inv_changes);
		attach_change_maps(maps, option_id, story_map);
	}
}

void parse_paragraph(std::stringstream &story, std::string line, std::map<std::string, paragraph> &story_map) {
	int colon = line.find(':');
	std::string tag = line.substr(0,colon);
	std::string text;
	std::string stat_inv_changes;
	switch (line[colon + 1]) {
		case ' ': {
			text = line.substr(colon + 2);
		} break;
		case '(': {
			int open_paren = line.find('(', colon);
			int close_paren = line.find(')', colon);
			if (line[close_paren + 1] != '[') {
				text = line.substr(close_paren + 2);
				stat_inv_changes = line.substr(open_paren, close_paren - open_paren);
			} else {
				int close_square = line.find(']', colon);
				text = line.substr(close_square + 2);
				stat_inv_changes = line.substr(open_paren, close_square - open_paren);
			}
		} break;
		case '[': {
			int open_square = line.find('[', colon);
			int close_square = line.find(']', colon);
			text = line.substr(close_square + 2);
			stat_inv_changes = line.substr(open_square, close_square - open_square);
		} break;
	}
	while (std::getline(story, line) && !line.empty()) {
		text += '\n' + line;
	}
	lowercase_string(tag);
	story_map[tag] = paragraph(text);
	if (!stat_inv_changes.empty()) {
		auto maps = initialize_stat_inv_changes(stat_inv_changes);
		attach_change_maps(maps, tag, story_map);
	}
}

void parse_nexus(std::stringstream &story, std::string line, std::map<std::string, paragraph> &story_map) {
	int colon = line.find(':');
	bool nexus = true;
	int close_bracket = line.rfind(']', colon);
	std::string tag = line.substr(1, close_bracket - 1);
	std::string text;

	while (std::getline(story, line) && !line.empty()) {
		text += line + '\n';
	}

	lowercase_string(tag);
	lowercase_string(text);
	story_map[tag] = paragraph(text, nexus);
}

std::map<std::string, paragraph>
parse_story(const std::string &writtenStory) {
	std::map<std::string, paragraph> story_map;
	std::stringstream story(writtenStory);
	std::string line;
	while (std::getline(story, line)) {
		int colon = line.find(':');
		if (line.length() == 0 || line.substr(0,2) == "//") {
			continue;
		}
		if (colon == std::string::npos) {
			throw std::runtime_error("Delimiter not found");
		}
		switch (line[0]) {
		case '{': {
			parse_option(story_map, line);
		} break;
		case '[': {
			parse_nexus(story, line, story_map);
		} break;
		default: {
			parse_paragraph(story, line, story_map);
		} break;
		}
	}
	return story_map;
}

item
read_item_data(std::string &item_data){
	std::stringstream data(item_data);
	std::string c;
	item item;
	while (getline(data, c) && c[0] != '_'){
		int colon = c.find(':');
		if (c.substr(0, 2) == "id"){
			item.ID = c.substr(colon + 2);
		}
		if(c.substr(0, colon) == "stats"){
			std::string changes;
			std::string line;
			while (getline(data, line) && !line.empty()){
				changes+= line;
			}
			item.stat_bonus = create_stat_map(changes);
		}
	}
	return item;
}

std::map<std::string, item>
load_items(const std::string &itemCatalog){
	std::stringstream i(itemCatalog);
	std::string line;
	std::map<std::string, item> items;
	while(std::getline(i, line)){
		lowercase_string(line);
		if (line.length() == 0 || line.substr(0,2) == "//" || line[0] == '_'){
			continue;
		}
		if (line[0] == ' '){
			std::cerr << "Item ID has preceding spaces: "
			<< std::strerror(errno);
		}

		std::string item_data = line;
		while (getline(i, line) && line[0] != '_'){
			item_data+= '\n' + line;
		}
		item loaded_item = read_item_data(item_data);
		items[loaded_item.ID] = loaded_item;

	}
	return items;
}

std::string
open_file(std::string path){
	std::ifstream file(path);
	if (!file) {
		std::cerr << "Error opening file: "
			<< std::strerror(errno);
	}
	std::stringstream read_file;
	read_file << file.rdbuf();
	return read_file.str();
}

files
open_files(){
	std::string story = open_file("Story.txt");
	std::string items = open_file("ItemCatalog.txt");
	files opened_files(story, items);
	return opened_files;
}

game_data
load_game(){
	files files = open_files();
	character player;
	player.lineage = randomize_lineage();
	auto story = parse_story(files.story);
	auto items = load_items(files.items);

	return game_data(story, "start", items, player);
}*/