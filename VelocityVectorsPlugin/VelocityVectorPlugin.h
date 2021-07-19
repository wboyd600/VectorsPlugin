#pragma once
#include <memory>
#pragma comment( lib, "pluginsdk.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "RenderingTools/Objects/Cone.h"


class VelocityVectorPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private: 
	std::shared_ptr<int> vectors_on;
	std::shared_ptr<LinearColor> vector_color;
	LinearColor colors[2] = { {250, 250, 0, 255}, {75, 0, 130, 240} };
	std::shared_ptr<float> vector_scale;
	std::shared_ptr<float> cone_height;
	std::shared_ptr<int> cone_segments;
	std::shared_ptr<int> cone_radius;
	std::shared_ptr<int> cone_thickness;
public:
	VelocityVectorPlugin();
	~VelocityVectorPlugin();
	virtual void onLoad();
	virtual void onUnload();
	void OnFreeplayLoad(std::string eventName);
	void OnFreeplayDestroy(std::string eventName);
	void OnShowVectorsChanged(std::string oldValue, CVarWrapper cvar);
	void Render(CanvasWrapper canvas);
	RT::Cone GetCone(Vector loc, Vector v, float difference);
	void ResetDefault();
};
