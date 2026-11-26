#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeBoundsModule.generated.h"

class ALumeCameraBoundsVolume;

/**
 * 4. Limites de movimentação.
 * Mantém a câmera dentro de volumes LUME Camera Bounds e das zonas com
 * "Constrain Camera To Zone" (misturado pelo peso da zona = transição suave).
 */
UCLASS(meta = (DisplayName = "Bounds (Limites)"))
class LUMECINEMATICCAMERA_API ULumeBoundsModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	/** Usa todos os LUME Camera Bounds do nível. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	bool bUseAllBoundsVolumes = true;

	/** Volumes específicos (quando Use All Bounds Volumes está desligado). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds", meta = (EditCondition = "!bUseAllBoundsVolumes"))
	TArray<TObjectPtr<ALumeCameraBoundsVolume>> BoundsVolumes;

	/** Aplica as restrições das zonas cinematográficas. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	bool bApplyZoneConstraints = true;

	/** Distância mínima das bordas (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds", meta = (ClampMin = "0", Units = "cm"))
	float EdgeMargin = 0.f;

	/** Depois de limitar a posição, gira a câmera para continuar olhando o jogador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bounds")
	bool bReaimAfterClamp = false;

	virtual ELumeModuleStage GetStage() const override { return ELumeModuleStage::PostCompose; }
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;
};
