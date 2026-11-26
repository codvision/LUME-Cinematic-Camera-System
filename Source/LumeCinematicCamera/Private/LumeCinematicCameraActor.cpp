#include "LumeCinematicCameraActor.h"
#include "LumeCinematicCameraComponent.h"

ALumeCinematicCamera::ALumeCinematicCamera()
{
	PrimaryActorTick.bCanEverTick = false;

	LumeCamera = CreateDefaultSubobject<ULumeCinematicCameraComponent>(TEXT("LumeCamera"));
	RootComponent = LumeCamera;

	// Câmeras do mundo normalmente são secundárias (ativadas por zonas ou scripts).
	// Marque "Take Over Player View" para usá-la como câmera principal.
	LumeCamera->bTakeOverPlayerView = false;
	LumeCamera->bDeactivateOtherCameras = false;
}

void ALumeCinematicCamera::ActivateCamera(float BlendTime, EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	if (LumeCamera)
	{
		LumeCamera->ActivateCamera(BlendTime, BlendFunction, BlendExponent);
	}
}
