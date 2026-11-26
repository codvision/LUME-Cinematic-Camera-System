#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "LumeCameraSubsystem.generated.h"

class ALumeCameraZone;
class ALumeCameraBoundsVolume;
class ULumeCinematicCameraComponent;
class ULumeInterestPointComponent;
class APlayerController;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLumeActiveCameraChanged, APlayerController*, PlayerController, AActor*, NewCameraActor);

/**
 * Registro central do LUME no mundo: zonas, volumes de limite e câmeras,
 * além do controle de qual câmera está ativa (com transições suaves e pilha push/pop).
 *
 * "Camera Actor" pode ser um ator LUME Cinematic Camera ou um personagem que tem o
 * componente LUME Cinematic Camera (qualquer ator com câmera funciona).
 */
UCLASS()
class LUMECINEMATICCAMERA_API ULumeCameraSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// ---------- Registro ----------

	void RegisterZone(ALumeCameraZone* Zone);
	void UnregisterZone(ALumeCameraZone* Zone);
	void RegisterBoundsVolume(ALumeCameraBoundsVolume* Volume);
	void UnregisterBoundsVolume(ALumeCameraBoundsVolume* Volume);
	void RegisterCamera(ULumeCinematicCameraComponent* Camera);
	void UnregisterCamera(ULumeCinematicCameraComponent* Camera);

	void RegisterInterestPoint(ULumeInterestPointComponent* Point);
	void UnregisterInterestPoint(ULumeInterestPointComponent* Point);

	const TArray<TObjectPtr<ALumeCameraZone>>& GetZones() const { return Zones; }
	const TArray<TObjectPtr<ULumeInterestPointComponent>>& GetInterestPoints() const { return InterestPoints; }
	const TArray<TObjectPtr<ALumeCameraBoundsVolume>>& GetBoundsVolumes() const { return BoundsVolumes; }

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	TArray<ULumeCinematicCameraComponent*> GetAllCameras() const;

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	TArray<ALumeCameraZone*> GetAllZones() const;

	// ---------- Câmera ativa ----------

	/** Ativa a câmera e reinicia a pilha. PlayerController vazio = primeiro jogador. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void ActivateCamera(AActor* CameraActor, APlayerController* PlayerController = nullptr, float BlendTime = 0.5f,
		EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	/** Empilha e ativa uma câmera temporária (ex.: ao entrar numa zona). */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void PushCamera(AActor* CameraActor, APlayerController* PlayerController = nullptr, float BlendTime = 0.5f,
		EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	/** Remove a câmera da pilha; se ela estava ativa, volta para a anterior (ou para o personagem). */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void PopCamera(AActor* CameraActor, APlayerController* PlayerController = nullptr, float BlendTime = 0.5f,
		EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	/** Ator de câmera ativo do jogador. */
	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	AActor* GetActiveCameraActor(APlayerController* PlayerController = nullptr) const;

	/** Componente LUME da câmera ativa (nulo se a câmera ativa não for LUME). */
	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	ULumeCinematicCameraComponent* GetActiveCamera(APlayerController* PlayerController = nullptr) const;

	UPROPERTY(BlueprintAssignable, Category = "LUME|Camera")
	FLumeActiveCameraChanged OnActiveCameraChanged;

	virtual void Deinitialize() override;

private:
	struct FCameraStack
	{
		TArray<TWeakObjectPtr<AActor>> Actors;

		void Compact()
		{
			Actors.RemoveAll([](const TWeakObjectPtr<AActor>& A) { return !A.IsValid(); });
		}
		void RemoveActor(const AActor* Actor)
		{
			Actors.RemoveAll([Actor](const TWeakObjectPtr<AActor>& A) { return A.Get() == Actor; });
		}
	};

	APlayerController* ResolvePlayerController(APlayerController* PlayerController) const;
	void SetViewTarget(APlayerController* PC, AActor* CameraActor, float BlendTime, EViewTargetBlendFunction BlendFunction, float BlendExponent);

	UPROPERTY(Transient)
	TArray<TObjectPtr<ALumeCameraZone>> Zones;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ALumeCameraBoundsVolume>> BoundsVolumes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ULumeCinematicCameraComponent>> Cameras;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ULumeInterestPointComponent>> InterestPoints;

	TMap<TWeakObjectPtr<APlayerController>, FCameraStack> Stacks;
};
