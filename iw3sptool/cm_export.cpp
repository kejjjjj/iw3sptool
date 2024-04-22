#include "pch.hpp"
static void CM_WriteHeader(std::ofstream& f)
{
	f << "iwmap 4\n";
	f << "\"000_Global\" flags  active\n";
	f << "\"The Map\" flags\n";
	f << "// entity 0\n{\n";
	f << "\"contrastGain\" " "\"0.125\"" << '\n';
	f << "\"diffusefraction\" " "\"0.5\"" << '\n';
	f << "\"_color\" " "\"0.2 0.27 0.3 1\"" << '\n';
	f << "\"sunlight\" " "\"1\"" << '\n';
	f << "\"sundirection\" " "\"-30 -95 0\"" << '\n';
	f << "\"sundiffusecolor\" " "\"0.2 0.27 0.3 1\"" << '\n';
	f << "\"suncolor\" " "\"0.2 0.27 0.3 1\"" << '\n';
	f << "\"ambient\" " "\".1\"" << '\n';
	f << "\"bouncefraction\" " "\".7\"" << '\n';
	f << "\"classname\" \"worldspawn\"\n";
}
static void CM_WriteAllBrushes(std::ofstream& o)
{
	if (CClipMap::size() == 0) {
		return FatalError("CClipMap::size() == 0");
	}

	int brushIndex = 0;

	auto& geo = CClipMap::get();
	bool entity_start = false;

	for (auto& geom : geo) {

		

		if (geom->type() == cm_geomtype::model && !entity_start) {
			entity_start = true;
			brushIndex = 1;
			//end brushes
			o << "}\n";
		}

		brushIndex = geom->map_export(o, brushIndex);
	}

	if (!entity_start) {
		o << "}\n";
	}

}

static void CM_WriteInOrder(std::ofstream& o)
{
	CM_WriteHeader(o);
	CM_WriteAllBrushes(o);



}

void CM_MapExport()
{
	CM_LoadMap();

	std::string path = fs::exe_path() + "\\map_source\\kej";

	std::string mapname = Dvar_FindMalleableVar("mapname")->current.string;

	std::string full_path = path + "\\" + mapname + ".map";

	if (!fs::directory_exists(path)) {
		if (!fs::create_directory(path)) {
			return FatalError(std::format("couldn't create the path \"{}\"\reason: {}", path, fs::get_last_error()));
		}
	}

	fs::create_file(full_path);

	std::ofstream o(full_path, static_cast<int>(fs::fileopen::FILE_OUT));

	if (!o.is_open()) {
		FatalError(std::format("an error occurred while trying to open \"{}\"!\nreason: {}", full_path, fs::get_last_error()));
		return;
	}

	CM_WriteInOrder(o);


	o.close();

}