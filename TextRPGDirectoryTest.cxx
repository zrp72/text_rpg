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
set_directory (std::string original_filepath){
	int last_slash = original_filepath.rfind('\\');
	std::string story_directory = original_filepath.substr(0,last_slash);
	//std::cout << "Original directory: " << std::filesystem::current_path() << '\n';
	//std::cout << "Story directory: " << story_directory << '\n';
	std::filesystem::current_path(story_directory);
}

std::string randomize_lineage(){
    std::string lineage = "";
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
	std::cout << "Lineage: " << lineage << '\n';
    return lineage;
}

class character {
    public: 
	std::map<std::string, int> attributes;
    std::string lineage;
    std::string alignment;
    std::string reputation;

    character():attributes(), lineage(""), alignment(""), reputation(""){};
};

class option {
	public:
	std::string prompt;
	std::string dest_label;
	std::map<std::string, int> stat_change_map;

	option() : prompt(), dest_label(){}
	option(std::string p, std::string dest) : prompt(p), dest_label(dest){}
	option(std::string p, std::string dest, std::map<std::string, int> map) : prompt(p), dest_label(dest), stat_change_map(map){}
};

class paragraph{
	public:
	std::string text;
	std::vector<option> options;

	paragraph() : text(""), options(){};
	paragraph(std::string t) : text(t){}

	void display() {
		std::cout << text << '\n';
		int n = 1;
		for (const option &o : options) {
			std::cout << "  " << n++ << ". " << o.prompt << '\n';
		}
	}
};

class game_data {
	public:
	std::map<std::string, paragraph> story;
	std::string story_location;
	character player;

	game_data(std::map<std::string, paragraph> s, std::string l, character p) : story(s), story_location(l), player(p){};
};

std::map<std::string, paragraph> 
parse_story(const std::string &writtenStory){
	std::map<std::string, paragraph> story_map;
	std::stringstream story(writtenStory);
	std::string line;

	while(std::getline(story, line)){
		int colon = line.find(':');
		if (line.length() == 0 || line.substr(0,2) == "//"){
			continue;
		}
		if (colon == std::string::npos){
			throw std::runtime_error("Delimiter not found");
		}
		//Nexuses
		switch (line[0]){ 
		case '[':{
			int close_bracket = line.find(']');
            std::string destinations;
			std::string label = line.substr(1,close_bracket - 1);
			while (std::getline(story, line) && !line.empty()){
				destinations+= line + '\n';
			}
            story_map[label] = paragraph(destinations);
		}
			break;
		//Options
		case '{':{
			int close_bracket = line.find('}');
			std::string source = line.substr(1,colon - 1);
			std::string destination = line.substr(colon + 1,close_bracket - colon - 1);
			paragraph &origin = story_map[source];
			if (line.substr(close_bracket + 1, 1) == "("){
				int second_colon = line.rfind(':');
				int open_paren = line.find('(');
				int close_paren = line.find(')');
				std::string affected_stat = line.substr(open_paren + 1, second_colon - open_paren - 1);
				int change = std::stoi(line.substr(second_colon + 1, close_paren - second_colon - 1));
				std::map<std::string, int> statchange_map;
				statchange_map[affected_stat] = int(change);
				origin.options.push_back(option(line.substr(close_paren + 1),destination, statchange_map));
			}else{
				origin.options.push_back(option(line.substr(close_bracket + 1),destination));
			}
		}
			break;
		//Paragraphs
		default:{
			std::string tag = line.substr(0,colon);
			std::string text = line.substr(colon + 2);
			while (std::getline(story, line) && !line.empty()){
				text+= '\n' + line;
			}
			story_map[tag] = paragraph(text);
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

std::string get_destination(game_data data, std::string par_nexus){
    std::string destination;
    std::string line;
    std::stringstream text(data.story[par_nexus].text);
	auto attributes = data.player.attributes;
    while (getline(text, line)){
        int equal = line.find('=');
		int colon = line.find(':');
		std::string dest_ver = line.substr(0,equal - 1);
		std::string x = line.substr(equal + 2,colon - equal - 2);

		if (line.substr(colon + 1) != ""){
			int v = std::stoi(line.substr(colon + 1));
			if (attributes[x] >= v){
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

void run_Story(game_data data){
	auto p = data.story[data.story_location];
	int decision = -1;
	while (decision == -1){;
		p.display();
		decision = get_valid_choice(p.options.size());
	}
	
	option chosen_option = p.options[decision - 1];
	std::string next_paragraph = chosen_option.dest_label;
	if (next_paragraph[next_paragraph.length() - 1] == '~'){
		next_paragraph = get_destination(data, next_paragraph);
	}
	if (!chosen_option.stat_change_map.empty()){
		auto stat_change = chosen_option.stat_change_map.begin();
		std::string attribute = stat_change->first;
		std::cout << "Attribute check: " << attribute << '\n'; //TESTING PURPOSES
		std::cout << "Attribute numerator: " << data.player.attributes[attribute] << '\n'; //TESTING PURPOSES
		int difference = stat_change->second;
		data.player.attributes[attribute] = data.player.attributes[attribute] + difference;
		std::cout << "New attribute numerator: " << data.player.attributes[attribute] << '\n'; //TESTING PURPOSES
	}
	data.story_location = next_paragraph;
	//save_game(data);
	run_Story(data);
}

game_data 
load_game(const std::string &writtenStory){
	character player;
    player.lineage = randomize_lineage();
	player.attributes["Rep"] = 0; //TESTING PURPOSES
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