#include "Modules/LumeShakeModule.h"
#include "GameFramework/Actor.h"

void ULumeShakeModule::AddTrauma(float Amount)
{
	Trauma = FMath::Clamp(Trauma + Amount, 0.f, 1.f);
}

void ULumeShakeModule::AddImpulse(FVector WorldImpulse)
{
	SpringVelocity += WorldImpulse;
}

void ULumeShakeModule::SetContinuousShake(float Amount)
{
	ContinuousTrauma = FMath::Clamp(Amount, 0.f, 1.f);
}

void ULumeShakeModule::PlayShake(ELumeShakePreset Preset, float Scale, FVector Direction)
{
	Scale = FMath::Max(0.f, Scale);

	const FQuat View = LastCameraRotation.Quaternion();
	const FVector RandomSide = View.GetRightVector() * (FMath::RandBool() ? 1.0 : -1.0);

	float PresetTrauma = 0.f;
	double ImpulseSpeed = 0.0;
	FVector DefaultDirection = RandomSide;

	switch (Preset)
	{
	case ELumeShakePreset::LightHit:
		PresetTrauma = 0.25f;	ImpulseSpeed = 90.0;
		break;
	case ELumeShakePreset::HeavyHit:
		PresetTrauma = 0.5f;	ImpulseSpeed = 180.0;
		break;
	case ELumeShakePreset::Explosion:
		PresetTrauma = 0.85f;	ImpulseSpeed = 260.0;	DefaultDirection = FVector::UpVector;
		break;
	case ELumeShakePreset::Landing:
		PresetTrauma = 0.3f;	ImpulseSpeed = 150.0;	DefaultDirection = -FVector::UpVector;
		break;
	case ELumeShakePreset::Jumpscare:
		// Tranco seco em direção à cena (a câmera "dá um passo para trás").
		PresetTrauma = 0.6f;	ImpulseSpeed = 220.0;	DefaultDirection = -View.GetForwardVector();
		break;
	default:
		break;
	}

	const FVector Dir = Direction.IsNearlyZero() ? DefaultDirection : Direction.GetSafeNormal();
	AddTrauma(PresetTrauma * Scale);
	AddImpulse(Dir * (ImpulseSpeed * Scale));
}

void ULumeShakeModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const double Dt = Context.DeltaTime;
	LastCameraRotation = State.CameraRotation;

	if (Context.bSnap)
	{
		SpringOffset = SpringVelocity = FVector::ZeroVector;
	}

	// --- Aterrissagem ---
	if (bShakeOnLanding)
	{
		if (State.bTargetAirborne)
		{
			if (Context.Target)
			{
				AirborneFallSpeed = FMath::Max(0.0, -Context.Target->GetVelocity().Z);
			}
		}
		else if (bWasAirborne && AirborneFallSpeed > LandingSpeedThreshold)
		{
			const double Range = FMath::Max(1.0, static_cast<double>(LandingSpeedForMax - LandingSpeedThreshold));
			const double Alpha = FMath::Clamp((AirborneFallSpeed - LandingSpeedThreshold) / Range, 0.0, 1.0);
			AddTrauma(LandingMaxTrauma * static_cast<float>(Alpha));
			AddImpulse(-FVector::UpVector * (AirborneFallSpeed * LandingImpulseScale * (0.5 + 0.5 * Alpha)));
		}
		if (!State.bTargetAirborne)
		{
			AirborneFallSpeed = 0.0;
		}
	}
	bWasAirborne = State.bTargetAirborne;

	// --- Mola do empurrão (subpassos para estabilidade) ---
	double Remaining = Dt;
	while (Remaining > 0.0)
	{
		const double Step = FMath::Min(Remaining, 1.0 / 120.0);
		const FVector Accel = -SpringOffset * static_cast<double>(SpringStiffness) - SpringVelocity * static_cast<double>(SpringDamping);
		SpringVelocity += Accel * Step;
		SpringOffset += SpringVelocity * Step;
		Remaining -= Step;
	}
	SpringOffset = SpringOffset.GetClampedToMaxSize(MaxImpulseOffset);

	// --- Trauma ---
	Trauma = FMath::Max(0.f, Trauma - TraumaDecay * static_cast<float>(Dt));
	NoiseTime += static_cast<float>(Dt) * Frequency;
	if (NoiseTime > 10000.f)
	{
		NoiseTime -= 10000.f;
	}

	const float Effective = FMath::Clamp(FMath::Max(Trauma, ContinuousTrauma), 0.f, 1.f);
	const double Shake = FMath::Pow(Effective, TraumaExponent) * GlobalIntensity;

	FVector Offset = SpringOffset * static_cast<double>(GlobalIntensity);

	if (Shake > 0.0)
	{
		const FQuat View = State.CameraRotation.Quaternion();
		auto Noise = [this](float Seed) { return static_cast<double>(FMath::PerlinNoise1D(NoiseTime + Seed)); };

		Offset += View.GetForwardVector() * (Noise(11.3f) * MaxOffset.X * Shake)
			+ View.GetRightVector() * (Noise(29.7f) * MaxOffset.Y * Shake)
			+ View.GetUpVector() * (Noise(47.1f) * MaxOffset.Z * Shake);

		State.CameraRotation += FRotator(
			Noise(63.9f) * MaxAngles.X * Shake,
			Noise(81.5f) * MaxAngles.Y * Shake,
			Noise(97.3f) * MaxAngles.Z * Shake);
	}

	State.CameraLocation += Offset;
}
