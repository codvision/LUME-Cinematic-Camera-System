#include "Modules/LumeInterestModule.h"
#include "LumeCameraMath.h"
#include "LumeCameraSubsystem.h"
#include "LumeInterestPointComponent.h"
#include "Engine/World.h"
#include "CoreGlobals.h"
#include "DrawDebugHelpers.h"

void ULumeInterestModule::SetLookInput(FVector2D Input)
{
	LookInput = FVector2D(FMath::Clamp(Input.X, -1.0, 1.0), FMath::Clamp(Input.Y, -1.0, 1.0));
	LookInputFrame = GFrameCounter;
}

void ULumeInterestModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const double Dt = Context.DeltaTime;
	const FVector Pivot = State.PivotLocation;

	// --- Pontos de interesse ---
	FVector DesiredShift = FVector::ZeroVector;
	double DesiredExtra = 0.0;
	double DesiredMinDistance = 0.0;

	const UWorld* World = GetWorld();
	const ULumeCameraSubsystem* Subsystem = World ? World->GetSubsystem<ULumeCameraSubsystem>() : nullptr;

	if (bUseInterestPoints && Subsystem)
	{
		double TotalWeight = 0.0;
		const double HalfFovRad = FMath::DegreesToRadians(FMath::Clamp(static_cast<double>(State.FieldOfView), 5.0, 170.0) * 0.5);
		const double TanHalfFov = FMath::Max(0.01, FMath::Tan(HalfFovRad));

		for (const ULumeInterestPointComponent* Point : Subsystem->GetInterestPoints())
		{
			if (!IsValid(Point) || Point->GetOwner() == Context.Target)
			{
				continue;
			}
			const double Weight = Point->GetInfluenceAt(State.TargetLocation);
			if (Weight <= 0.0)
			{
				continue;
			}

			const FVector PointLocation = Point->GetComponentLocation();
			const FVector ToPoint = (PointLocation - Pivot) * Weight;
			DesiredShift += ToPoint.GetClampedToMaxSize(Point->MaxShift);
			TotalWeight += Weight;

			const double Falloff = Point->Influence > 0.f ? Weight / Point->Influence : 0.0;
			DesiredExtra += Point->ExtraDistance * Falloff;

			if (Point->bKeepInFrame)
			{
				const double Radius = FVector::Dist(Pivot, PointLocation) * 0.5 + Point->FramePadding;
				DesiredMinDistance = FMath::Max(DesiredMinDistance, Radius / TanHalfFov * Falloff);
			}
		}

		if (TotalWeight > 1.0)
		{
			DesiredShift /= TotalWeight;
		}
		DesiredShift = DesiredShift.GetClampedToMaxSize(MaxTotalShift);
	}

	// --- Olhar em volta (no plano da câmera) ---
	FVector2D DesiredLook = FVector2D::ZeroVector;
	const bool bInputFresh = LookInputFrame + 1 >= GFrameCounter;
	if (bAllowLookAround && bInputFresh && LookInput.Size() > LookInputDeadZone)
	{
		DesiredLook = FVector2D(LookInput.X * LookAroundDistance.X, LookInput.Y * LookAroundDistance.Y);
	}

	if (Context.bSnap || !bInitialized)
	{
		bInitialized = true;
		Shift = DesiredShift;
		ShiftRate = FVector::ZeroVector;
		Extra = DesiredExtra;
		ExtraRate = 0.0;
		MinDistance = DesiredMinDistance;
		MinDistanceRate = 0.0;
		LookOffset = FVector2D::ZeroVector;
		LookRate = FVector2D::ZeroVector;
	}
	else
	{
		Shift = LumeMath::SmoothDamp(Shift, DesiredShift, ShiftRate, InterestSmoothTime, Dt);
		Extra = LumeMath::SmoothDamp(Extra, DesiredExtra, ExtraRate, InterestSmoothTime, Dt);
		MinDistance = LumeMath::SmoothDamp(MinDistance, DesiredMinDistance, MinDistanceRate, InterestSmoothTime, Dt);

		const double LookTime = DesiredLook.IsNearlyZero() ? LookReturnSmoothTime : LookSmoothTime;
		LookOffset = LumeMath::SmoothDamp(LookOffset, DesiredLook, LookRate, LookTime, Dt);
	}

	const FQuat View = State.CameraRotation.Quaternion();
	State.PivotLocation += Shift
		+ View.GetRightVector() * LookOffset.X
		+ View.GetUpVector() * LookOffset.Y;
	State.ExtraDistance += static_cast<float>(Extra);
	State.MinimumDistance = FMath::Max(State.MinimumDistance, static_cast<float>(MinDistance));
}

void ULumeInterestModule::DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	const ULumeCameraSubsystem* Subsystem = World ? World->GetSubsystem<ULumeCameraSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}
	for (const ULumeInterestPointComponent* Point : Subsystem->GetInterestPoints())
	{
		if (!IsValid(Point))
		{
			continue;
		}
		const float Weight = Point->GetInfluenceAt(State.TargetLocation);
		const FVector Location = Point->GetComponentLocation();
		DrawDebugSphere(World, Location, 20.f, 8, Weight > 0.f ? FColor::Green : FColor(60, 90, 60), false, -1.f, 0, 1.f);
		if (Weight > 0.f)
		{
			DrawDebugLine(World, State.TargetLocation, Location, FColor::Green, false, -1.f, 0, 1.f);
		}
	}
#endif
}
