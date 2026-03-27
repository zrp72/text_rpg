#include <charconv>
#include <string>
#include <cctype>

#include "functions.h"
#include "basic_functions.h"

#define debug(x)  std::cout << x

//Creates a substring from the given position to any of the given ends.
//End will not be included in returned string.
//
//iterator will be moved to the character directly after the given end.
std::string
create_substring(std::string& source, int& c, std::string ends, bool lower){
	std::string string;

	size_t start = source.find_first_not_of(" \n\t\v\f\r", c);
	if(start == std::string::npos){
		c = source.length();
		return string;
	}

	size_t end = source.find_first_of(ends, c);
	if(end == std::string::npos){
		string = source.substr(start);
		c = source.length();

	}else{

		size_t trimmed_end = source.find_last_not_of(" \n\t\v\f\r", end - 1);
		string = source.substr(start, trimmed_end - start + 1);
		c = end + 1;

	}

	if(lower){lowercase_string(string);}

	debug("String cut at \"" << source[c - 1] << "\": " << string << '\n');

	return string;
}

//To be called at newline to check if there are any characters between
//the current c pos and the next newline.
//
//This functions does not move c.
bool
check_double_newline(std::string& story, int c){
	while(c < story.length()){
		if(story[c] == '\n'){
			return true;
		}
		if(std::isspace(story[c])){
			c += 1;
			continue;
		}else{
			return false;
		}
	}
	return false;
}

//All end characters should also be included in delimiters.
//
//Created strings will be lowercased.
//Numbers must be connected to be parsed correctly, meaning 0 whitespace.
story_variables
parse_story_variables(std::string& story, int& c, std::string delimiters, std::string end){

	std::vector<std::string> conditions;
	story_variables variables;

	bool _break = false;
	while(c < story.length()){
		conditions.push_back(create_substring(story, c, delimiters, true));
		for(char e : end){
			if(story[c - 1] == e){
				_break = true;
				break;
			}
		}
		if(_break){break;}
	}

	for(auto& s : conditions){

		if(s == "default"){
			return story_variables();

		}else if(s.find_first_of("-+1234567890") == std::string::npos){

			variables.decisions.push_back(s);

		}else{

			int n_begin = s.find_first_of("-+1234567890");
			int n_end = s.find_first_not_of("-+1234567890", n_begin);

			std::string numbers = s.substr(n_begin, n_end - n_begin);

			int amount;
			std::from_chars(numbers.data(), numbers.data() + numbers.length(), amount);

			//if/when revisited, be wary that n_end is moved by create_substring
			std::string amount_of = create_substring(s, n_end, "", true);
			if(amount_of.find_first_of("-+1234567890") != std::string::npos){
				debug("Some numbers may have been cut off within variable condition: " << s << '\n');
			}
			variables.stats_and_items.push_back({amount_of, amount});
		}
	}
	return variables;
}

void
styling::add_marker(std::string marker, std::string meaning){
	std::list<std::string>& bm = this->bold_markers;
	std::list<std::string>& im = this->italic_markers;

	if(meaning == "bold"){

		if(std::find(bm.begin(), bm.end(), marker) == bm.end()){
			if(std::find(im.begin(), im.end(), marker) != im.end()){
				debug("Style marker " << marker << "defined for both bold and italic.\n");
			}

			bm.push_back(marker);
			debug(__LINE__ << '\n');
			return;
		}else{
			debug("Redefinition of bold marker: " << marker << '\n');
			return;
		}

	}else if(meaning == "italic"){

		if(std::find(im.begin(), im.end(), marker) == im.end()){
			if(std::find(bm.begin(), bm.end(), marker) != bm.end()){
				debug("Style marker " << marker << "defined for both bold and italic.\n");
			}

			im.push_back(marker);
			return;
		}else{
			debug("Redefinition of italic marker " << marker << '\n');
			return;
		}
	}

	debug("Improper definition: " << marker << ':' << meaning << '\n');
	return;
}

void
styling::remove_marker(std::string marker){
	this->italic_markers.remove(marker);
	this->bold_markers.remove(marker);
}

void
game_data::parse_story_file(files& files){

	auto& used_glyphs = this->story.used_glyphs;

	styling style;
	texture_info option_textures;
	texture_info paragraph_textures;
	glyph_styling glyph_style;
	std::string& story = files.story;
	story_variables option_conditions;

	int c = 0;
	bool undefine = false;
	while(c < story.length()){
		debug(__LINE__ << ": " << c << '\n');
		switch(story[c]){
		case ' ': {
			c += 1;
			continue;
		}
		//Just visually pleasing to be able to use underscores for seperation.
		case '_': {
			int non_underscore = story.find_first_not_of(" _", c);
			int newline = story.find('\n', c);
			if(newline == non_underscore){
				c = newline;
			}else{
				c += 1;
			}
			continue;
		}
		//For inline commenting
		case '/': {
			c = story.find('\n', c) + 1;
			continue;
		}
		case '\n': {
			option_conditions.clear();
			undefine = false;
			c += 1;
			continue;
		}
		case '\\': {
			undefine = true;
			c += 1;
			continue;
		}
		case '#': {
			c += 1;

			debug(undefine << '\n');
			if(undefine){

				style.remove_marker(create_substring(story, c, "\n", true));

			}else{

				//revisit and figure out why create_substring cant be directly passed twice
				std::string marker = create_substring(story, c, ":", true);
				std::string meaning = create_substring(story, c, "\n", true);
				style.add_marker(marker, meaning);

			}

			continue;

		}
		case '|': {
			c += 1;
			std::string visual = create_substring(story, c, ":", true);
			std::string filename = create_substring(story, c, "\n", true);
			//TODO: Surely there is a better way to write this
			if(visual == "font"){
				glyph_style.font = filename;
			}else if(visual == "option_border"){
				option_textures.border = filename;
			}else if(visual == "option_entry_anim"){
				option_textures.entry_anim = filename;
			}else if(visual == "option_exit_anim"){
				option_textures.exit_anim = filename;
			}else if(visual == "paragraph_border"){
				paragraph_textures.border = filename;
			}else if(visual == "paragraph_entry_anim"){
				paragraph_textures.entry_anim = filename;
			}else if(visual == "paragraph_exit_anim"){
				paragraph_textures.exit_anim = filename;
			}else{
				debug("Undefined visual setting: " << visual << ': ' << filename << '\n');
			}
			continue;
		}
		case '[': {
			c += 1;
			std::string tag = create_substring(story, c, ":", true);
			if(this->story.tag_map.contains(tag)){
				debug("Nexus already exists:" << tag << '\n');
			}else{
				int vector_pos = this->story.nexuses.size();
				this->story.tag_map.emplace(tag, std::pair('n', vector_pos));

				nexus nexus;

				while(c < story.length()){
					nexus::exit_group exit_group;

					exit_group.conditions = parse_story_variables(story, c, ",:]", ":]");

					if(story[c - 1] == ']'){
						debug("Nexus closed with empty exit group.\n");
						break;
					}else if(move_iterator_past_whitespace(story, c); story.at(c) == ']'){
						debug("Nexus closed with empty exit group.\n");
						c += 1;
						break;
					}

					while(c < story.length()){
						exit_group.exits.push_back(create_substring(story, c, "\n]", true));

						if(story[c - 1] == '\n' && check_double_newline(story, c)){
							break;
						}else if(story[c - 1] == ']'){
							break;
						}else{
							move_iterator_past_whitespace(story, c);
							if(story.at(c) == ']'){
								c += 1;
								break;
							}
						}
					}

					nexus.exits.push_back(exit_group);

					if(story.at(c - 1) == ']'){break;}
				}

				this->story.nexuses.push_back(nexus);
			}

			continue;
		}
		case '(': {
			c += 1;
			option_conditions = parse_story_variables(story, c, ",)", ")");
			continue;
		}
		case '{': {
			c += 1;
			std::string tag = create_substring(story, c, ":", true);
			debug("Option tag: " << tag << '\n');
			if(!this->story.tag_map.contains(tag)){
				debug("Attempted to attach option to undefined paragraph: " << tag << '\n');
			}else if(this->story.tag_map[tag].first == 'n'){
				debug("Attempted to attach option to nexus: " << tag << '\n');

			}else{

				int pos = this->story.tag_map.at(tag).second;

				std::string dstlabel = create_substring(story, c, "}", true);
				option option(dstlabel, option_textures);

				if(!option_conditions.empty()){
					option.reveal_conditions = option_conditions;
				}

				move_iterator_past_whitespace(story, c);\
				if(story[c] == '('){
					c += 1;
					option.changes = parse_story_variables(story, c, ",)", ")");
					move_iterator_past_whitespace(story, c);
				}

				while(c < story.length()){
					if(story[c] == '\n'){
						break;
					}else if(story[c] == ' ' && option.prompt.back().unicode == ' '){
						c += 1;
					}else{
						glyph glyph(story[c], glyph_style);

						if(std::find(used_glyphs.begin(), used_glyphs.end(), glyph) == used_glyphs.end()){
							used_glyphs.push_back(glyph);
						}

						option.prompt.push_back(glyph);
						c += 1;
					}
				}
				while(option.prompt.back().unicode == ' '){
					option.prompt.pop_back();
				}

				this->story.paragraphs.at(pos).options.push_back(option);
			}
			continue;
		}
		default: {
			std::string tag = std::string(create_substring(story, c, ":", true));

			if(this->story.tag_map.contains(tag)){
				if(this->story.tag_map.at(tag).first == 'n'){
					debug("Paragraph tag already exists as a nexus: " << tag << '\n');
				}else{
					debug("Paragraph tag " << tag << " already exists.\n");
				}

			}else{

				int vector_pos = this->story.paragraphs.size();
				this->story.tag_map.emplace(tag, std::pair('p', vector_pos));

				paragraph paragraph(paragraph_textures);

				move_iterator_past_whitespace(story, c);
				if(story[c] == '('){
					c += 1;
					auto variables = parse_story_variables(story, c, ",)", ")");
					paragraph.changes = variables;
					move_iterator_past_whitespace(story, c);
				}

				while(c < story.length()){
					//TODO: implement style marker reading
					if(story[c] == '\n'){
						c += 1;
						if(check_double_newline(story, c)){
							while(paragraph.text[paragraph.text.size() - 1].unicode == ' '){
								paragraph.text.pop_back();
							}
							break;
						}else{
							if(paragraph.text.back().unicode != ' '){
								paragraph.text.push_back(glyph(' ', glyph_style));
							}
							move_iterator_past_whitespace(story, c);
						}
					}else if(story[c] == ' ' && paragraph.text.back().unicode == ' '){
						c += 1;
					}else{
						glyph glyph(story[c], glyph_style);

						if(std::find(used_glyphs.begin(), used_glyphs.end(), glyph) == used_glyphs.end()){
							used_glyphs.push_back(glyph);
						}

						paragraph.text.push_back(glyph);
						c += 1;
					}
				}
				while(paragraph.text.back().unicode == ' '){
					paragraph.text.pop_back();
				}

				this->story.paragraphs.push_back(paragraph);
			}
			continue;
		}
		}
	}
}
