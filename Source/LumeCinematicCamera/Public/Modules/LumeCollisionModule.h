#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Modules/LumeCameraModule.h"
#include "LumeCollisionModule.generated.h"

/**
 * Evita que a câmera atravesse paredes: aproxima instantaneamente ao colidir
 * e volta suavemente para a distância desejada.
 */
UCLASS(meta = (DisplayName = "Collision (Colisão)"))
class LUMECINEMATICCAMERA_API ULumeCollisionModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "0", Units = "cm"))
	float ProbeRadius = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TEnumAsByte<ECollisionChannel> ProbeChannel = ECC_Camera;

	/** Distância mínima do pivô ao colidir (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "0", Units = "cm"))
	float MinDistance = 50.f;

	/** Tempo para voltar à distância desejada após a colisão (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (ClampMin = "0", Units = "s"))
	float ReturnSmoothTime = 0.45f;

	virtual ELumeModuleStage GetStage() const override { return ELumeModuleStage::PostCompose; }
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

private:
	double CurrentDistance = -1.0;
	double DistanceRate = 0.0;
};
