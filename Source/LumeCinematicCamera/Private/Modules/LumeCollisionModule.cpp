#include "Modules/LumeCollisionModule.h"
#include "LumeCameraMath.h"
#include "LumeCinematicCameraComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

void ULumeCollisionModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Start = State.LookAtPoint;
	const FVector Desired = State.CameraLocation;
	const FVector ToCamera = Desired - Start;
	const double DesiredDistance = ToCamera.Size();
	if (DesiredDistance < 1.0)
	{
		return;
	}
	const FVector Direction = ToCamera / DesiredDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(LumeCameraCollision), false);
	if (Context.Camera)
	{
		Params.AddIgnoredActor(Context.Camera->GetOwner());
	}
	if (Context.Target)
	{
		Params.AddIgnoredActor(Context.Target.Get());
	}

	double Allowed = DesiredDistance;
	FHitResult Hit;
	if (World->SweepSingleByChannel(Hit, Start, Desired, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeRadius), Params))
	{
		Allowed = FMath::Clamp(static_cast<double>(Hit.Distance), FMath::Min(static_cast<double>(MinDistance), DesiredDistance), DesiredDistance);
	}

	if (Context.bSnap || CurrentDistance < 0.0 || Allowed <= CurrentDistance)
	{
		// Aproxima imediatamente: nunca atravessa a parede.
		CurrentDistance = Allowed;
		DistanceRate = 0.0;
	}
	else
	{
		CurrentDistance = LumeMath::SmoothDamp(CurrentDistance, Allowed, DistanceRate, ReturnSmoothTime, Context.DeltaTime);
	}

	// Pontos de vista fixos (zonas) não sofrem colisão.
	const double FreeAlpha = 1.0 - FMath::Clamp(static_cast<double>(Context.FixedViewpointAlpha), 0.0, 1.0);
	const double FinalDistance = FMath::Lerp(DesiredDistance, CurrentDistance, FreeAlpha);

	State.CameraLocation = Start + Direction * FinalDistance;
}
