#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LumeCameraTypes.h"
#include "LumeCameraLibrary.generated.h"

class ULumeCinematicCameraComponent;

/**
 * Nós de Blueprint do LUME que funcionam de qualquer lugar
 * (inimigo, armadilha, Level Blueprint...), sem precisar achar a câmera antes.
 */
UCLASS()
class LUMECINEMATICCAMERA_API ULumeCameraLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Câmera LUME que o jogador está vendo agora. */
	UFUNCTION(BlueprintPure, Category = "LUME", meta = (WorldContext = "WorldContextObject", DisplayName = "Get Active LUME Camera"))
	static ULumeCinematicCameraComponent* GetActiveLumeCamera(const UObject* WorldContextObject, int32 PlayerIndex = 0);

	/** Treme a câmera do jogador com um preset (golpe, explosão, aterrissagem, susto). */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "Direction,PlayerIndex", DisplayName = "LUME Shake Camera"))
	static void LumeShakeCamera(const UObject* WorldContextObject, ELumeShakePreset Preset, float Scale = 1.f, FVector Direction = FVector::ZeroVector, int32 PlayerIndex = 0);

	/**
	 * Tremor com origem num ponto do mundo (explosão, impacto de chefão).
	 * Força total até InnerRadius, diminuindo até zero em OuterRadius. Empurra para longe da origem.
	 */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake", meta = (WorldContext = "WorldContextObject", DisplayName = "LUME Shake At Location"))
	static void LumeShakeAtLocation(const UObject* WorldContextObject, ELumeShakePreset Preset, FVector Location, float InnerRadius = 300.f, float OuterRadius = 2500.f, float Scale = 1.f);

	/** Empurrão físico na câmera do jogador (cm/s, no mundo). */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "PlayerIndex", DisplayName = "LUME Camera Impulse"))
	static void LumeCameraImpulse(const UObject* WorldContextObject, FVector WorldImpulse, int32 PlayerIndex = 0);

	/** Tremor contínuo (0..1), ex.: terremoto. 0 para parar. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Shake", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "PlayerIndex", DisplayName = "LUME Set Continuous Shake"))
	static void LumeSetContinuousShake(const UObject* WorldContextObject, float Amount, int32 PlayerIndex = 0);

	/** Olhar em volta (-1..1). Ligue no analógico direito ou nas setas, a cada frame enquanto pressionado. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Look Around", meta = (WorldContext = "WorldContextObject", AdvancedDisplay = "PlayerIndex", DisplayName = "LUME Set Look Input"))
	static void LumeSetLookInput(const UObject* WorldContextObject, FVector2D Input, int32 PlayerIndex = 0);
};
