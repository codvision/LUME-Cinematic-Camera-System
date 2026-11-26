#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeRotationModule.generated.h"

/**
 * 2. Rotação suave da câmera.
 * A rotação vem do modo escolhido e é sempre suavizada (Framing.RotationSmoothTime),
 * então mudanças de ângulo entre zonas são transições, nunca cortes.
 */
UCLASS(meta = (DisplayName = "Rotation (Rotação)"))
class LUMECINEMATICCAMERA_API ULumeRotationModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	ELumeRotationMode Mode = ELumeRotationMode::WorldFixed;

	/** Velocidade mínima (cm/s) para o modo Follow Movement Direction atualizar a direção. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0"))
	float MovementThreshold = 60.f;

	/** Suavização usada no modo Control Rotation (s). Pequena para manter a resposta ao jogador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "0", Units = "s"))
	float ControlRotationSmoothTime = 0.05f;

	/** Limites de pitch (modos não fixos). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "-89", ClampMax = "89"))
	float MinPitch = -75.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (ClampMin = "-89", ClampMax = "89"))
	float MaxPitch = 45.f;

	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

private:
	bool bInitialized = false;
	FRotator Current = FRotator::ZeroRotator;
	FVector Rate = FVector::ZeroVector;
	double HeadingYaw = 0.0;
};
