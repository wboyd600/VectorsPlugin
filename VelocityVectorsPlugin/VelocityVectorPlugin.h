#pragma once
#include <memory>
#pragma comment( lib, "pluginsdk.lib" )
#include "bakkesmod/plugin/bakkesmodplugin.h"

struct RGBA
{
	unsigned char r, g, b, a; //rgba can be a value of 0-255
};


class VelocityVectorPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
private: 
	std::shared_ptr<int> vectors_on;
	std::shared_ptr<LinearColor> vector_color;

public:
	VelocityVectorPlugin();
	~VelocityVectorPlugin();
	virtual void onLoad();
	virtual void onUnload();
	void OnFreeplayLoad(std::string eventName);
	void OnFreeplayDestroy(std::string eventName);
	void OnShowVectorsChanged(std::string oldValue, CVarWrapper cvar);
	void Render(CanvasWrapper canvas);
};
