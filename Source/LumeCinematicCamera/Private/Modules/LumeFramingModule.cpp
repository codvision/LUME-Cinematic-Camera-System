#include "Modules/LumeFramingModule.h"
#include "LumeCameraMath.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

void ULumeFramingModule::AddFramingTarget(AActor* Actor)
{
	if (IsValid(Actor))
	{
		AdditionalTargets.AddUnique(Actor);
	}
}

void ULumeFramingModule::RemoveFramingTarget(AActor* Actor)
{
	AdditionalTargets.Remove(Actor);
}

void ULumeFramingModule::ClearFramingTargets()
{
	AdditionalTargets.Reset();
}

void ULumeFramingModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const double Dt = Context.DeltaTime;
	const FLumeFramingSettings& F = Context.Framing;

	// --- Grupo: calcula deslocamento do pivô e distância necessária ---
	FVector DesiredShift = FVector::ZeroVector;
	double DesiredDistance = 0.0;

	if (bFrameAdditionalTargets && AdditionalTargets.Num() > 0)
	{
		const FVector Main = State.PivotLocation;
		const double MaxDistSq = FMath::Square(static_cast<double>(MaxTargetDistance));

		TArray<FVector, TInlineAllocator<8>> Points;
		FVector Sum = Main;
		for (const TObjectPtr<AActor>& Actor : AdditionalTargets)
		{
			if (!IsValid(Actor) || Actor == Context.Target)
			{
				continue;
			}
			const FVector P = Actor->GetActorLocation() + F.PivotOffset;
			if (MaxTargetDistance > 0.f && FVector::DistSquared(P, State.TargetLocation) > MaxDistSq)
			{
				continue;
			}
			Points.Add(P);
			Sum += P;
		}

		if (Points.Num() > 0)
		{
			const FVector Centroid = Sum / static_cast<double>(Points.Num() + 1);
			DesiredShift = (Centroid - Main) * static_cast<double>(GroupInfluence);

			const FVector Center = Main + DesiredShift;
			double Radius = FVector::Dist(Center, Main);
			for (const FVector& P : Points)
			{
				Radius = FMath::Max(Radius, FVector::Dist(Center, P));
			}
			Radius += FramingPadding;

			const double HalfFovRad = FMath::DegreesToRadians(FMath::Clamp(static_cast<double>(State.FieldOfView), 5.0, 170.0) * 0.5);
			const double Needed = Radius / FMath::Max(0.01, FMath::Tan(HalfFovRad));
			DesiredDistance = FMath::Min(Needed, static_cast<double>(F.Distance) + static_cast<double>(MaxExtraDistance));
		}
	}

	if (Context.bSnap || !bInitialized)
	{
		bInitialized = true;
		Offset = F.ScreenOffset;
		OffsetRate = FVector2D::ZeroVector;
		GroupShift = DesiredShift;
		GroupShiftRate = FVector::ZeroVector;
		GroupDistance = DesiredDistance;
		GroupDistanceRate = 0.0;
	}
	else
	{
		Offset = LumeMath::SmoothDamp(Offset, F.ScreenOffset, OffsetRate, ScreenOffsetSmoothTime, Dt);
		GroupShift = LumeMath::SmoothDamp(GroupShift, DesiredShift, GroupShiftRate, GroupSmoothTime, Dt);
		GroupDistance = LumeMath::SmoothDamp(GroupDistance, DesiredDistance, GroupDistanceRate, GroupSmoothTime, Dt);
	}

	State.ScreenOffset = Offset;
	State.PivotLocation += GroupShift;
	State.MinimumDistance = FMath::Max(State.MinimumDistance, static_cast<float>(GroupDistance));
}

void ULumeFramingModule::DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	if (!World || !bFrameAdditionalTargets)
	{
		return;
	}
	for (const TObjectPtr<AActor>& Actor : AdditionalTargets)
	{
		if (IsValid(Actor))
		{
			DrawDebugLine(World, State.PivotLocation, Actor->GetActorLocation(), FColor::Cyan, false, -1.f, 0, 1.f);
		}
	}
#endif
}
