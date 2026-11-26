#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeTrackingModule.generated.h"

/**
 * 1. Rastreamento inteligente.
 * - Segue o jogador suavemente (mola criticamente amortecida, sem trancos).
 * - Antecipa a direção do movimento (look-ahead baseado na velocidade suavizada).
 * - Zona morta: pequenos movimentos não mexem a câmera.
 * - Estabilização vertical: ignora pulos para evitar movimentos bruscos.
 * - Detecção de teleporte.
 * A velocidade é calculada a partir da posição do alvo, então funciona com qualquer ator.
 */
UCLASS(meta = (DisplayName = "Tracking (Rastreamento)"))
class LUMECINEMATICCAMERA_API ULumeTrackingModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	/** Antecipa a direção do movimento, deslocando o pivô para frente do jogador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anticipation")
	bool bAnticipateMovement = true;

	/** Velocidade (cm/s) em que a antecipação atinge 100% de Framing.LookAheadDistance. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anticipation", meta = (EditCondition = "bAnticipateMovement", ClampMin = "1"))
	float SpeedForFullLookAhead = 500.f;

	/** Suavização da antecipação (s). Valores altos evitam que a câmera "balance" ao mudar de direção. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anticipation", meta = (EditCondition = "bAnticipateMovement", ClampMin = "0", Units = "s"))
	float LookAheadSmoothTime = 0.9f;

	/** Antecipação apenas no plano horizontal. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anticipation", meta = (EditCondition = "bAnticipateMovement"))
	bool bPlanarLookAhead = true;

	/** Suavização da velocidade medida do alvo (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anticipation", meta = (ClampMin = "0", Units = "s"))
	float VelocitySmoothTime = 0.2f;

	/** Usa a zona morta definida no enquadramento (Framing.DeadZone). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dead Zone")
	bool bUseDeadZone = true;

	/** Comportamento vertical. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical")
	ELumeVerticalTracking VerticalTracking = ELumeVerticalTracking::StableWhileAirborne;

	/** No ar, a câmera só acompanha quando o alvo se afasta mais que isso da altura do chão (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical", meta = (ClampMin = "0", Units = "cm"))
	float AirborneVerticalTolerance = 200.f;

	/** Multiplicador do tempo de suavização vertical (vertical costuma ser mais suave que horizontal). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vertical", meta = (ClampMin = "0"))
	float VerticalSmoothMultiplier = 1.5f;

	/** Se o alvo andar mais que isso em um frame, a câmera salta direto (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Safety", meta = (ClampMin = "0", Units = "cm"))
	float TeleportDistance = 2000.f;

	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;
	virtual void DrawDebug(const FLumeCameraContext& Context, const FLumeCameraState& State) const override;

private:
	bool bInitialized = false;
	FVector PreviousRaw = FVector::ZeroVector;
	FVector SmoothedVelocity = FVector::ZeroVector;
	FVector VelocityRate = FVector::ZeroVector;
	FVector LookAhead = FVector::ZeroVector;
	FVector LookAheadRate = FVector::ZeroVector;
	FVector Anchor = FVector::ZeroVector;
	FVector Pivot = FVector::ZeroVector;
	FVector PivotRate = FVector::ZeroVector;
	double GroundedZ = 0.0;
};
