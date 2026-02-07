#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <stdexcept>
#include <limits>

std::string writtenStory = R"(
00001: The world before you 

00001:00002 Turn left
00001:00003 Turn right

00002: lava

00003: field
)";

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
		if (line.length() == 0){
			continue;
		}
		else if (line.length() <7 ){
			throw std::runtime_error("Line too short");
		}
		else if (line[5] != ':' ){
			throw std::runtime_error("Identifier not distinguished");
		}
		else if (line[5] == ':' && line[6] == ' '){
			std::string tag = line.substr(0,5);
			std::string text = line.substr(7);
			while (std::getline(story, line) && !line.empty()) {
				text+= '\n' + line;
			}
			paragraphs[tag] = paragraph(text);
		}
		else if (line.length() >= 11 && line[11] == ' '){
			std::string source = line.substr(0,5);
			std::string destination = line.substr(6,5);
			paragraph &origin = paragraphs[source];
			origin.options.push_back(option(line.substr(11),destination));
		}
	}
	return paragraphs;
}

int get_valid_integer(int max){
	int decision;
	std::cin >> decision;
	while (std::cin.fail() || decision <= 0 || decision > max){
		std::cerr << "Invalid choice: " << decision << "\nMax: " << max << '\n';
		std::cin.clear();
		std::cin.ignore(std::numeric_limits<std::streamsize> ::max());
		std::cin >> decision;
	}

	return decision;
}

void run_Story(std::map<std::string, paragraph> paragraphs, std::string current_paragraph){
	auto p = paragraphs[current_paragraph];
	p.display();
	int decision = get_valid_integer(p.options.size());
	auto next_paragraph = p.options[decision - 1].dest_label;
	run_Story(paragraphs, next_paragraph);
};

int main() {
	try {
		auto story = loadStory(writtenStory);
		run_Story(story, "00001");
	} catch (std::exception &e) {
		std::cerr << e.what() << '\n';
		return 1;
	}
	return 0;
}