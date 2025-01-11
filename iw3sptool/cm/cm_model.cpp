#include "cm_model.hpp"
#include "cm_brush.hpp"
#include "cm_typedefs.hpp"

#include "com/com_vector.hpp"

#include <fstream>
#include <iomanip>

void CM_AddModel(GfxStaticModelDrawInst* model)
{
	cm_model xmodel;

	xmodel.modelscale = model->placement.scale;
	xmodel.origin = model->placement.origin;
	xmodel.angles = AxisToAngles(model->placement.axis);
	xmodel.name = model->model->name;

	CClipMap::Insert(std::make_unique<cm_model>(xmodel));
}

int cm_model::map_export(std::ofstream& o, int index) const
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