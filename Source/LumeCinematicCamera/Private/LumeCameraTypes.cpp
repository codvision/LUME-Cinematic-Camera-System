#include "LumeCameraTypes.h"

void FLumeFramingOverrides::ApplyTo(FLumeFramingSettings& Out, float Alpha) const
{
	Alpha = FMath::Clamp(Alpha, 0.f, 1.f);
	if (Alpha <= 0.f)
	{
		return;
	}
	const double AlphaD = Alpha;

#define LUME_BLEND_FLOAT(Field)  if (bOverride_##Field) { Out.Field = FMath::Lerp(Out.Field, Field, Alpha); }
#define LUME_BLEND_VECTOR(Field) if (bOverride_##Field) { Out.Field = FMath::Lerp(Out.Field, Field, AlphaD); }

	LUME_BLEND_FLOAT(Distance)
	LUME_BLEND_FLOAT(FieldOfView)
	LUME_BLEND_VECTOR(PivotOffset)
	LUME_BLEND_VECTOR(ScreenOffset)
	LUME_BLEND_FLOAT(LookAheadDistance)
	LUME_BLEND_VECTOR(DeadZone)
	LUME_BLEND_FLOAT(TrackingSmoothTime)
	LUME_BLEND_FLOAT(RotationSmoothTime)
	LUME_BLEND_FLOAT(ZoomSmoothTime)
	LUME_BLEND_FLOAT(Aperture)
	LUME_BLEND_FLOAT(FocusOffset)
	LUME_BLEND_FLOAT(VignetteIntensity)
	LUME_BLEND_FLOAT(FilmGrainIntensity)
	LUME_BLEND_FLOAT(NoiseAmplitude)
	LUME_BLEND_FLOAT(NoiseFrequency)

#undef LUME_BLEND_FLOAT
#undef LUME_BLEND_VECTOR

	// Ângulos misturados pelo caminho mais curto.
	if (bOverride_Pitch)
	{
		Out.Rotation.Pitch += FMath::FindDeltaAngleDegrees(Out.Rotation.Pitch, static_cast<double>(Pitch)) * AlphaD;
	}
	if (bOverride_Yaw)
	{
		Out.Rotation.Yaw += FMath::FindDeltaAngleDegrees(Out.Rotation.Yaw, static_cast<double>(Yaw)) * AlphaD;
	}
	if (bOverride_Roll)
	{
		Out.Rotation.Roll += FMath::FindDeltaAngleDegrees(Out.Rotation.Roll, static_cast<double>(Roll)) * AlphaD;
	}
}

FLumeFramingOverrides FLumeFramingOverrides::MakePreset(ELumeZonePreset Preset)
{
	FLumeFramingOverrides O;

	switch (Preset)
	{
	case ELumeZonePreset::Corridor:
		// Corredor: aproxima a câmera, fecha o FOV, reduz antecipação e zona morta.
		O.bOverride_Distance = true;			O.Distance = 340.f;
		O.bOverride_FieldOfView = true;			O.FieldOfView = 60.f;
		O.bOverride_Pitch = true;				O.Pitch = -10.f;
		O.bOverride_LookAheadDistance = true;	O.LookAheadDistance = 70.f;
		O.bOverride_DeadZone = true;			O.DeadZone = FVector2D(15.0, 10.0);
		O.bOverride_Aperture = true;			O.Aperture = 4.f;
		break;

	case ELumeZonePreset::LargeRoom:
		// Sala grande: afasta e eleva a câmera, abre o FOV, mais antecipação, movimentos lentos.
		O.bOverride_Distance = true;			O.Distance = 1150.f;
		O.bOverride_FieldOfView = true;			O.FieldOfView = 78.f;
		O.bOverride_Pitch = true;				O.Pitch = -28.f;
		O.bOverride_LookAheadDistance = true;	O.LookAheadDistance = 280.f;
		O.bOverride_ZoomSmoothTime = true;		O.ZoomSmoothTime = 1.6f;
		O.bOverride_Aperture = true;			O.Aperture = 11.f;
		break;

	case ELumeZonePreset::Horror:
		// Terror: personagem descentralizado, dutch angle, foco raso, vinheta forte,
		// granulação, câmera "pesada" e câmera na mão sutil. Pouca visão à frente.
		O.bOverride_Distance = true;			O.Distance = 420.f;
		O.bOverride_FieldOfView = true;			O.FieldOfView = 52.f;
		O.bOverride_ScreenOffset = true;		O.ScreenOffset = FVector2D(0.32, -0.12);
		O.bOverride_Pitch = true;				O.Pitch = -6.f;
		O.bOverride_Roll = true;				O.Roll = 5.f;
		O.bOverride_LookAheadDistance = true;	O.LookAheadDistance = 30.f;
		O.bOverride_TrackingSmoothTime = true;	O.TrackingSmoothTime = 0.75f;
		O.bOverride_RotationSmoothTime = true;	O.RotationSmoothTime = 1.2f;
		O.bOverride_Aperture = true;			O.Aperture = 1.8f;
		O.bOverride_VignetteIntensity = true;	O.VignetteIntensity = 0.9f;
		O.bOverride_FilmGrainIntensity = true;	O.FilmGrainIntensity = 0.35f;
		O.bOverride_NoiseAmplitude = true;		O.NoiseAmplitude = 0.6f;
		O.bOverride_NoiseFrequency = true;		O.NoiseFrequency = 0.25f;
		break;

	case ELumeZonePreset::Custom:
	default:
		break;
	}

	return O;
}
