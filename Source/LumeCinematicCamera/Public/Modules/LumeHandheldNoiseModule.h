#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeHandheldNoiseModule.generated.h"

/**
 * Efeito "câmera na mão": ruído Perlin suave na rotação.
 * Amplitude e frequência vêm do enquadramento (desligado por padrão; o preset Terror liga).
 */
UCLASS(meta = (DisplayName = "Handheld Noise (Câmera na Mão)"))
class LUMECINEMATICCAMERA_API ULumeHandheldNoiseModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	/** Multiplicador global da amplitude. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (ClampMin = "0"))
	float AmplitudeScale = 1.f;

	/** Peso por eixo (Pitch, Yaw, Roll). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld")
	FVector AxisWeights = FVector(1.0, 1.0, 0.5);

	virtual ELumeModuleStage GetStage() const override { return ELumeModuleStage::PostCompose; }
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

private:
	float NoiseTime = 0.f;
};
