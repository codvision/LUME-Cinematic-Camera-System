#include "Modules/LumeDynamicZoomModule.h"
#include "LumeCameraMath.h"

void ULumeDynamicZoomModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const double Dt = Context.DeltaTime;
	const FLumeFramingSettings& F = Context.Framing;

	FVector Planar = State.TargetVelocity;
	Planar.Z = 0.0;
	const double Speed = Planar.Size();

	// --- Velocidade ---
	const double TargetSpeedAlpha = bSpeedZoom
		? FMath::Clamp(Speed / FMath::Max(1.0, static_cast<double>(SpeedForMaxZoom)), 0.0, 1.0)
		: 0.0;

	// --- Parado ---
	IdleTime = Speed < 10.0 ? IdleTime + Dt : 0.0;
	const double TargetIdleAlpha = (bIdlePushIn && IdleTime > IdleDelay) ? 1.0 : 0.0;

	if (Context.bSnap || !bInitialized)
	{
		SpeedAlpha = TargetSpeedAlpha;
		SpeedAlphaRate = 0.0;
		IdleAlpha = TargetIdleAlpha;
		IdleAlphaRate = 0.0;
	}
	else
	{
		SpeedAlpha = LumeMath::SmoothDamp(SpeedAlpha, TargetSpeedAlpha, SpeedAlphaRate, SpeedResponseTime, Dt);
		// Aproxima devagar; volta rápido quando o jogador se move.
		const double IdleSmooth = TargetIdleAlpha > IdleAlpha ? IdlePushInTime : F.ZoomSmoothTime * 0.5;
		IdleAlpha = LumeMath::SmoothDamp(IdleAlpha, TargetIdleAlpha, IdleAlphaRate, IdleSmooth, Dt);
	}

	double DesiredDistance = F.Distance + ExtraDistanceAtMaxSpeed * SpeedAlpha + static_cast<double>(State.ExtraDistance);
	DesiredDistance *= FMath::Lerp(1.0, static_cast<double>(IdleDistanceMultiplier), IdleAlpha);
	DesiredDistance = FMath::Max(DesiredDistance, static_cast<double>(State.MinimumDistance));
	DesiredDistance = FMath::Clamp(DesiredDistance, static_cast<double>(MinDistance), FMath::Max(static_cast<double>(MinDistance), static_cast<double>(MaxDistance)));

	const double DesiredFov = FMath::Clamp(F.FieldOfView + ExtraFOVAtMaxSpeed * SpeedAlpha, 5.0, 170.0);

	if (Context.bSnap || !bInitialized)
	{
		bInitialized = true;
		Distance = DesiredDistance;
		DistanceRate = 0.0;
		Fov = DesiredFov;
		FovRate = 0.0;
	}
	else
	{
		Distance = LumeMath::SmoothDamp(Distance, DesiredDistance, DistanceRate, F.ZoomSmoothTime, Dt);
		Fov = LumeMath::SmoothDamp(Fov, DesiredFov, FovRate, F.ZoomSmoothTime, Dt);
	}

	State.Distance = static_cast<float>(Distance);
	State.FieldOfView = static_cast<float>(Fov);
}
