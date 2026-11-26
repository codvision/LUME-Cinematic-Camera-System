#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/PlayerCameraManager.h"
#include "LumeCameraTypes.h"
#include "LumeCameraZone.generated.h"

class UBoxComponent;
class UArrowComponent;
class UPrimitiveComponent;
class ULumeCameraZoneProfile;
class ALumeCinematicCamera;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLumeZonePlayerEvent, AActor*, PlayerActor);

/**
 * 3. Zona cinematográfica.
 * Cada ambiente pode ter seu próprio enquadramento. Quando o alvo da câmera entra
 * na caixa, as sobrescritas são misturadas suavemente (Blend In) e, ao sair,
 * desfeitas suavemente (Blend Out). Zonas sobrepostas são combinadas por prioridade.
 *
 * Opcional:
 * - Ponto de vista fixo (enquadramento especial, ex.: câmera de segurança / terror).
 * - Restringir a câmera ao volume da zona.
 * - Trocar para outra LUME Camera com transição suave.
 */
UCLASS(Blueprintable, ClassGroup = (LUME), meta = (DisplayName = "LUME Camera Zone"))
class LUMECINEMATICCAMERA_API ALumeCameraZone : public AActor
{
	GENERATED_BODY()

public:
	ALumeCameraZone();

	// ---------- Componentes ----------

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LUME|Components")
	TObjectPtr<UBoxComponent> ZoneBox;

	/** Posição usada quando "Use Fixed Viewpoint" está ligado. Mova-a no viewport. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LUME|Components")
	TObjectPtr<USceneComponent> FixedViewpoint;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	TObjectPtr<UArrowComponent> ViewpointArrow;
#endif

	// ---------- Zona ----------

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Zone")
	bool bEnabled = true;

	/** Zonas com prioridade maior são aplicadas por cima das menores. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Zone")
	int32 Priority = 0;

	/** Tempo de transição ao entrar (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Zone", meta = (ClampMin = "0", Units = "s"))
	float BlendInTime = 1.0f;

	/** Tempo de transição ao sair (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Zone", meta = (ClampMin = "0", Units = "s"))
	float BlendOutTime = 1.25f;

	/** Câmeras afetadas. Vazio = todas as LUME Cameras. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Zone")
	TArray<TObjectPtr<ALumeCinematicCamera>> AffectedCameras;

	// ---------- Enquadramento ----------

	/** Mudar o preset preenche "Framing" (Corredor, Sala Grande, Terror). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Framing", meta = (EditCondition = "Profile == nullptr"))
	ELumeZonePreset Preset = ELumeZonePreset::Custom;

	/** Perfil reutilizável. Se definido, substitui "Framing". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Framing")
	TObjectPtr<ULumeCameraZoneProfile> Profile;

	/** Sobrescritas de enquadramento desta zona. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Framing", meta = (EditCondition = "Profile == nullptr"))
	FLumeFramingOverrides Framing;

	// ---------- Enquadramentos especiais ----------

	/** A câmera vai para o ponto "FixedViewpoint" e gira para acompanhar o jogador. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Special")
	bool bUseFixedViewpoint = false;

	/** A câmera não sai do volume desta zona enquanto ela estiver ativa. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Special")
	bool bConstrainCameraToZone = false;

	// ---------- Troca de câmera ----------

	/** Ao entrar, ativa esta câmera com transição suave. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Camera Switch")
	TObjectPtr<ALumeCinematicCamera> CameraToActivate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Camera Switch", meta = (EditCondition = "CameraToActivate != nullptr", ClampMin = "0", Units = "s"))
	float CameraTransitionTime = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Camera Switch", meta = (EditCondition = "CameraToActivate != nullptr"))
	TEnumAsByte<EViewTargetBlendFunction> CameraTransitionFunction = VTBlend_EaseInOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Camera Switch", meta = (EditCondition = "CameraToActivate != nullptr", ClampMin = "0"))
	float CameraTransitionExponent = 2.f;

	/** Ao sair, volta para a câmera anterior. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME|Camera Switch", meta = (EditCondition = "CameraToActivate != nullptr"))
	bool bReturnToPreviousCameraOnExit = true;

	// ---------- Eventos ----------

	UPROPERTY(BlueprintAssignable, Category = "LUME|Events")
	FLumeZonePlayerEvent OnPlayerEnterZone;

	UPROPERTY(BlueprintAssignable, Category = "LUME|Events")
	FLumeZonePlayerEvent OnPlayerExitZone;

	// ---------- API ----------

	UFUNCTION(BlueprintPure, Category = "LUME|Zone")
	bool ContainsPoint(const FVector& Point) const;

	UFUNCTION(BlueprintPure, Category = "LUME|Zone")
	bool AffectsCamera(const ALumeCinematicCamera* Camera) const;

	UFUNCTION(BlueprintPure, Category = "LUME|Zone")
	FVector GetFixedViewpointLocation() const;

	UFUNCTION(BlueprintPure, Category = "LUME|Zone")
	UBoxComponent* GetZoneBox() const { return ZoneBox; }

	/** Sobrescritas efetivas (perfil, se houver; senão as da própria zona). */
	const FLumeFramingOverrides& GetEffectiveOverrides() const;

	/** Aplica o preset selecionado a "Framing". */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "LUME|Framing")
	void ApplyPreset();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	static bool IsPlayerActor(const AActor* Actor);

	TSet<TWeakObjectPtr<AActor>> OverlappingPlayers;
};
