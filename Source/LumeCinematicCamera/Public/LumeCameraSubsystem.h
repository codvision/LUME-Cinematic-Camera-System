#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Camera/PlayerCameraManager.h"
#include "LumeCameraSubsystem.generated.h"

class ALumeCameraZone;
class ALumeCameraBoundsVolume;
class ALumeCinematicCamera;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLumeActiveCameraChanged, APlayerController*, PlayerController, ALumeCinematicCamera*, NewCamera);

/**
 * Registro central do LUME no mundo: zonas, volumes de limite e câmeras,
 * além do controle de qual câmera está ativa (com transições suaves e pilha push/pop).
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
	void RegisterCamera(ALumeCinematicCamera* Camera);
	void UnregisterCamera(ALumeCinematicCamera* Camera);

	const TArray<TObjectPtr<ALumeCameraZone>>& GetZones() const { return Zones; }
	const TArray<TObjectPtr<ALumeCameraBoundsVolume>>& GetBoundsVolumes() const { return BoundsVolumes; }

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	TArray<ALumeCinematicCamera*> GetAllCameras() const;

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	TArray<ALumeCameraZone*> GetAllZones() const;

	// ---------- Câmera ativa ----------

	/** Ativa a câmera e reinicia a pilha. PlayerController vazio = primeiro jogador. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void ActivateCamera(ALumeCinematicCamera* Camera, APlayerController* PlayerController = nullptr, float BlendTime = 0.5f,
		EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	/** Empilha e ativa uma câmera temporária (ex.: ao entrar numa zona). */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void PushCamera(ALumeCinematicCamera* Camera, APlayerController* PlayerController = nullptr, float BlendTime = 0.5f,
		EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	/** Remove a câmera da pilha; se ela estava ativa, volta para a anterior. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void PopCamera(ALumeCinematicCamera* Camera, APlayerController* PlayerController = nullptr, float BlendTime = 0.5f,
		EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	ALumeCinematicCamera* GetActiveCamera(APlayerController* PlayerController = nullptr) const;

	UPROPERTY(BlueprintAssignable, Category = "LUME|Camera")
	FLumeActiveCameraChanged OnActiveCameraChanged;

	virtual void Deinitialize() override;

private:
	struct FCameraStack
	{
		TArray<TWeakObjectPtr<ALumeCinematicCamera>> Cameras;

		void Compact()
		{
			Cameras.RemoveAll([](const TWeakObjectPtr<ALumeCinematicCamera>& C) { return !C.IsValid(); });
		}
	};

	APlayerController* ResolvePlayerController(APlayerController* PlayerController) const;
	void SetViewTarget(APlayerController* PC, ALumeCinematicCamera* Camera, float BlendTime, EViewTargetBlendFunction BlendFunction, float BlendExponent);

	UPROPERTY(Transient)
	TArray<TObjectPtr<ALumeCameraZone>> Zones;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ALumeCameraBoundsVolume>> BoundsVolumes;

	UPROPERTY(Transient)
	TArray<TObjectPtr<ALumeCinematicCamera>> Cameras;

	TMap<TWeakObjectPtr<APlayerController>, FCameraStack> Stacks;
};
