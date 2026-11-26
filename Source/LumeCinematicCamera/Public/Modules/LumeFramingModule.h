#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeFramingModule.generated.h"

/**
 * 2. Ajuste de enquadramento.
 * - Mantém o personagem numa posição cinematográfica na tela (Framing.ScreenOffset),
 *   com transição suave quando o enquadramento muda (zonas).
 * - Enquadramento de grupo: inclui alvos adicionais (inimigos, NPCs, objetos de interesse),
 *   deslocando o pivô e afastando a câmera para que todos caibam na tela.
 */
UCLASS(meta = (DisplayName = "Framing (Enquadramento)"))
class LUMECINEMATICCAMERA_API ULumeFramingModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	/** Suavização do deslocamento horizontal/vertical na tela (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen", meta = (ClampMin = "0", Units = "s"))
	float ScreenOffsetSmoothTime = 0.8f;

	/** Enquadra também os alvos adicionais. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing")
	bool bFrameAdditionalTargets = true;

	/** Atores extras que devem aparecer no quadro. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing", meta = (EditCondition = "bFrameAdditionalTargets"))
	TArray<TObjectPtr<AActor>> AdditionalTargets;

	/** Quanto o pivô se desloca para o centro do grupo (0 = jogador sempre no centro, 1 = centro do grupo). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing", meta = (EditCondition = "bFrameAdditionalTargets", ClampMin = "0", ClampMax = "1"))
	float GroupInfluence = 0.5f;

	/** Alvos mais distantes que isso do jogador são ignorados (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing", meta = (EditCondition = "bFrameAdditionalTargets", ClampMin = "0", Units = "cm"))
	float MaxTargetDistance = 2000.f;

	/** Margem extra em volta do grupo (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing", meta = (EditCondition = "bFrameAdditionalTargets", ClampMin = "0", Units = "cm"))
	float FramingPadding = 150.f;

	/** Afastamento máximo adicional causado pelo grupo (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing", meta = (EditCondition = "bFrameAdditionalTargets", ClampMin = "0", Units = "cm"))
	float MaxExtraDistance = 1500.f;

	/** Suavização da entrada/saída de alvos do grupo (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Framing", meta = (EditCondition = "bFrameAdditionalTargets", ClampMin = "0", Units = "s"))
	float GroupSmoothTime = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "LUME|Framing")
	void AddFramingTarget(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "LUME|Framing")
	void RemoveFramingTarget(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "LUME|Framing")
	void ClearFramingTargets();

	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;
	virtual void DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const override;

private:
	bool bInitialized = false;
	FVector2D Offset = FVector2D::ZeroVector;
	FVector2D OffsetRate = FVector2D::ZeroVector;
	FVector GroupShift = FVector::ZeroVector;
	FVector GroupShiftRate = FVector::ZeroVector;
	double GroupDistance = 0.0;
	double GroupDistanceRate = 0.0;
};
