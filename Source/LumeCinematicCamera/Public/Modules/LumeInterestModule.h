#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeInterestModule.generated.h"

/**
 * Pontos de interesse e "olhar em volta".
 *
 * - Pontos de interesse (LUME Interest Point): perto deles, a câmera desliza suavemente
 *   em direção ao ponto, pode se afastar (Extra Distance) e mantê-lo no quadro.
 * - Olhar em volta: chame "Set Look Input" com o analógico/setas (-1..1) e a câmera espia
 *   para os lados e para cima/baixo; ao soltar, volta sozinha. Clássico em jogos 2.5D.
 */
UCLASS(meta = (DisplayName = "Interest & Look Around (Pontos de Interesse)"))
class LUMECINEMATICCAMERA_API ULumeInterestModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interest")
	bool bUseInterestPoints = true;

	/** Suavidade da atração pelos pontos (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interest", meta = (EditCondition = "bUseInterestPoints", ClampMin = "0", Units = "s"))
	float InterestSmoothTime = 1.2f;

	/** Deslocamento total máximo somando todos os pontos (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interest", meta = (EditCondition = "bUseInterestPoints", ClampMin = "0", Units = "cm"))
	float MaxTotalShift = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look Around")
	bool bAllowLookAround = true;

	/** Quanto a câmera espia com o analógico no máximo (cm): X = lateral, Y = vertical. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look Around", meta = (EditCondition = "bAllowLookAround"))
	FVector2D LookAroundDistance = FVector2D(300.0, 220.0);

	/** Suavidade ao espiar (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look Around", meta = (EditCondition = "bAllowLookAround", ClampMin = "0", Units = "s"))
	float LookSmoothTime = 0.45f;

	/** Suavidade ao voltar ao centro (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look Around", meta = (EditCondition = "bAllowLookAround", ClampMin = "0", Units = "s"))
	float LookReturnSmoothTime = 0.8f;

	/** Valores do analógico abaixo disso são ignorados. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look Around", meta = (EditCondition = "bAllowLookAround", ClampMin = "0", ClampMax = "0.9"))
	float LookInputDeadZone = 0.15f;

	/**
	 * Entrada de "olhar em volta" (-1..1 em X e Y). Chame a cada frame enquanto o botão/analógico
	 * estiver pressionado (ex.: evento "Triggered" do Enhanced Input). Ao parar de chamar, a câmera volta.
	 */
	UFUNCTION(BlueprintCallable, Category = "LUME|Look Around")
	void SetLookInput(FVector2D Input);

	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;
	virtual void DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const override;

private:
	bool bInitialized = false;
	FVector Shift = FVector::ZeroVector;
	FVector ShiftRate = FVector::ZeroVector;
	double Extra = 0.0;
	double ExtraRate = 0.0;
	double MinDistance = 0.0;
	double MinDistanceRate = 0.0;
	FVector2D LookInput = FVector2D::ZeroVector;
	uint64 LookInputFrame = 0;
	FVector2D LookOffset = FVector2D::ZeroVector;
	FVector2D LookRate = FVector2D::ZeroVector;
};
