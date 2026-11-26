#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LumeCameraBoundsVolume.generated.h"

class UBoxComponent;

/**
 * 4. Limites de movimentação.
 * Volume que limita onde a câmera pode ficar. Com vários volumes, a câmera pode
 * estar em qualquer um deles (união); fora de todos, é projetada no mais próximo.
 */
UCLASS(Blueprintable, ClassGroup = (LUME), meta = (DisplayName = "LUME Camera Bounds"))
class LUMECINEMATICCAMERA_API ALumeCameraBoundsVolume : public AActor
{
	GENERATED_BODY()

public:
	ALumeCameraBoundsVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LUME")
	TObjectPtr<UBoxComponent> Bounds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LUME")
	bool bEnabled = true;

	UFUNCTION(BlueprintPure, Category = "LUME|Bounds")
	bool ContainsPoint(const FVector& Point) const;

	UFUNCTION(BlueprintPure, Category = "LUME|Bounds")
	FVector ClampPoint(const FVector& Point, float Margin = 0.f) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
