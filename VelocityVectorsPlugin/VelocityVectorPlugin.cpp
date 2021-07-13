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
#include "RenderingTools/Objects/Cone.h"
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
	vector_scale = 3.f;

	vector_color = std::make_shared<LinearColor>(LinearColor{ 0.f,0.f,0.f,0.f });
	cvarManager->registerCvar("cl_vector_color", "#FFFF00", "Color of the velocity vector visualization.", true).bindTo(vector_color);

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
	if (*vectors_on && ingame) {

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

		Rotator rot_car = car.GetRotation();
		Rotator rot_ball = game.GetBall().GetRotation();

		float diff = (camera.GetLocation() - loc_car).magnitude();
		float ball_diff = (camera.GetLocation() - loc_ball).magnitude();
		Quat car_rot = RotatorToQuat(rot_car);
		Quat ball_rot = RotatorToQuat(rot_ball);
		auto currentTime = game.GetSecondsElapsed();
		auto difference = currentTime - last_time;
		last_time = currentTime;
		canvas.SetColor(*vector_color);
		
		if (diff < 1000.f) {
			auto car_v = car.GetVelocity();
			if (abs(car_v.Z) < 20.f) {
				car_v.Z = 0.f;
			}
			auto add = car.GetVelocity().getNormalized() * 15.f;
			RT::Line(loc_car, loc_car + car_v * (difference) * vector_scale + add, 7.f).DrawWithinFrustum(canvas, frust);
			auto car_cone = RT::Cone(loc_car + car_v * (difference) * vector_scale, car_v);
			car_cone.height = 15;
			car_cone.segments = 20;
			car_cone.radius = 20;
			car_cone.thickness = 3;
			car_cone.Draw(canvas);

		}
		if (ball_diff < 1000.f) {
			auto add2 = game.GetBall().GetVelocity().getNormalized() * 15.f;
			auto ball_v = game.GetBall().GetVelocity();
			if (abs(ball_v.Z) < 20.f) {
				ball_v.Z = 0.f;
			}
			RT::Line(loc_ball, loc_ball + ball_v * (difference) *vector_scale + add2, 7.f).DrawWithinFrustum(canvas, frust);
			auto ball_cone = RT::Cone(loc_ball + ball_v * (difference) *vector_scale, ball_v );
			ball_cone.height = 15;
			ball_cone.segments = 20;
			ball_cone.radius = 20;
			ball_cone.thickness = 3;
			ball_cone.Draw(canvas);
		}
	}

}


