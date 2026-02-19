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

#define debug(x)  std::cout << x

class item_instance {
	public:
	std::string item_ID;
	int quantity;
};

class character {
    public: 
	std::map<std::string, int> attributes = {
		{"Reputation", 0},
		{"Strenth", 0},
		{"Agility", 0}
	};
    std::string lineage;
	std::vector<item_instance> inventory;
	int inventory_size = 30;

    character():attributes(){};
};

class location {
	public:
	std::vector<std::string> dropped_loot;
};

class option {
	public:
	std::string prompt;
	std::string dest_label;
	std::map<std::string, int> stat_change_map;
    std::map<std::string, int> item_change_map;
	
	option(std::string p, std::string dest) : prompt(p), dest_label(dest){}
};

class paragraph{
	public:
	std::string text;
	std::string location;
	std::map<std::string, int> stat_change_map;
    std::map<std::string, int> item_change_map;
	std::vector<option> options;
	//for nexuses
	bool nexus = false;
	int visits = 0; 

	paragraph() : text(""){}
	paragraph(std::string t, std::string l) : text(t), location(l){}
	paragraph(std::string t, std::string l, bool n) : text(t), location(l), nexus(n){}

	void display() {
		std::cout << text << '\n';
		int n = 1;
		for (const option &o : options) {
			std::cout << "  " << n++ << ". " << o.prompt << '\n';
		}
	}
};

class item {
	public:
	std::string ID;
	std::string item_type;
	std::map<std::string, int> stat_bonus;

	item(): ID(""){}
	item(std::string i) : ID(i){}
};

class files {
	public:
	std::string story;
	std::string items;

	files(std::string s, std::string i): story(s), items(i){};
};

class game_data {
	public:
	std::map<std::string, paragraph> story;
	std::string story_location;
	std::map<std::string, item> items;
	character player;

	game_data(auto s, std::string l, auto i, character p) 
	: story(s), story_location(l), items(i), player(p){};
};

void 
set_directory (std::filesystem::path original_filepath){
	std::filesystem::path story_directory = original_filepath.parent_path();
	//std::cout << "Original directory: " << std::filesystem::current_path() << '\n';
	//std::cout << "Story directory: " << story_directory << '\n';
	std::filesystem::current_path(story_directory);
}

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
	std::cout << "Lineage: " << lineage << '\n';
    return lineage;
}

int
find_operation(std::string line){
	int operation = line.find_first_of("+-");
	if (operation == std::string::npos){
		debug("Operator not found: " << '\n');
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
		std::cout << line_start << '\n';
        std::string affected_stat = current_change.substr(line_start, operation - 2);
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
	if (id.substr(0,3) == "Opt"){
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
	std::string source = line.substr(1,colon - 1);
	std::string destination = line.substr(colon + 1,close_bracket - colon - 1);
	std::string prompt;
	paragraph &origin = story_map[source];
    std::string stat_inv_changes;

	switch (line[close_bracket + 1]){
	case ' ':{
		prompt = line.substr(close_bracket + 1);
	}
	break;
	case '(':{
		int open_paren = line.find('(');
		int close_paren = line.find(')');
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
    origin.options.push_back(option(prompt, destination));

	if (!stat_inv_changes.empty()){
		int p = origin.options.size();
    	std::string option_id = "Option(" + source + ")" + std::to_string(p);
        auto maps = initialize_stat_inv_changes(stat_inv_changes);
		attach_change_maps(maps, option_id, story_map);
	}
}

void parse_paragraph(std::stringstream &story, std::string line, std::map<std::string, paragraph> &story_map) {
	int colon = line.find(':');
	std::string tag = line.substr(0,colon);
	std::string text;
	std::string location;
	std::string stat_inv_changes;
	if (tag.find('(')) {
		int variant = tag.find('(');
		location = tag.substr(0, variant);
	} else {
		location = tag;
	}
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
	story_map[tag] = paragraph(text, location);
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
	std::string location = tag;
	std::string text;
	while (std::getline(story, line) && !line.empty()) {
		text += line + '\n';
	}
	story_map[tag] = paragraph(text, location, nexus);
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
		default: {
			if (line[0] != '[') {
				parse_paragraph(story, line, story_map);
			} else {
				parse_nexus(story, line, story_map);
			}
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
		if (c.substr(0, 2) == "ID"){
			item.ID = c.substr(colon + 2);
		}
		if(c.substr(0, colon) == "Stats"){
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

int 
get_valid_choice(int max){
	int decision;
	std::cin >> decision;
	if (std::cin.fail() || decision <= 0 || decision > max){
		std::cerr << "Invalid choice: " << decision << "\nMax: " << max << '\n';
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize> ::max(), '\n');
		decision = -1;
	}
	return decision;
}

std::string
get_destination(game_data &data, paragraph &par_nexus){

    std::string destination;
    std::string line;
    std::stringstream text(par_nexus.text);
	++par_nexus.visits;
	auto attributes = data.player.attributes;

    while (getline(text, line) && !line.empty()){
		int colon = line.find(':');
		int line_start = line.find_first_not_of(' ');
		int dest_tag_end = line.find_first_of(" =", line_start + 1);
		std::string dest_ver = line.substr(line_start, dest_tag_end);

		if (colon == std::string::npos){
			destination = par_nexus.location + '(' + dest_ver + ')';
		}else{
			int conditions_start = line.find_first_not_of(" =", dest_tag_end);
			std::stringstream conds(line.substr(conditions_start));
			std::string c;
			bool viable = true;
			while (viable && std::getline(conds, c, ',')){
				int c_start = line.find_first_not_of(' ');
				int comparator = c.find(':');
				int c_end = line.find_last_not_of(' ');

				std::string stat = c.substr(c_start, comparator - c_start);
				int value = std::stoi(c.substr(comparator + 1, c_end + 1));

				if (stat == "Visits"){
					viable = par_nexus.visits >= value;
					continue;
				}

				if (attributes.contains(stat)){
					viable = attributes[stat] >= value;
				}else{
					debug(par_nexus.location << dest_ver << " nexus has unrecognized stat check." << '\n');
					viable = false;
				}
			}
			if (viable){
            	destination = par_nexus.location + '(' + dest_ver + ')';
        	}
		}
    }
    return destination;
}

/*void 
save_game(game_data active_data){
	std::ofstream save_data;
	save_data.open("save_data");
	save_data.close();
}*/

void 
alter_inventory(game_data &data, auto &st){
	int open_inventory_space = data.player.inventory_size - data.player.inventory.size();
	int map_size = st.item_change_map.size();
	if ((open_inventory_space - map_size) < 0){
		debug("No inventory" << '\n');
	}
	for (const auto &[c_item, difference] : st.item_change_map){
		bool applied = false;
		for (auto &i : data.player.inventory){
			if (i.item_ID == c_item){
				i.quantity += difference;
				applied = true;
				debug(i.item_ID << ' ' << i.quantity << '\n');
				break;
			}
		}
		if (!applied && open_inventory_space > 0){
			data.player.inventory.push_back(item_instance{c_item, difference});
			debug("New item: " << c_item << '\n');
		}
	}
}

//handles option and paragraph change maps, so auto is NEEDED
void
check_for_change_maps(game_data &data, auto &search_target){
	auto &st = search_target;
	if (!st.stat_change_map.empty()){
		for (const auto &[target, difference] : st.stat_change_map) {
			debug(target << ": " << data.player.attributes[target] << '\n'); 
			data.player.attributes[target] = data.player.attributes[target] + difference;
			debug(target << ": " << data.player.attributes[target] << '\n'); 
		}
	}
	if (!st.item_change_map.empty()){
		alter_inventory(data, st);
	}
}

void run_Story(game_data &data){
	paragraph p = data.story[data.story_location];
	debug("Location: " << p.location << '\n');

	check_for_change_maps(data, p);
	
	int decision = -1;
	while (decision == -1){;
		p.display();
		decision = get_valid_choice(p.options.size());
	}
	
	option c = p.options[decision - 1];
	debug("Dest label: " << c.dest_label << '\n');
	std::string next_paragraph_tag = c.dest_label;

	check_for_change_maps(data, c);

	if (data.story[next_paragraph_tag].nexus){
		next_paragraph_tag = get_destination(data, data.story[next_paragraph_tag]);
	}

	data.story_location = next_paragraph_tag;
	//save_game(data);
	run_Story(data);
}

game_data 
load_game(const files &files){
	character player;
    player.lineage = randomize_lineage();
	auto story = parse_story(files.story);
	auto items = load_items(files.items);
	game_data data(story, "Start", items, player);

	return data;
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

int main(int argc, char** argv){
	set_directory(argv[0]);

    try {
		std::string story = open_file("Story.txt");
		std::string items = open_file("ItemCatalog.txt");
		files opened_files(story, items);
		game_data loaded_data = load_game(opened_files);
		run_Story(loaded_data);
	} catch (std::exception &e) {
		std::cerr << "err: " << e.what() << '\n';
		return 1;
	}
	return 0;
}