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

#include "functions.h"

#define debug(x)  std::cout << x

bool
does_stash_contain(std::string stash, std::string items, game_data &data){
	bool viable = false;
	if (items.find('('))

	if (data.player.stashed_items.contains(stash)){
		for (const auto &[item_ID, quantity] : data.player.stashed_items[stash]){
			if (item_ID == items){
				viable = true;
				break;
			}else{
				viable = false;
			}
		}
	}else{
		viable = false;
	}
	return viable;
}

bool
read_nexus_conditions(std::string conditions, game_data &data, paragraph &nexus){

	std::stringstream conds(conditions);
	std::string c;
	bool viable = true;

	while (viable && std::getline(conds, c, ',')){
		int c_start = c.find_first_not_of(' ');
		int comparator = c.find(':');
		int c_end = c.find_last_not_of(' ');

		std::string var = c.substr(c_start, comparator - c_start);

		std::string check = c.substr(comparator + 1, c_end - comparator);

		if (check == "true"){
			if (data.player.decisions[var]){
				viable = true;
			}else{
				viable = false;
			}
		}else if (check == "false"){
			if (!data.player.decisions[var]){
				viable = true;
			}else{
				viable = false;
			}

		}else if (check.find_first_not_of("1234567890") != std::string::npos){
			viable = does_stash_contain(var, check, data);

		}else{
			int value = std::stoi(check);

			if (var == "visits"){
				viable = nexus.visits >= value;
				continue;
			}

			viable = data.player.attributes[var] >= value;
		}
	}
	return viable;
}

std::string
get_destination(game_data &data, paragraph &par_nexus){

    std::string destination;
    std::string line;
    std::stringstream text(par_nexus.text);
	++par_nexus.visits;

    while (getline(text, line) && !line.empty()){
		line = lowercase_string(line);
		int colon = line.find(':');
		int line_start = line.find_first_not_of(' ');
		int dest_tag_end = line.find_first_of(" =", line_start + 1);
		std::string dest_ver = line.substr(line_start, dest_tag_end);

		if (colon == std::string::npos){
			destination = dest_ver;
		}else{
			int conditions_start = line.find_first_not_of(" =", dest_tag_end);
			std::string conditions = line.substr(conditions_start);

			bool viable = read_nexus_conditions(conditions, data, par_nexus);

			if (viable){
            	destination = dest_ver;
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

int
update_UI(game_data& data, assets& assets, gui_core& gui){
	auto& s = assets.elements["story_display"];

	paragraph p = data.story[data.story_location];
	debug(p.text.data());

	s.text[s.text.size() - 1].text = p.text;
	create_option_display(gui, assets, data);

	return 0;
}

int
progress_story(game_data &data, assets& main_assets, gui_core& gui, int choice){
	paragraph p = data.story[data.story_location];

	if(choice > p.options.size()){
		debug("Clicked option does not exist");
		return 1;
	}else{
		option c = p.options[choice - 1];
		debug("Dest label: " << c.dest_label << '\n');
		std::string next_paragraph_tag = c.dest_label;

		check_for_change_maps(data, c);

		debug("Is nexus: " << data.story[next_paragraph_tag].nexus << '\n');
		if (data.story[next_paragraph_tag].nexus){
			next_paragraph_tag = get_destination(data, data.story[next_paragraph_tag]);
		}

		data.story_location = next_paragraph_tag;
		check_for_change_maps(data, data.story[data.story_location]);

		update_UI(data, main_assets, gui);
		//save_game(data);
	}
	return 0;
}