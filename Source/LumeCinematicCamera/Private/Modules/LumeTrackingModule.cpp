#include "Modules/LumeTrackingModule.h"
#include "LumeCameraMath.h"
#include "DrawDebugHelpers.h"

void ULumeTrackingModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const double Dt = Context.DeltaTime;
	const FLumeFramingSettings& F = Context.Framing;
	const FVector Raw = State.TargetLocation + F.PivotOffset;

	const bool bTeleported = bInitialized && TeleportDistance > 0.f
		&& FVector::DistSquared(Raw, PreviousRaw) > FMath::Square(static_cast<double>(TeleportDistance));

	if (Context.bSnap || !bInitialized || bTeleported)
	{
		bInitialized = true;
		PreviousRaw = Raw;
		SmoothedVelocity = VelocityRate = FVector::ZeroVector;
		LookAhead = LookAheadRate = FVector::ZeroVector;
		Anchor = Pivot = Raw;
		PivotRate = FVector::ZeroVector;
		GroundedZ = Raw.Z;

		State.TargetVelocity = FVector::ZeroVector;
		State.PivotLocation = Raw;
		return;
	}

	// --- Velocidade medida (independe de CharacterMovement) ---
	const FVector InstantVelocity = Dt > 0.0 ? (Raw - PreviousRaw) / Dt : FVector::ZeroVector;
	PreviousRaw = Raw;
	SmoothedVelocity = LumeMath::SmoothDamp(SmoothedVelocity, InstantVelocity, VelocityRate, VelocitySmoothTime, Dt);
	State.TargetVelocity = SmoothedVelocity;

	FVector Goal = Raw;

	// --- Estabilização vertical: pulos não movem a câmera ---
	if (VerticalTracking == ELumeVerticalTracking::StableWhileAirborne)
	{
		if (State.bTargetAirborne)
		{
			const double Tolerance = AirborneVerticalTolerance;
			Goal.Z = FMath::Clamp(GroundedZ, Raw.Z - Tolerance, Raw.Z + Tolerance);
		}
		else
		{
			GroundedZ = Raw.Z;
		}
	}

	// --- Antecipação do movimento ---
	FVector DesiredLookAhead = FVector::ZeroVector;
	if (bAnticipateMovement && F.LookAheadDistance > 0.f)
	{
		FVector Velocity = SmoothedVelocity;
		if (bPlanarLookAhead)
		{
			Velocity.Z = 0.0;
		}
		const double Speed = Velocity.Size();
		if (Speed > 1.0)
		{
			const double Alpha = FMath::Clamp(Speed / FMath::Max(1.0, static_cast<double>(SpeedForFullLookAhead)), 0.0, 1.0);
			DesiredLookAhead = (Velocity / Speed) * (static_cast<double>(F.LookAheadDistance) * Alpha);
		}
	}
	LookAhead = LumeMath::SmoothDamp(LookAhead, DesiredLookAhead, LookAheadRate, LookAheadSmoothTime, Dt);
	Goal += LookAhead;

	// --- Zona morta, no plano da câmera (direita/cima) ---
	if (bUseDeadZone && (F.DeadZone.X > 0.0 || F.DeadZone.Y > 0.0))
	{
		const FQuat ViewQuat = State.CameraRotation.Quaternion();
		const FVector Right = ViewQuat.GetRightVector();
		const FVector Up = ViewQuat.GetUpVector();

		const FVector Delta = Goal - Anchor;
		const double DX = FVector::DotProduct(Delta, Right);
		const double DZ = FVector::DotProduct(Delta, Up);
		const FVector Depth = Delta - Right * DX - Up * DZ;

		auto Excess = [](double Value, double Radius)
		{
			return FMath::Abs(Value) > Radius ? Value - FMath::Sign(Value) * Radius : 0.0;
		};

		Anchor += Depth + Right * Excess(DX, F.DeadZone.X) + Up * Excess(DZ, F.DeadZone.Y);
	}
	else
	{
		Anchor = Goal;
	}

	// --- Seguimento suave ---
	const double SmoothTime = F.TrackingSmoothTime;
	const double VerticalSmoothTime = SmoothTime * VerticalSmoothMultiplier;
	Pivot.X = LumeMath::SmoothDamp(Pivot.X, Anchor.X, PivotRate.X, SmoothTime, Dt);
	Pivot.Y = LumeMath::SmoothDamp(Pivot.Y, Anchor.Y, PivotRate.Y, SmoothTime, Dt);
	Pivot.Z = LumeMath::SmoothDamp(Pivot.Z, Anchor.Z, PivotRate.Z, VerticalSmoothTime, Dt);

	State.PivotLocation = Pivot;
}

void ULumeTrackingModule::DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const
{
#if ENABLE_DRAW_DEBUG
	if (UWorld* World = GetWorld())
	{
		const FVector Base = State.TargetLocation + Context.Framing.PivotOffset;
		DrawDebugDirectionalArrow(World, Base, Base + LookAhead, 20.f, FColor::Magenta, false, -1.f, 0, 1.5f);
		DrawDebugSphere(World, Anchor, 8.f, 8, FColor::Orange, false, -1.f, 0, 1.f);

		const FVector2D DZ = Context.Framing.DeadZone;
		DrawDebugBox(World, Anchor, FVector(1.0, DZ.X, DZ.Y), State.CameraRotation.Quaternion(), FColor::Orange, false, -1.f, 0, 1.f);
	}
#endif
}
