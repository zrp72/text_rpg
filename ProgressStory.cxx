#include <iostream>
#include <vector>
#include <map>

#include <stdexcept> //may not be needed
#include <cerrno>

#include "functions.h"
#include "basic_functions.h"

#define debug(x)  std::cout << x

std::pair<bool, story_variables>
compare_variables_against_player(game_data& data, story_variables& variables){
	story_variables remaining;
	auto& items = data.items;
	auto& attributes = data.player.attributes;
	if(attributes.empty()){
		debug("No attributes within given player.\n");
	}else{
		for(auto& a : attributes){
			debug(a.second << '\n');
		}
	}
	auto& decisions = data.player.decisions;
	auto& inv = data.player.inventory;

	bool viable = true;
	for(auto& condition : variables.stats_and_items){
		if(attributes.contains(condition.first)){
			viable = attributes.at(condition.first) >= condition.second;
		}else if(items.contains(condition.first)){
			if(!inv.contains(condition.first)){
				viable = false;
			}
		}else{
			remaining.stats_and_items.push_back(condition);
		}
		if(!viable){break;}
	}if(viable){
		for(auto& decision : variables.decisions){
			if(!decisions.contains(decision)){
				viable = false;
				break;
			}
		}
	}
	return {viable, variables};
}

std::pair<char, int>
get_destination(game_data &data, nexus &nexus){

	std::vector<std::pair<char, int>> destination;
	nexus.visits += 1;
	int highest_exit_group;

	int exit_group_iteration = -1;
	for(auto& exit_group : nexus.exits){
		exit_group_iteration += 1;

		auto viable = compare_variables_against_player(data, exit_group.conditions);
		if(!viable.first){continue;}
		if(!viable.second.empty()){
			for(auto& condition : viable.second.stats_and_items){
				if(condition.first == "visits" || condition.first == "visit"){
					viable.first = nexus.visits >= condition.second;
					if(!viable.first){break;}
				}else{
					debug("Undefined remaining condition within nexus: \"" << condition.first << "\"\n");
				}
			}
		}

		if(viable.first){
			highest_exit_group = exit_group_iteration;
		}
	}
	if(highest_exit_group == -1){
		debug("No exit found for nexus.\n");
	}
	std::vector<std::string> exits;
	for(auto& exit : nexus.exits[highest_exit_group].exits){
		exits.push_back(exit);
	}
	if(exits.size() == 0){
		debug("Nexus exit group has no exits.\n");
	}
	int exit = randomize_number(exits.size() - 1);
	return data.story.tag_map[exits[exit]];
}

//TODO: Move this function into class item_container
void
alter_inventory(character& player, auto& change){
	auto& inventory = player.inventory;
	int open_inventory_space = player.inventory.max_size - inventory.used_space;

	if ((open_inventory_space - change.second) < 0){
		debug("Overburdened" << '\n');
	}

	if(inventory.contains(change.first)){
		inventory.at(change.first) += change.second;
		debug(change.first << " + " << change.second << '\n');
	}else{
		inventory.items.emplace(change.first, change.second);
		debug("New item: " << change.first << '\n');
	}
}

//handles option and paragraph change maps, so auto is NEEDED
void
check_for_change_maps(game_data &data, auto &search_target){
	auto &st = search_target;
	if (!st.changes.empty()){
		for (const auto& change : st.changes.stats_and_items) {
			if(data.items.contains(change.first)){
				alter_inventory(data.player, change);
			}else if(data.player.attributes.contains(change.first)){
				debug(change.first << ": " << data.player.attributes[change.first] << '\n');
				data.player.attributes[change.first] += change.second;
				debug(change.first << ": " << data.player.attributes[change.first] << '\n');
			}else{
				debug("Change tag has no matching stat or item: \"" << change.first << "\"\n");
			}
		}
		for (const auto& change : st.changes.decisions) {
			data.player.decisions.emplace(change, true);
			debug("Added decision: " << change << '\n');
		}
	}
}

int
update_UI(game_data& data, assets& assets, gui_core& gui){

	create_story_display(gui, assets, data);
	resize_element(assets.elements.at("story_display"), gui, assets);

	create_option_display(gui, assets, data);
	resize_element(assets.elements.at("option_display"), gui, assets);

	return 0;
}

int
progress_story(game_data &data, assets& main_assets, gui_core& gui, int choice){
	auto location = data.story.tag_map.at(data.story.story_location);
	paragraph& p = data.story.paragraphs.at(location.second);

	if(choice > p.options.size()){
		debug("Clicked option does not exist.\n");
		return 1;
	}else{
		option& c = p.options[choice - 1];
		debug("Dest label: " << c.dest_label << '\n');
		auto& dest = data.story.tag_map.at(c.dest_label);

		check_for_change_maps(data, c);

		if(dest.first == 'n'){
			dest = get_destination(data, data.story.nexuses[dest.second]);
		}

		data.story.story_location = c.dest_label;
		paragraph& paragraph_location = data.story.paragraphs[dest.second];
		check_for_change_maps(data, paragraph_location);

		update_UI(data, main_assets, gui);
		//save_game(data);
	}
	return 0;
}