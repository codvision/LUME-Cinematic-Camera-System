#pragma once

#include "CoreMinimal.h"
#include "Modules/LumeCameraModule.h"
#include "LumeShakeModule.generated.h"

/**
 * Tremor e impactos.
 *
 * - Trauma: cada golpe soma "trauma" (0..1), que diminui sozinho com o tempo. O tremor cresce
 *   com o quadrado do trauma: golpes pequenos quase não tremem, golpes seguidos se acumulam.
 * - Empurrão com mola: a câmera é deslocada numa direção e volta quicando (impacto físico).
 * - Aterrissagem: quedas altas geram tranco automático.
 * - Tremor contínuo: terremotos, máquinas, veículos.
 *
 * Os offsets são no espaço da câmera, ideais para 2.5D (tremor no plano da tela).
 */
UCLASS(meta = (DisplayName = "Shake & Impacts (Tremor)"))
class LUMECINEMATICCAMERA_API ULumeShakeModule : public ULumeCameraModule
{
	GENERATED_BODY()

public:
	/** Intensidade geral de todos os tremores (0 = desligado; útil como opção de acessibilidade). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0", UIMax = "2"))
	float GlobalIntensity = 1.f;

	/** Quanto trauma some por segundo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0"))
	float TraumaDecay = 1.3f;

	/** Expoente do trauma (2 = golpes pequenos bem sutis). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "1", ClampMax = "4"))
	float TraumaExponent = 2.f;

	/** Deslocamento máximo (cm) no espaço da câmera: X = profundidade, Y = lateral, Z = vertical. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FVector MaxOffset = FVector(0.0, 28.0, 22.0);

	/** Rotação máxima (graus): X = pitch, Y = yaw, Z = roll. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
	FVector MaxAngles = FVector(0.8, 0.8, 2.0);

	/** Velocidade do tremor (Hz). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake", meta = (ClampMin = "0.1", UIMax = "40"))
	float Frequency = 15.f;

	/** Rigidez da mola do empurrão. Maior = volta mais rápido. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "1"))
	float SpringStiffness = 220.f;

	/** Amortecimento da mola. Menor = quica mais. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "0"))
	float SpringDamping = 15.f;

	/** Deslocamento máximo do empurrão (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Impulse", meta = (ClampMin = "0", Units = "cm"))
	float MaxImpulseOffset = 80.f;

	/** Tranco automático ao aterrissar de quedas altas. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing")
	bool bShakeOnLanding = true;

	/** Velocidade de queda (cm/s) a partir da qual há tranco. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing", meta = (EditCondition = "bShakeOnLanding", ClampMin = "0"))
	float LandingSpeedThreshold = 900.f;

	/** Velocidade de queda (cm/s) do tranco máximo. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing", meta = (EditCondition = "bShakeOnLanding", ClampMin = "1"))
	float LandingSpeedForMax = 2000.f;

	/** Trauma do tranco máximo de aterrissagem. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing", meta = (EditCondition = "bShakeOnLanding", ClampMin = "0", ClampMax = "1"))
	float LandingMaxTrauma = 0.35f;

	/** Força do tranco para baixo ao aterrissar (fração da velocidade de queda). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landing", meta = (EditCondition = "bShakeOnLanding", ClampMin = "0"))
	float LandingImpulseScale = 0.12f;

	/** Soma trauma (0..1). Ex.: 0.2 golpe leve, 0.5 golpe forte, 0.9 explosão. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake")
	void AddTrauma(float Amount);

	/** Empurra a câmera (velocidade em cm/s, no mundo). A mola traz de volta. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake")
	void AddImpulse(FVector WorldImpulse);

	/** Toca um preset. Direction (opcional) = direção do empurrão no mundo. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake")
	void PlayShake(ELumeShakePreset Preset, float Scale = 1.f, FVector Direction = FVector::ZeroVector);

	/** Tremor contínuo mínimo (0..1). Use 0 para parar. Ex.: terremoto = 0.4. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake")
	void SetContinuousShake(float Amount);

	UFUNCTION(BlueprintPure, Category = "LUME|Shake")
	float GetTrauma() const { return Trauma; }

	virtual ELumeModuleStage GetStage() const override { return ELumeModuleStage::PostCompose; }
	virtual void UpdateModule(const FLumeCameraContext& Context, FLumeCameraState& State) override;

private:
	float Trauma = 0.f;
	float ContinuousTrauma = 0.f;
	float NoiseTime = 0.f;
	FVector SpringOffset = FVector::ZeroVector;
	FVector SpringVelocity = FVector::ZeroVector;
	FRotator LastCameraRotation = FRotator::ZeroRotator;
	bool bWasAirborne = false;
	double AirborneFallSpeed = 0.0;
};
