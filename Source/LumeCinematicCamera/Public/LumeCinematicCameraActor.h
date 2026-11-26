#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/PlayerCameraManager.h"
#include "LumeCinematicCameraActor.generated.h"

class ULumeCinematicCameraComponent;

/**
 * LUME Cinematic Camera (ator do mundo).
 *
 * Um suporte para o componente LUME Cinematic Camera, para quando você quer uma câmera
 * solta no nível: câmeras secundárias ativadas por zonas, cutscenes, câmeras de segurança,
 * ou um jogo onde a câmera não pertence ao personagem.
 *
 * Para a câmera principal do jogador, o recomendado é adicionar o componente direto
 * na Blueprint do personagem.
 */
UCLASS(Blueprintable, ClassGroup = (LUME), meta = (DisplayName = "LUME Cinematic Camera"))
class LUMECINEMATICCAMERA_API ALumeCinematicCamera : public AActor
{
	GENERATED_BODY()

public:
	ALumeCinematicCamera();

	/** Toda a configuração da câmera fica aqui. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LUME")
	TObjectPtr<ULumeCinematicCameraComponent> LumeCamera;

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	ULumeCinematicCameraComponent* GetLumeCamera() const { return LumeCamera; }

	/** Torna esta a câmera ativa do jogador, com transição suave. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void ActivateCamera(float BlendTime = 0.75f, EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);
};
