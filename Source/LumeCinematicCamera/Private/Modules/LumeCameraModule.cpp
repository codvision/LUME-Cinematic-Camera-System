#include "Modules/LumeCameraModule.h"
#include "LumeCinematicCameraActor.h"

ALumeCinematicCamera* ULumeCameraModule::GetOwningCamera() const
{
	return Cast<ALumeCinematicCamera>(GetOuter());
}

UWorld* ULumeCameraModule::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		return nullptr;
	}
	if (const UObject* Outer = GetOuter())
	{
		return Outer->GetWorld();
	}
	return nullptr;
}

void ULumeBlueprintCameraModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	// Sem implementação em Blueprint, o evento retornaria um State vazio: não chame.
	if (GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(ULumeBlueprintCameraModule, ReceiveUpdateModule)))
	{
		State = ReceiveUpdateModule(Context, State);
	}
}
