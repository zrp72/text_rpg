#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <limits>
#include <fstream>

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

	paragraph():text(""), options(){};
	paragraph(std::string t) : text(t) {}

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
	std::map<std::string, paragraph> paragraphs;
	std::stringstream story(writtenStory);
	std::string line;

	while(std::getline(story, line)){
		int colon = line.find(':');
		int space = line.find(' ');
		if (line.length() == 0){
			continue;
		}
		if (colon == std::string::npos){
			throw std::runtime_error("Delimiter not found");
		}
		if (line[colon + 1] == ' '){
			std::string tag = line.substr(0,colon);
			std::string text = line.substr(colon + 2);
			while (std::getline(story, line) && !line.empty()) {
				text+= '\n' + line;
			}
			paragraphs[tag] = paragraph(text);
		}
		else if (line[colon + 1] != ' '){
			std::string source = line.substr(0,colon);
			std::string destination = line.substr(colon + 1,space - colon - 1);
			paragraph &origin = paragraphs[source];
			origin.options.push_back(option(line.substr(space + 1),destination));
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

void run_Story(std::map<std::string, paragraph> paragraphs, std::string current_paragraph){
	auto p = paragraphs[current_paragraph];
	int decision = -1;
	while (decision == -1){
		p.display();
		decision = get_valid_choice(p.options.size());
	}
	auto next_paragraph = p.options[decision - 1].dest_label;
	run_Story(paragraphs, next_paragraph);
};

int main() {
	try {
		std::ifstream story_file("Story.txt");
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