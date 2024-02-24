#include "pch.hpp"

void CM_AddModel(GfxStaticModelDrawInst* model)
{
	CClipMap::wip_geom = std::make_unique<cm_model>();

	auto xmodel = dynamic_cast<cm_model*>(CClipMap::wip_geom.get());

	xmodel->modelscale = model->placement.scale;
	xmodel->origin = model->placement.origin;
	xmodel->angles = AxisToAngles(model->placement.axis);
	xmodel->name = model->model->name;

	CClipMap::InsertGeometry(CClipMap::wip_geom);
}

int cm_model::map_export(std::ofstream& o, int index)
{

	o << "// entity " << index << '\n';
	o << "{\n";
	o << std::quoted("angles")		<< ' ' << std::quoted(std::format("{} {} {}", angles.x, angles.y, angles.z)) << '\n';
	o << std::quoted("modelscale")	<< ' ' << std::quoted(std::format("{}", modelscale)) << '\n';
	o << std::quoted("origin")		<< ' ' << std::quoted(std::format("{} {} {}", origin.x, origin.y, origin.z)) << '\n';
	o << std::quoted("model")		<< ' ' << std::quoted(name) << '\n';
	o << std::quoted("classname")	<< ' ' << std::quoted("misc_model") << '\n';
	o << "}\n";

	return ++index;

}