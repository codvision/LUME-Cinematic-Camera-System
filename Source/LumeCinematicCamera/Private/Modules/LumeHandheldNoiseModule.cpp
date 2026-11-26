#include "Modules/LumeHandheldNoiseModule.h"

void ULumeHandheldNoiseModule::UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State)
{
	const FLumeFramingSettings& F = Context.Framing;

	// O tempo acumula com a frequência atual: mudar a frequência não causa saltos.
	NoiseTime += Context.DeltaTime * F.NoiseFrequency;
	if (NoiseTime > 10000.f)
	{
		NoiseTime -= 10000.f;
	}

	const double Amplitude = static_cast<double>(F.NoiseAmplitude) * AmplitudeScale;
	if (Amplitude <= 0.0)
	{
		return;
	}

	const double Pitch = FMath::PerlinNoise1D(NoiseTime) * Amplitude * AxisWeights.X;
	const double Yaw = FMath::PerlinNoise1D(NoiseTime + 37.17f) * Amplitude * AxisWeights.Y;
	const double Roll = FMath::PerlinNoise1D(NoiseTime + 71.53f) * Amplitude * AxisWeights.Z;

	State.CameraRotation += FRotator(Pitch, Yaw, Roll);
}
