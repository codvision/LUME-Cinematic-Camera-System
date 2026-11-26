#include "Modules/LumeRotationModule.h"
#include "LumeCameraMath.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

void ULumeRotationModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const FLumeFramingSettings& F = Context.Framing;
	const AActor* Target = Context.Target;

	if (!bInitialized && Target)
	{
		HeadingYaw = Target->GetActorRotation().Yaw;
	}

	FRotator Desired = F.Rotation;
	double SmoothTime = F.RotationSmoothTime;

	switch (Mode)
	{
	case ELumeRotationMode::FollowTargetHeading:
		if (Target)
		{
			Desired.Yaw = Target->GetActorRotation().Yaw + F.Rotation.Yaw;
		}
		break;

	case ELumeRotationMode::FollowMovementDirection:
	{
		FVector Planar = State.TargetVelocity;
		Planar.Z = 0.0;
		if (Planar.SizeSquared() > FMath::Square(static_cast<double>(MovementThreshold)))
		{
			HeadingYaw = Planar.Rotation().Yaw;
		}
		Desired.Yaw = HeadingYaw + F.Rotation.Yaw;
		break;
	}

	case ELumeRotationMode::ControlRotation:
		if (const APawn* Pawn = Cast<APawn>(Target))
		{
			if (const AController* Controller = Pawn->GetController())
			{
				const FRotator Control = Controller->GetControlRotation();
				Desired.Pitch = FRotator::NormalizeAxis(Control.Pitch) + F.Rotation.Pitch;
				Desired.Yaw = Control.Yaw + F.Rotation.Yaw;
			}
		}
		SmoothTime = ControlRotationSmoothTime;
		break;

	case ELumeRotationMode::WorldFixed:
	default:
		break;
	}

	if (Mode != ELumeRotationMode::WorldFixed)
	{
		Desired.Pitch = FMath::Clamp(Desired.Pitch, static_cast<double>(MinPitch), static_cast<double>(MaxPitch));
	}
	Desired.Normalize();

	if (Context.bSnap || !bInitialized)
	{
		bInitialized = true;
		Current = Desired;
		Rate = FVector::ZeroVector;
	}
	else
	{
		Current = LumeMath::SmoothDampRotator(Current, Desired, Rate, SmoothTime, Context.DeltaTime);
	}

	State.Rotation = Current;
}
