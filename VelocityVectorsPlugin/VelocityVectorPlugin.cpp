#define LINMATH_H //Conflicts with linmath.h if we done declare this here
#include "VelocityVectorPlugin.h"
#include "bakkesmod\wrappers\includes.h"
#include "bakkesmod\wrappers\GameEvent\TutorialWrapper.h"
#include "bakkesmod\wrappers\GameObject\BallWrapper.h"
#include "bakkesmod\wrappers\GameObject\CarWrapper.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/GameObject/BallWrapper.h"
#include "bakkesmod/wrappers/GameObject/CarWrapper.h"
#include "bakkesmod\wrappers\GameEvent\TutorialWrapper.h"
#include "bakkesmod/wrappers/arraywrapper.h"
#include "RenderingTools/Objects/Circle.h"
#include "RenderingTools/Objects/Frustum.h"
#include "RenderingTools/Objects/Line.h"
#include "RenderingTools/Extra/WrapperStructsExtensions.h"
#include "RenderingTools/Extra/RenderingMath.h"
BAKKESMOD_PLUGIN(VelocityVectorPlugin, "Velocity Vector plugin", "0.1", PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)

VelocityVectorPlugin::VelocityVectorPlugin() {

}

VelocityVectorPlugin::~VelocityVectorPlugin() {

}
void VelocityVectorPlugin::onLoad()
{
	vectors_on= std::make_shared<int>(0);
	cvarManager->registerCvar("cl_show_vectors", "0", "Enable or Disable the Velocity Vector Plugin", true, true, 0, true, 1, true).bindTo(vectors_on);
	cvarManager->getCvar("cl_show_vectors").addOnValueChanged(std::bind(&VelocityVectorPlugin::OnShowVectorsChanged, this, std::placeholders::_1, std::placeholders::_2));
	// Set and bind vector scale
	vector_scale = std::make_shared<float>(1.f);
	cvarManager->registerCvar("cl_vector_scale", "3.f", "Scale of velocity vector projection", true, true, 1.f, true, 5.f, true).bindTo(vector_scale);
	// Set and bind vector color
	vector_color = std::make_shared<LinearColor>(LinearColor{ 0.f,0.f,0.f,0.f });
	cvarManager->registerCvar("cl_vector_color", "#FFFF00", "Color of the velocity vector visualization.", true).bindTo(vector_color);
	// Set and bind cone height
	cone_height = std::make_shared<float>(15.f);
	cvarManager->registerCvar("cl_cone_height", "15.f", "Height of cone", true, true, 1.f, true, 20.f, true).bindTo(cone_height);
	// Set and bind cone segments
	cone_segments = std::make_shared<int>(20);
	cvarManager->registerCvar("cl_cone_segments", "20", "Height of cone", true, true, 6, true, 30, true).bindTo(cone_segments);
	// Set and bind cone radius
	cone_radius = std::make_shared<int>(20);
	cvarManager->registerCvar("cl_cone_radius", "20", "Radius of cone", true, true, 5, true, 30, true).bindTo(cone_radius);
	// Set and bind cone thickness
	cone_thickness = std::make_shared<int>(3);
	cvarManager->registerCvar("cl_cone_thickness", "3", "Thickness of cone", true, true, 1, true, 10, true).bindTo(cone_thickness);
	//Notifier calls ResetDefaults function
	cvarManager->registerNotifier("notifier_reset_defaults", [this](std::vector<std::string>params) {ResetDefault(); }, "Reset Defaults", PERMISSION_ALL);
	
	gameWrapper->HookEvent("Function TAGame.Mutator_Freeplay_TA.Init", bind(&VelocityVectorPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.Destroyed", bind(&VelocityVectorPlugin::OnFreeplayDestroy, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.StartPlayTest", bind(&VelocityVectorPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameEvent_TrainingEditor_TA.Destroyed", bind(&VelocityVectorPlugin::OnFreeplayDestroy, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.GameInfo_Replay_TA.InitGame", bind(&VelocityVectorPlugin::OnFreeplayLoad, this, std::placeholders::_1));
	gameWrapper->HookEvent("Function TAGame.Replay_TA.EventPostTimeSkip", bind(&VelocityVectorPlugin::OnFreeplayLoad, this, std::placeholders::_1));
}

void VelocityVectorPlugin::onUnload()
{
}

void VelocityVectorPlugin::OnFreeplayLoad(std::string eventName)
{
	cvarManager->log(std::string("OnFreeplayLoad") + eventName);
	if (*vectors_on) 
		gameWrapper->RegisterDrawable(std::bind(&VelocityVectorPlugin::Render, this, std::placeholders::_1));
}

void VelocityVectorPlugin::OnFreeplayDestroy(std::string eventName)
{
	gameWrapper->UnregisterDrawables();
}

void VelocityVectorPlugin::OnShowVectorsChanged(std::string oldValue, CVarWrapper cvar)
{
	int ingame = (gameWrapper->IsInReplay()) ? 2 : ((gameWrapper->IsInOnlineGame()) ? 0 : ((gameWrapper->IsInGame()) ? 1 : 0));
	if (cvar.getIntValue() && ingame) {
		OnFreeplayLoad("Load");
	}
	else {
		OnFreeplayDestroy("Destroy");
	}
}

float last_time = 0.f;
void VelocityVectorPlugin::Render(CanvasWrapper canvas)
{
	int ingame = (gameWrapper->IsInGame()) ? 1 : (gameWrapper->IsInReplay()) ? 2 : 0;
	if (*vectors_on && ingame && !gameWrapper->IsPaused()) {

		if (gameWrapper->IsInOnlineGame() && ingame != 2) return;
		ServerWrapper game = (ingame == 1) ? gameWrapper->GetGameEventAsServer() : gameWrapper->GetGameEventAsReplay();
		if (game.IsNull())
			return;
		ArrayWrapper<CarWrapper> cars = game.GetCars();
		auto camera = gameWrapper->GetCamera();
		if (camera.IsNull()) return;
		RT::Frustum frust{ canvas, camera };
		if (cars.IsNull())
			return;
		auto car = cars.Get(0);

		Vector loc_car = car.GetLocation();
		Vector loc_ball = game.GetBall().GetLocation();

		float diff = (camera.GetLocation() - loc_car).magnitude();
		float ball_diff = (camera.GetLocation() - loc_ball).magnitude();

		auto currentTime = game.GetSecondsElapsed();
		auto difference = currentTime - last_time;
		last_time = currentTime;
		canvas.SetColor(*vector_color);
		if (diff < 1000.f) {
			auto add = car.GetVelocity().getNormalized() * *cone_height;
			auto car_v = car.GetVelocity();
			if (abs(car_v.Z) < 20.f) {
				car_v.Z = 0.f;
			}
			RT::Line(loc_car, loc_car + car_v * (difference) * *vector_scale + add, 7.f).DrawWithinFrustum(canvas, frust);
			auto car_cone = GetCone(loc_car, car_v, difference);
			car_cone.Draw(canvas);

		}
		if (ball_diff < 1000.f) {
			auto add_ball = game.GetBall().GetVelocity().getNormalized() * *cone_height;
			auto ball_v = game.GetBall().GetVelocity();
			if (abs(ball_v.Z) < 20.f) {
				ball_v.Z = 0.f;
			}
			RT::Line(loc_ball, loc_ball + ball_v * (difference) * *vector_scale + add_ball, 7.f).DrawWithinFrustum(canvas, frust);
			auto ball_cone = GetCone(loc_ball,ball_v,difference);
			ball_cone.Draw(canvas);

		}
	}

}

RT::Cone VelocityVectorPlugin::GetCone(Vector loc, Vector v, float difference)
{
	auto cone_ret = RT::Cone(loc + v * (difference)* *vector_scale, v);
	cone_ret.height = *cone_height;
	cone_ret.segments = *cone_segments;
	cone_ret.radius = *cone_radius;
	cone_ret.thickness = *cone_thickness;
	return cone_ret;
}


void VelocityVectorPlugin::ResetDefault() {
	cvarManager->getCvar("cl_vector_scale").setValue(1.f);
	cvarManager->getCvar("cl_vector_color").setValue(colors[0]);
	cvarManager->getCvar("cl_cone_height").setValue(15.f);
	cvarManager->getCvar("cl_cone_segments").setValue(20);
	cvarManager->getCvar("cl_cone_radius").setValue(20);
	cvarManager->getCvar("cl_cone_thickness").setValue(3);
	*vector_scale = 1.f;
	*vector_color = colors[0];
	*cone_height = 15.f;
	*cone_segments = 20;
	*cone_radius = 20;
	*cone_thickness = 3;
}