#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "LumeCameraTypes.h"
#include "LumeCinematicCameraComponent.generated.h"

class ULumeCameraModule;
class ALumeCameraZone;
class ULumeCameraSubsystem;
class APlayerController;

/**
 * LUME Cinematic Camera (componente).
 *
 * Jeito recomendado: adicione este componente na Blueprint do personagem
 * (Add Component → "LUME Cinematic Camera"). Todas as configurações ficam no painel Details.
 *
 * Mesmo dentro do personagem, a câmera é independente: usa transform absoluto, então
 * cápsula, mesh, animações e root motion nunca a movem. Ela só observa o alvo e executa
 * uma lista de módulos (rastreamento, enquadramento, rotação, zoom, colisão, limites,
 * profundidade de campo, câmera na mão...).
 *
 * Também pode ser usada solta no mundo, através do ator "LUME Cinematic Camera".
 */
// Observação: não use HideCategories(CameraSettings/CameraOptions) aqui. A customização de
// detalhes da UCameraComponent no editor espera essas propriedades e trava (DetailGroup.cpp) sem elas.
// FOV e pós-processo são controlados pelo LUME a cada frame; bUsePawnControlRotation é forçado a false.
UCLASS(ClassGroup = (LUME), Blueprintable, meta = (BlueprintSpawnableComponent, DisplayName = "LUME Cinematic Camera"))
class LUMECINEMATICCAMERA_API ULumeCinematicCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	ULumeCinematicCameraComponent();

	// ---------- Alvo (referência do jogador configurável no editor) ----------

	/**
	 * Ator seguido pela câmera. Vazio + "Auto Target" = o próprio personagem
	 * (quando o componente está num Pawn) ou o pawn do jogador (quando está num ator do mundo).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LUME|Target")
	TObjectPtr<AActor> TargetActor;

	/** Escolhe o alvo automaticamente quando nenhum está definido (ou ele foi destruído). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Target")
	bool bAutoTarget = true;

	/** Jogador local usado quando o componente NÃO está num Pawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Target", meta = (ClampMin = "0"))
	int32 PlayerIndex = 0;

	/** Que ponto do alvo é rastreado. Actor Location é independente das animações. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Target")
	ELumeTrackingReference TrackingReference = ELumeTrackingReference::ActorLocation;

	/** Nome do componente usado como referência (Named Component / Socket). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Target", meta = (EditCondition = "TrackingReference != ELumeTrackingReference::ActorLocation"))
	FName ReferenceComponentName;

	/** Socket/bone usado como referência (modo Socket). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Target", meta = (EditCondition = "TrackingReference == ELumeTrackingReference::Socket"))
	FName ReferenceSocketName;

	/** Compensa a mudança de altura da cápsula ao agachar (evita a câmera "afundar"). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Target", meta = (EditCondition = "TrackingReference == ELumeTrackingReference::ActorLocation"))
	bool bCompensateCrouch = true;

	// ---------- Ativação ----------

	/** Assume a visão do jogador ao iniciar. Desligue em câmeras secundárias (ativadas por zonas). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Activation")
	bool bTakeOverPlayerView = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Activation", meta = (EditCondition = "bTakeOverPlayerView", ClampMin = "0", Units = "s"))
	float TakeOverBlendTime = 0.f;

	/** Desativa as outras câmeras do mesmo ator (ex.: a FollowCamera do template Third Person). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Activation")
	bool bDeactivateOtherCameras = true;

	/**
	 * No modo de rotação World Fixed, gira a Control Rotation junto com a câmera.
	 * Assim "frente" no controle é sempre "para dentro da tela", sem mexer na Blueprint do personagem.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Activation")
	bool bAlignControlsToCamera = true;

	// ---------- Enquadramento ----------

	/** Enquadramento base. Zonas e overrides são misturados sobre ele. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Framing", meta = (ShowOnlyInnerProperties))
	FLumeFramingSettings BaseFraming;

	/** Reage às LUME Camera Zones do nível. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Framing")
	bool bUseCameraZones = true;

	// ---------- Módulos ----------

	/** Pipeline de módulos, executados em ordem. Adicione, remova ou reordene livremente. */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "LUME|Modules")
	TArray<TObjectPtr<ULumeCameraModule>> Modules;

	// ---------- Depuração ----------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Debug")
	bool bDrawDebug = false;

	// ---------- API ----------

	/** Troca o alvo. bSnap = salta direto, sem transição. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera")
	void SetTarget(AActor* NewTarget, bool bSnap = false);

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	AActor* GetTarget() const { return TargetActor; }

	/** Remove toda a suavização no próximo frame (ex.: após teleporte ou corte). */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera")
	void SnapToTarget();

	/** Torna esta a câmera ativa do jogador, com transição suave. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Camera", meta = (AdvancedDisplay = "BlendFunction,BlendExponent"))
	void ActivateCamera(float BlendTime = 0.75f, EViewTargetBlendFunction BlendFunction = VTBlend_EaseInOut, float BlendExponent = 2.f);

	/**
	 * Aplica sobrescritas de enquadramento por script (cutscenes, eventos, sustos).
	 * Retorna um handle para remover com PopFramingOverride.
	 */
	UFUNCTION(BlueprintCallable, Category = "LUME|Framing")
	int32 PushFramingOverride(const FLumeFramingOverrides& Overrides, int32 Priority = 100, float BlendInTime = 1.f, float BlendOutTime = 1.f);

	UFUNCTION(BlueprintCallable, Category = "LUME|Framing")
	void PopFramingOverride(int32 Handle);

	UFUNCTION(BlueprintCallable, Category = "LUME|Framing")
	void ClearFramingOverrides();

	/** Peso atual (0..1) de uma zona nesta câmera. */
	UFUNCTION(BlueprintPure, Category = "LUME|Framing")
	float GetZoneWeight(const ALumeCameraZone* Zone) const;

	/** Enquadramento resultante neste frame (base + zonas + overrides). */
	UFUNCTION(BlueprintPure, Category = "LUME|Framing")
	FLumeFramingSettings GetCurrentFraming() const { return Context.Framing; }

	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	FLumeCameraState GetCameraState() const { return State; }

	/** Rotação só com o Yaw da câmera: útil para movimento relativo à câmera. */
	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	FRotator GetCameraYawRotation() const { return FRotator(0.0, State.CameraRotation.Yaw, 0.0); }

	/** Referência do alvo usada pelo rastreamento (antes do Pivot Offset). */
	UFUNCTION(BlueprintPure, Category = "LUME|Camera")
	FVector GetTrackingReferenceLocation() const;

	UFUNCTION(BlueprintPure, Category = "LUME|Modules", meta = (DeterminesOutputType = "ModuleClass"))
	ULumeCameraModule* FindModuleByClass(TSubclassOf<ULumeCameraModule> ModuleClass) const;

	/** Cria e adiciona um módulo em tempo de execução. Index < 0 = no final. */
	UFUNCTION(BlueprintCallable, Category = "LUME|Modules", meta = (DeterminesOutputType = "ModuleClass"))
	ULumeCameraModule* AddModule(TSubclassOf<ULumeCameraModule> ModuleClass, int32 Index = -1);

	template <typename T>
	T* FindModule() const
	{
		return Cast<T>(FindModuleByClass(T::StaticClass()));
	}

	/** Controller do jogador que esta câmera serve (o do personagem dono, ou o de PlayerIndex). */
	APlayerController* GetServedPlayerController() const;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateCamera(float DeltaSeconds);
	void UpdateOverrides(float DeltaSeconds, const FVector& ReferenceLocation, bool bSnap);
	void RunModules(ELumeModuleStage Stage);
	void ComposeCamera();
	void AlignControlRotation();
	void DrawDebugInfo() const;

	void ResolveTarget();
	void TryTakeOverView();
	void DeactivateOtherCameras();
	bool IsTargetAirborne() const;
	float GetViewAspectRatio() const;
	USceneComponent* FindReferenceComponent() const;
	ULumeCameraSubsystem* GetLumeSubsystem() const;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "LUME|Runtime")
	FLumeCameraState State;

	UPROPERTY(Transient)
	FLumeCameraContext Context;

private:
	struct FActiveOverride
	{
		TWeakObjectPtr<ALumeCameraZone> Zone;
		int32 Handle = INDEX_NONE;
		FLumeFramingOverrides Overrides;
		int32 Priority = 0;
		int32 Order = 0;
		float BlendInTime = 1.f;
		float BlendOutTime = 1.f;
		float Weight = 0.f;
		bool bWantsActive = false;
	};

	TArray<FActiveOverride> ActiveOverrides;
	int32 NextHandle = 1;
	int32 NextOrder = 0;
	bool bNeedsSnap = true;
	bool bTakeOverPending = false;
};
