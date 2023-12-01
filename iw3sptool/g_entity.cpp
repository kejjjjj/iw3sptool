#include "pch.hpp"

void Cmd_ShowEntities_f()
{


	decltype(auto) ents = gameEntities::getInstance();

	if (cmd_args->argc[cmd_args->nesting] > 2) {
		Com_Printf(CON_CHANNEL_CONSOLEONLY, "usage: cm_showEntities <classname>\n");
		return;
	}

	if (cmd_args->argc[cmd_args->nesting] == 1) {

		if (ents.empty()) {
			return Com_Printf("there are no entities to be cleared.. did you intend to use cm_showEntities <classname>?\n");
		}

		Com_Printf("clearing %i entities from the render queue\n", ents.size());
		ents.clear();
		return;
	}

	auto filter = *(cmd_args->argv[cmd_args->nesting] + 1);
	ents.clear();

	G_DiscoverGentities(level, filter);
}


void G_DiscoverGentities(level_locals_t* l, const char* classname)
{
	rb_requesting_to_stop_rendering = true;
	decltype(auto) ents = gameEntities::getInstance();

	std::string classname_s;
	for (int i = 0; i < l->num_entities; i++) {


		classname_s = Scr_GetString(l->gentities[i].classname);

		if (classname_s.contains(classname) == false && strcmp(classname, "all"))
			continue;

		ents.push_back(&l->gentities[i]);

			

	}

	Com_Printf("adding %i entities to the render queue\n", ents.size());


	std::cout << "a total of " << ents.size() << " entities\n";

	rb_requesting_to_stop_rendering = false;

}
