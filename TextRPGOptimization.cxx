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

class character {
    public: 
	std::map<std::string, int> attributes;
    std::string lineage;
	std::map<std::string, int> inventory;

    character():attributes(), lineage(""), inventory(){};
};

class option {
	public:
	std::string prompt;
	std::string dest_label;
	std::map<std::string, int> stat_change_map;
    std::map<std::string, int> items;
	bool chosen = false;
	bool loopable;
	
	option(std::string p, std::string dest) : prompt(p), dest_label(dest){}
	option(std::string p, std::string dest, bool loop) 
	: prompt(p), dest_label(dest), loopable(loop){}
};

class paragraph{
	public:
	std::string text;
	std::map<std::string, int> stat_change_map;
    std::map<std::string, int> items;
	std::vector<option> options;
	bool has_been = false;
	bool loopable;

	paragraph() : text(""), stat_change_map(), items (), options(), has_been(), loopable(){};
	paragraph(std::string t) : text(t){}
	paragraph(std::string t, auto scm) : text(t), stat_change_map(scm){};

	void display() {
		std::cout << text << '\n';
		int n = 1;
		for (const option &o : options) {
			std::cout << "  " << n++ << ". " << o.prompt << '\n';
		}
	}
};

class items {
	public:
	std::string item_type;
	std::vector<int> stat_bonus;

	items() : item_type(""), stat_bonus(){}
};

class game_data {
	public:
	std::map<std::string, paragraph> story;
	std::string story_location;
	std::map<std::string, items> items;
	character player;

	game_data(std::map<std::string, paragraph> s, std::string l, character p) 
	: story(s), story_location(l), player(p){};
};

int
find_operation(std::string line){
	std::map<int, char> operations = {
		{0, '-'}
	};
	int op_pos = 0;
	int operation = line.find('+');
    while (operation == std::string::npos){
        operation = line.find(operations[op_pos]);
		op_pos++;
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
		int item_pos = current_item.rfind(' ') + 1;
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
        std::string affected_stat = current_change.substr(1, operation - 2);
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
        story_map[source].options[opt_num - 1].items = maps[1];
    }else{
        story_map[id].stat_change_map = maps[0];
        story_map[id].items = maps[1];
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

std::map<std::string, paragraph> 
parse_story(const std::string &writtenStory){
	std::map<std::string, paragraph> story_map;
	std::stringstream story(writtenStory);
	std::string line;

	while(std::getline(story, line)){
		int colon = line.find(':');
		std::map<std::string, int> statchange_map;

		if (line.length() == 0 || line.substr(0,2) == "//"){
			continue;
		}
		if (colon == std::string::npos){
			throw std::runtime_error("Delimiter not found");
		}

		switch (line[0]){ 
		case '{':{
			parse_option(story_map, line);	
		}
		break;
		default:{
		//Paragraphs and nexuses
			std::string tag;
			std::string text;
            std::string stat_inv_changes;
	
			if (line[0] != '['){
			//paragraph
				tag = line.substr(0,colon);

				switch (line[colon + 1]){
				case ' ':{
					text = line.substr(colon + 2);
				}
				break;
				case '(':{
					int open_paren = line.find('(');
                    int close_paren = line.find(')');
                    if (line[close_paren + 1] != '['){
                        text = line.substr(close_paren + 2);
                        stat_inv_changes = line.substr(open_paren, close_paren - open_paren);
                    }else{
                        int close_square = line.find(']');
                        text = line.substr(close_square + 2);
                        stat_inv_changes = line.substr(open_paren, close_square - open_paren);
                    }
				}
				break;
				case '[':{
					int open_square = line.find('[');
                    int close_square = line.find(']');
                    text = line.substr(close_square + 2);
                    stat_inv_changes = line.substr(open_square, close_square - open_square);
				}
				}

				while (std::getline(story, line) && !line.empty()){
						text+= '\n' + line;
				}
			}else{
			//nexus
				int close_bracket = line.find(']');
				tag = line.substr(1,close_bracket - 1);
				while (std::getline(story, line) && !line.empty()){
				text+= line + '\n';
				}
			}
			story_map[tag] = paragraph(text, statchange_map);
            if (!stat_inv_changes.empty()){
                auto maps = initialize_stat_inv_changes(stat_inv_changes);
				attach_change_maps(maps, tag, story_map);
            }
		}
		}
	}
	return story_map;
}

int get_valid_choice(int max){
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

void change_stats(game_data &data, auto stat_change_map){
	for (const std::pair<std::string, int> &it : stat_change_map) {
        std::string stat = it.first;
		int difference = it.second;
		std::cout << stat << ": " << data.player.attributes[stat] << '\n'; 
		data.player.attributes[stat] = data.player.attributes[stat] + difference;
		std::cout << stat << ": " << data.player.attributes[stat] << '\n'; 
	}
}

std::string get_destination(game_data &data, std::string par_nexus){
    std::string destination;
    std::string line;
    std::stringstream text(data.story[par_nexus].text);
	auto attributes = data.player.attributes;
    while (getline(text, line) && !line.empty()){
        int equal = line.find('=');
		int colon = line.find(':');
		std::string dest_ver = line.substr(0,equal - 1);

		if (line[colon + 1] != '-'){
			std::stringstream text(line.substr(equal + 1));
			std::string c;
			bool viable = true;
			while (viable && std::getline(text, c, ',')){
				int ccolon = c.find(':');
				std::string stat = c.substr(1, ccolon - 1);
				int value = std::stoi(c.substr(ccolon + 1));
				viable = attributes[stat] >= value;
			}
			if (viable){
            	destination = par_nexus + dest_ver;
        	}
		}else{
			destination = par_nexus + dest_ver;
		}
    }
    return destination;
}

/*void save_game(game_data active_data){
	std::ofstream save_data;
	save_data.open("save_data");
	save_data.close();
}*/

void run_Story(game_data &data){
	auto p = data.story[data.story_location];

	if (!p.stat_change_map.empty() && (!p.has_been || p.has_been && p.loopable)){
		change_stats(data, p.stat_change_map);
	}

	int decision = -1;
	while (decision == -1){;
		p.display();
		decision = get_valid_choice(p.options.size());
	}
	
	option c = p.options[decision - 1];
	std::string next_paragraph = c.dest_label;

	if (!c.stat_change_map.empty() && (!c.chosen || c.chosen && c.loopable)){
		change_stats(data, c.stat_change_map);
	}

	if (next_paragraph[next_paragraph.length() - 1] == '~'){
		next_paragraph = get_destination(data, next_paragraph);
	}
	data.story_location = next_paragraph;
	//save_game(data);
	run_Story(data);
}

game_data 
load_game(const std::string &writtenStory){
	character player;
    player.lineage = randomize_lineage();
	auto story = parse_story(writtenStory);
	game_data data(story, "Start", player);

	return data;
}


int main(int argc, char** argv){
	set_directory(argv[0]);

    try {
		std::ifstream story_file("Story.txt");
		if (!story_file) {
    		std::cerr << "Error opening file: " 
                  << std::strerror(errno); 
    		return 1;
		}
		std::stringstream story;
		story << story_file.rdbuf();
		game_data loaded_data = load_game(story.str());
		run_Story(loaded_data);
	} catch (std::exception &e) {
		std::cerr << "err: " << e.what() << '\n';
		return 1;
	}
	return 0;
}