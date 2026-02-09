#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <limits>
#include <random>
#include <fstream>

#include <cerrno>
#include <cstring>
#include <filesystem>

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

	option():prompt(""), dest_label(""){};
	option(std::string p, std::string dest) : prompt(p), dest_label(dest) {}
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

std::map<std::string, paragraph> 
loadStory(const std::string &writtenStory){
    character main_character;
    main_character.lineage = randomize_lineage();
    std::cout << "Lineage: " << main_character.lineage << '\n';

	std::map<std::string, paragraph> paragraphs;
	std::stringstream story(writtenStory);
	std::string line;

	while(std::getline(story, line)){
		int colon = line.find(':');
		if (line.length() == 0){
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
            paragraphs[label] = paragraph(destinations);
		}
			break;
		//Options
		case '{':{
			int close_bracket = line.find('}');
			std::string source = line.substr(1,colon - 1);
			std::string destination = line.substr(colon + 1,close_bracket - colon - 1);
			paragraph &origin = paragraphs[source];
			origin.options.push_back(option(line.substr(close_bracket + 1),destination));
		}
			break;
		//Paragraphs
		default:{
			std::string tag = line.substr(0,colon);
			std::string text = line.substr(colon + 2);
			while (std::getline(story, line) && !line.empty()){
				text+= '\n' + line;
			}
			paragraphs[tag] = paragraph(text);
		}
		}
	}
	return paragraphs;
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

std::string get_destination(std::map<std::string, paragraph> story_map, std::string par_nexus){
    std::string destination;
    std::string line;
    std::stringstream routes(story_map[par_nexus].text);
	std::map<std::string, int> attributes;
	attributes["Rep"] = int(1);
    while (getline(routes, line)){
        int equal = line.find('=');
		int colon = line.find(':');
		std::string dest_ver = line.substr(0,equal - 1);
		std::string x = line.substr(equal + 2,colon - equal - 2);
		int v = std::stoi(line.substr(colon + 1));

        if (attributes[x] > v){
            destination = par_nexus.substr(0,par_nexus.length() - 1) + dest_ver;
        }
    }
    return destination;
}

void run_Story(std::map<std::string, paragraph> paragraphs, std::string current_paragraph){
	auto p = paragraphs[current_paragraph];
	int decision = -1;
	while (decision == -1){;
		p.display();
		decision = get_valid_choice(p.options.size());
	}
	auto next_paragraph = p.options[decision - 1].dest_label;
	if (next_paragraph[next_paragraph.length() - 1] == '~'){
		next_paragraph = get_destination(paragraphs, next_paragraph);
	}
	run_Story(paragraphs, next_paragraph);
}

int main(int argc, char** argv){
	std::string file_path(argv[0]);
	int last_slash = file_path.rfind('\\');
	std::string story_directory = file_path.substr(0,last_slash);
	std::cout << "Original filesystem: " << std::filesystem::current_path() << '\n';
	std::cout << "Story directory: " << story_directory << '\n';

    try {
		std::filesystem::current_path(story_directory);
		std::ifstream story_file("Story.txt");
		if (!story_file) {
    		std::cerr << "Error opening file: " 
                  << std::strerror(errno); 
    		return 1;
		}
		std::stringstream written_story;
		written_story << story_file.rdbuf();
		auto story = loadStory(written_story.str());
		run_Story(story, "Start");
	} catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}