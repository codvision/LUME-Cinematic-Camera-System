#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "LumeCameraTypes.h"
#include "LumeCameraModule.generated.h"

class ALumeCinematicCamera;

/**
 * Base de todos os módulos do LUME.
 *
 * A câmera executa sua lista de módulos em ordem, a cada frame:
 *   1. Módulos PreCompose (rastreamento, enquadramento, rotação, zoom...)
 *   2. Composição (pivô + rotação + distância + deslocamento de tela => transform da câmera)
 *   3. Módulos PostCompose (colisão, limites, profundidade de campo, ruído...)
 *
 * Crie novos módulos em C++ herdando desta classe, ou em Blueprint herdando de ULumeBlueprintCameraModule.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, CollapseCategories)
class LUMECINEMATICCAMERA_API ULumeCameraModule : public UObject
{
	GENERATED_BODY()

public:
	/** Liga/desliga o módulo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Module")
	bool bEnabled = true;

	/** Etapa do pipeline em que este módulo roda. */
	virtual ELumeModuleStage GetStage() const { return ELumeModuleStage::PreCompose; }

	/** Atualiza o módulo. Leia o Context e modifique o State. */
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) {}

	/** Desenho de depuração (somente quando a câmera tem bDrawDebug ligado). */
	virtual void DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const {}

	/** Câmera dona deste módulo. */
	UFUNCTION(BlueprintPure, Category = "LUME|Module")
	ALumeCinematicCamera* GetOwningCamera() const;

	virtual UWorld* GetWorld() const override;
};

/**
 * Módulo programável em Blueprint. Crie uma classe Blueprint filha e implemente "Update Module".
 * Retorne o State modificado.
 */
UCLASS(Abstract, Blueprintable, meta = (DisplayName = "Blueprint Module"))
class LUMECINEMATICCAMERA_API ULumeBlueprintCameraModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Module")
	ELumeModuleStage Stage = ELumeModuleStage::PostCompose;

	virtual ELumeModuleStage GetStage() const override { return Stage; }
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "LUME|Module", meta = (DisplayName = "Update Module"))
	FLumeCameraState ReceiveUpdateModule(const FLumeCameraContext& Context, const FLumeCameraState& State);
};
