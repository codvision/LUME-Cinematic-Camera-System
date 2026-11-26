#include "Modules/LumeBoundsModule.h"
#include "LumeCameraMath.h"
#include "LumeCameraBoundsVolume.h"
#include "LumeCameraSubsystem.h"
#include "Engine/World.h"

void ULumeBoundsModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const FVector Original = State.CameraLocation;
	FVector Location = Original;

	// --- Volumes globais: união ---
	TArray<ALumeCameraBoundsVolume*, TInlineAllocator<8>> Volumes;
	if (bUseAllBoundsVolumes)
	{
		if (UWorld* World = GetWorld())
		{
			if (const ULumeCameraSubsystem* Subsystem = World->GetSubsystem<ULumeCameraSubsystem>())
			{
				for (ALumeCameraBoundsVolume* Volume : Subsystem->GetBoundsVolumes())
				{
					Volumes.Add(Volume);
				}
			}
		}
	}
	else
	{
		for (ALumeCameraBoundsVolume* Volume : BoundsVolumes)
		{
			Volumes.Add(Volume);
		}
	}

	bool bInsideAny = false;
	bool bHasVolume = false;
	double BestDistSq = TNumericLimits<double>::Max();
	FVector Best = Location;

	for (const ALumeCameraBoundsVolume* Volume : Volumes)
	{
		if (!IsValid(Volume) || !Volume->bEnabled)
		{
			continue;
		}
		bHasVolume = true;
		const FVector Clamped = Volume->ClampPoint(Location, EdgeMargin);
		const double DistSq = FVector::DistSquared(Clamped, Location);
		if (DistSq <= UE_KINDA_SMALL_NUMBER)
		{
			bInsideAny = true;
			break;
		}
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Best = Clamped;
		}
	}

	if (bHasVolume && !bInsideAny)
	{
		Location = Best;
	}

	// --- Restrições das zonas, ponderadas pelo peso de mistura ---
	if (bApplyZoneConstraints)
	{
		for (const FLumeZoneConstraint& Constraint : Context.ZoneConstraints)
		{
			if (Constraint.Box && Constraint.Weight > 0.f)
			{
				const FVector Clamped = LumeMath::ClampPointToBox(Constraint.Box, Location, EdgeMargin);
				Location = FMath::Lerp(Location, Clamped, static_cast<double>(Constraint.Weight));
			}
		}
	}

	State.CameraLocation = Location;

	if (bReaimAfterClamp && !Location.Equals(Original, 0.1))
	{
		FRotator LookAt = (State.LookAtPoint - Location).Rotation();
		LookAt.Roll = State.CameraRotation.Roll;
		State.CameraRotation = LookAt;
	}
}
