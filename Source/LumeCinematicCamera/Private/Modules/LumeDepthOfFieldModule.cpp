#include "Modules/LumeDepthOfFieldModule.h"
#include "LumeCameraMath.h"
#include "LumeCinematicCameraActor.h"
#include "Camera/CameraComponent.h"

void ULumeDepthOfFieldModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const FLumeFramingSettings& F = Context.Framing;
	ALumeCinematicCamera* Camera = Context.Camera;
	UCameraComponent* CameraComponent = Camera ? Camera->GetCameraComponent() : nullptr;
	if (!CameraComponent)
	{
		return;
	}

	const FVector FocusPoint = State.TargetLocation + F.PivotOffset;
	const double BaseFocus = bAutoFocusOnTarget
		? FVector::Dist(State.CameraLocation, FocusPoint)
		: static_cast<double>(ManualFocusDistance);
	const double DesiredFocus = FMath::Max(1.0, BaseFocus + static_cast<double>(F.FocusOffset));

	if (Context.bSnap || !bInitialized)
	{
		bInitialized = true;
		Focus = DesiredFocus;
		FocusRate = 0.0;
	}
	else
	{
		Focus = LumeMath::SmoothDamp(Focus, DesiredFocus, FocusRate, FocusSmoothTime, Context.DeltaTime);
	}
	State.FocusDistance = static_cast<float>(Focus);

	FPostProcessSettings& PP = CameraComponent->PostProcessSettings;
	CameraComponent->PostProcessBlendWeight = 1.f;

	PP.bOverride_DepthOfFieldFocalDistance = bEnableDepthOfField;
	PP.bOverride_DepthOfFieldFstop = bEnableDepthOfField;
	PP.bOverride_DepthOfFieldSensorWidth = bEnableDepthOfField;
	if (bEnableDepthOfField)
	{
		PP.DepthOfFieldFocalDistance = State.FocusDistance;
		PP.DepthOfFieldFstop = F.Aperture;
		PP.DepthOfFieldSensorWidth = SensorWidth;
	}

	PP.bOverride_VignetteIntensity = bApplyLensEffects;
	PP.bOverride_FilmGrainIntensity = bApplyLensEffects;
	if (bApplyLensEffects)
	{
		PP.VignetteIntensity = F.VignetteIntensity;
		PP.FilmGrainIntensity = F.FilmGrainIntensity;
	}
}
