#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "LumeInterestPointComponent.generated.h"

class USphereComponent;

/**
 * Ponto de interesse da câmera.
 *
 * Adicione em qualquer Blueprint (inimigo, porta, baú, chefão...) ou use o ator
 * "LUME Camera Interest Point" no mapa. Quando o jogador se aproxima, a câmera desliza
 * suavemente na direção do ponto e pode se afastar para mostrar os dois na tela.
 */
UCLASS(ClassGroup = (LUME), Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName = "LUME Interest Point"))
class LUMECINEMATICCAMERA_API ULumeInterestPointComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	ULumeInterestPointComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest")
	bool bInterestEnabled = true;

	/** Distância do jogador em que o ponto começa a atrair a câmera (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest", meta = (ClampMin = "0", Units = "cm"))
	float ActivationRadius = 900.f;

	/** Dentro desta distância a atração é total (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest", meta = (ClampMin = "0", Units = "cm"))
	float FullInfluenceRadius = 350.f;

	/** Quanto a câmera se desloca em direção ao ponto (0 = nada, 0.5 = metade do caminho, 1 = centraliza o ponto). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest", meta = (ClampMin = "0", ClampMax = "1"))
	float Influence = 0.4f;

	/** Deslocamento máximo causado por este ponto (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest", meta = (ClampMin = "0", Units = "cm"))
	float MaxShift = 450.f;

	/** Afasta a câmera para que jogador e ponto caibam na tela. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest")
	bool bKeepInFrame = false;

	/** Margem em volta do ponto ao manter no quadro (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest", meta = (EditCondition = "bKeepInFrame", ClampMin = "0", Units = "cm"))
	float FramePadding = 200.f;

	/** Distância extra (cm) que a câmera se afasta enquanto o ponto está ativo (negativo = aproxima). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Interest", meta = (Units = "cm"))
	float ExtraDistance = 0.f;

	/** Peso (0..1) deste ponto para um jogador nesta posição, já com a queda por distância. */
	UFUNCTION(BlueprintPure, Category = "LUME|Interest")
	float GetInfluenceAt(const FVector& PlayerLocation) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};

/** Ator de ponto de interesse para colocar no mapa. As esferas mostram os raios no editor. */
UCLASS(Blueprintable, ClassGroup = (LUME), meta = (DisplayName = "LUME Camera Interest Point"))
class LUMECINEMATICCAMERA_API ALumeCameraInterestPoint : public AActor
{
	GENERATED_BODY()

public:
	ALumeCameraInterestPoint();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LUME")
	TObjectPtr<ULumeInterestPointComponent> InterestPoint;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<USphereComponent> ActivationSphere;

	UPROPERTY()
	TObjectPtr<USphereComponent> FullInfluenceSphere;
#endif

	virtual void OnConstruction(const FTransform& Transform) override;
};
