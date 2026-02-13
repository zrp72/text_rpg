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
	std::vector<std::string> inventory;

    character():attributes(), lineage(""), inventory(){};
};

class option {
	public:
	std::string prompt;
	std::string dest_label;
	std::map<std::string, int> stat_change_map;
	bool chosen = false;
	bool loopable;
	
	option(std::string p, std::string dest) : prompt(p), dest_label(dest){}
	option(std::string p, std::string dest, std::map<std::string, int> map, bool loop) 
	: prompt(p), dest_label(dest), stat_change_map(map), loopable(loop){}
};

class paragraph{
	public:
	std::string text;
	std::map<std::string, int> stat_change_map;
	std::vector<option> options;
	bool has_been = false;
	bool loopable;

	paragraph() : text(""), stat_change_map(), options(), has_been(), loopable(){};
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

	game_data(std::map<std::string, paragraph> s, std::string l, character p) : story(s), story_location(l), player(p){};
};

std::map<std::string, int>
create_stat_change_map(std::string stat_changes){
	std::stringstream s(stat_changes);
	std::string current_change;
	std::map<std::string, int> statchange_map;

	while(std::getline(s, current_change, ',')){
		int operation = current_change.find('+');
		if (operation == std::string::npos){
			operation = current_change.find('-');
		}
		std::string affected_stat = current_change.substr(1, operation - 2);
		int change = std::stoi(current_change.substr(operation));
		statchange_map[affected_stat] = int(change);
	}
	return statchange_map;
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
		//Options
		case '{':{
			int close_bracket = line.find('}');
			std::string source = line.substr(1,colon - 1);
			std::string destination = line.substr(colon + 1,close_bracket - colon - 1);
			std::string prompt;
			paragraph &origin = story_map[source];
			std::string stat_item_notations = line.substr(close_bracket + 1, 1);

			if (stat_item_notations == " "){
				prompt = line.substr(close_bracket + 1);
				origin.options.push_back(option(prompt, destination));
			}else if (stat_item_notations == "("){
				int open_paren = line.find('(');
				int close_paren = line.find(')');
				bool loopable = false;

				if (line.substr(close_paren + 1, 1) != "*"){
					prompt = line.substr(close_paren + 1);
				}else{
					loopable = true;
					prompt = line.substr(close_paren + 2);
				}

				std::string stat_changes = line.substr(open_paren, close_paren - open_paren);
				statchange_map = create_stat_change_map(stat_changes);

				origin.options.push_back(option(prompt, destination, statchange_map, loopable));	
			}else if (stat_item_notations == "["){
				
			}
		}
			break;
		//Paragraphs and nexuses
		default:{
			std::string tag;
			std::string text;
	
			if (line[0] != '['){
			//paragraph
				tag = line.substr(0,colon);
				if (line.substr(colon + 1, 1) != "("){
					text = line.substr(colon + 2);
					
				}else{

					int open_paren = line.find('(');
					int close_paren = line.find(')');
					std::string stat_changes = line.substr(open_paren, close_paren);
					statchange_map = create_stat_change_map(stat_changes);
					text = line.substr(close_paren + 2);

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
		data.player.attributes[stat] = data.player.attributes[stat] + difference;
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

		if (line.substr(colon + 1, 1) != ""){
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