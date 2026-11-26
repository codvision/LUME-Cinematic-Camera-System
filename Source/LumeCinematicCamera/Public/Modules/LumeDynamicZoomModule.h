#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeDynamicZoomModule.generated.h"

/**
 * 2. Zoom automático.
 * - Distância e FOV vêm do enquadramento (perfil base + zonas), sempre suavizados.
 * - Afasta a câmera e abre o FOV conforme a velocidade (sensação de movimento).
 * - Aproxima lentamente quando o jogador fica parado (push-in cinematográfico).
 * - Respeita a distância mínima pedida pelo enquadramento de grupo.
 */
UCLASS(meta = (DisplayName = "Dynamic Zoom (Zoom Automático)"))
class LUMECINEMATICCAMERA_API ULumeDynamicZoomModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Zoom")
	bool bSpeedZoom = true;

	/** Velocidade (cm/s) em que o zoom por velocidade chega ao máximo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Zoom", meta = (EditCondition = "bSpeedZoom", ClampMin = "1"))
	float SpeedForMaxZoom = 600.f;

	/** Distância extra na velocidade máxima (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Zoom", meta = (EditCondition = "bSpeedZoom", Units = "cm"))
	float ExtraDistanceAtMaxSpeed = 150.f;

	/** FOV extra na velocidade máxima (graus). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Zoom", meta = (EditCondition = "bSpeedZoom", Units = "deg"))
	float ExtraFOVAtMaxSpeed = 4.f;

	/** Suavização da resposta à velocidade (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speed Zoom", meta = (EditCondition = "bSpeedZoom", ClampMin = "0", Units = "s"))
	float SpeedResponseTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Push-In")
	bool bIdlePushIn = true;

	/** Tempo parado antes de aproximar (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Push-In", meta = (EditCondition = "bIdlePushIn", ClampMin = "0", Units = "s"))
	float IdleDelay = 4.f;

	/** Multiplicador da distância quando parado (ex.: 0.85 = 15% mais perto). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Push-In", meta = (EditCondition = "bIdlePushIn", ClampMin = "0.1", ClampMax = "1"))
	float IdleDistanceMultiplier = 0.85f;

	/** Duração da aproximação lenta (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Idle Push-In", meta = (EditCondition = "bIdlePushIn", ClampMin = "0", Units = "s"))
	float IdlePushInTime = 3.f;

	/** Limites absolutos de distância (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "0", Units = "cm"))
	float MinDistance = 80.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits", meta = (ClampMin = "0", Units = "cm"))
	float MaxDistance = 6000.f;

	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

private:
	bool bInitialized = false;
	double Distance = 0.0;
	double DistanceRate = 0.0;
	double Fov = 70.0;
	double FovRate = 0.0;
	double SpeedAlpha = 0.0;
	double SpeedAlphaRate = 0.0;
	double IdleTime = 0.0;
	double IdleAlpha = 0.0;
	double IdleAlphaRate = 0.0;
};
