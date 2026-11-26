#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LumeCameraTypes.h"
#include "LumeCameraZoneProfile.generated.h"

/**
 * Perfil de enquadramento reutilizável (Data Asset).
 * Ex.: DA_Corredor, DA_SalaGrande, DA_Terror. Várias zonas podem usar o mesmo perfil.
 */
UCLASS(BlueprintType, meta = (DisplayName = "LUME Camera Zone Profile"))
class LUMECINEMATICCAMERA_API ULumeCameraZoneProfile : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Mudar o preset preenche as sobrescritas abaixo (depois você pode ajustá-las). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LUME")
	ELumeZonePreset Preset = ELumeZonePreset::Custom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LUME")
	FLumeFramingOverrides Framing;

	/** Aplica o preset selecionado às sobrescritas. */
	UFUNCTION(CallInEditor, BlueprintCallable, Category = "LUME")
	void ApplyPreset();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
