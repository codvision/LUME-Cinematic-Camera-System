#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeDepthOfFieldModule.generated.h"

/**
 * 4. Profundidade de campo (e efeitos de lente).
 * Foco automático no jogador com suavização; abertura, vinheta e granulação
 * vêm do enquadramento, então cada zona pode ter seu próprio "look".
 */
UCLASS(meta = (DisplayName = "Depth of Field (Profundidade de Campo)"))
class LUMECINEMATICCAMERA_API ULumeDepthOfFieldModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth of Field")
	bool bEnableDepthOfField = true;

	/** Foca automaticamente no jogador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth of Field", meta = (EditCondition = "bEnableDepthOfField"))
	bool bAutoFocusOnTarget = true;

	/** Distância de foco manual (cm), quando o foco automático está desligado. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth of Field", meta = (EditCondition = "bEnableDepthOfField && !bAutoFocusOnTarget", ClampMin = "0", Units = "cm"))
	float ManualFocusDistance = 600.f;

	/** Suavização do "puxar foco" (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth of Field", meta = (EditCondition = "bEnableDepthOfField", ClampMin = "0", Units = "s"))
	float FocusSmoothTime = 0.25f;

	/** Largura do sensor (mm). 24.576 = Super 35. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Depth of Field", meta = (EditCondition = "bEnableDepthOfField", ClampMin = "1"))
	float SensorWidth = 24.576f;

	/** Aplica vinheta e granulação do enquadramento. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens Effects")
	bool bApplyLensEffects = true;

	virtual ELumeModuleStage GetStage() const override { return ELumeModuleStage::PostCompose; }
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

private:
	bool bInitialized = false;
	double Focus = 0.0;
	double FocusRate = 0.0;
};
