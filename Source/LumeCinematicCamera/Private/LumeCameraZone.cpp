#include "LumeCameraZone.h"
#include "LumeCameraMath.h"
#include "LumeCameraSubsystem.h"
#include "LumeCameraZoneProfile.h"
#include "LumeCinematicCameraActor.h"
#include "LumeCinematicCameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

ALumeCameraZone::ALumeCameraZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	ZoneBox->SetBoxExtent(FVector(400.0, 400.0, 200.0));
	ZoneBox->SetCollisionProfileName(TEXT("Trigger"));
	ZoneBox->SetGenerateOverlapEvents(true);
	ZoneBox->ShapeColor = FColor(255, 170, 40);
	ZoneBox->SetCanEverAffectNavigation(false);
	RootComponent = ZoneBox;

	FixedViewpoint = CreateDefaultSubobject<USceneComponent>(TEXT("FixedViewpoint"));
	FixedViewpoint->SetupAttachment(ZoneBox);
	FixedViewpoint->SetRelativeLocation(FVector(-500.0, 0.0, 300.0));
	FixedViewpoint->SetRelativeRotation(FRotator(-25.0, 0.0, 0.0));

#if WITH_EDITORONLY_DATA
	ViewpointArrow = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ViewpointArrow"));
	if (ViewpointArrow)
	{
		ViewpointArrow->SetupAttachment(FixedViewpoint);
		ViewpointArrow->ArrowColor = FColor(255, 60, 60);
		ViewpointArrow->ArrowSize = 1.5f;
		ViewpointArrow->bIsScreenSizeScaled = true;
	}
#endif
}

void ALumeCameraZone::BeginPlay()
{
	// Liga os eventos antes do BeginPlay para receber também as sobreposições iniciais.
	ZoneBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ALumeCameraZone::HandleBeginOverlap);
	ZoneBox->OnComponentEndOverlap.AddUniqueDynamic(this, &ALumeCameraZone::HandleEndOverlap);

	Super::BeginPlay();

	if (ULumeCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<ULumeCameraSubsystem>())
	{
		Subsystem->RegisterZone(this);
	}
}

void ALumeCameraZone::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (ULumeCameraSubsystem* Subsystem = World->GetSubsystem<ULumeCameraSubsystem>())
		{
			Subsystem->UnregisterZone(this);
		}
	}
	OverlappingPlayers.Reset();
	Super::EndPlay(EndPlayReason);
}

bool ALumeCameraZone::ContainsPoint(const FVector& Point) const
{
	return LumeMath::IsPointInsideBox(ZoneBox, Point);
}

bool ALumeCameraZone::AffectsCamera(const ULumeCinematicCameraComponent* Camera) const
{
	if (AffectedCameras.Num() == 0)
	{
		return true;
	}
	const AActor* CameraOwner = Camera ? Camera->GetOwner() : nullptr;
	for (const TObjectPtr<AActor>& Affected : AffectedCameras)
	{
		if (Affected && Affected == CameraOwner)
		{
			return true;
		}
	}
	return false;
}

FVector ALumeCameraZone::GetFixedViewpointLocation() const
{
	return FixedViewpoint ? FixedViewpoint->GetComponentLocation() : GetActorLocation();
}

const FLumeFramingOverrides& ALumeCameraZone::GetEffectiveOverrides() const
{
	return Profile ? Profile->Framing : Framing;
}

void ALumeCameraZone::ApplyPreset()
{
	if (Preset != ELumeZonePreset::Custom)
	{
		Modify();
		Framing = FLumeFramingOverrides::MakePreset(Preset);
	}
}

#if WITH_EDITOR
void ALumeCameraZone::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ALumeCameraZone, Preset))
	{
		ApplyPreset();
	}
}
#endif

bool ALumeCameraZone::IsPlayerActor(const AActor* Actor)
{
	const APawn* Pawn = Cast<APawn>(Actor);
	return Pawn && Pawn->IsPlayerControlled();
}

void ALumeCameraZone::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bEnabled || !IsPlayerActor(OtherActor))
	{
		return;
	}

	// Um personagem pode ter vários componentes sobrepondo: conta só a primeira entrada.
	const TWeakObjectPtr<AActor> Key(OtherActor);
	if (OverlappingPlayers.Contains(Key))
	{
		return;
	}
	OverlappingPlayers.Add(Key);

	if (CameraToActivate)
	{
		if (ULumeCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<ULumeCameraSubsystem>())
		{
			APlayerController* PC = Cast<APlayerController>(CastChecked<APawn>(OtherActor)->GetController());
			Subsystem->PushCamera(CameraToActivate.Get(), PC, CameraTransitionTime, CameraTransitionFunction, CameraTransitionExponent);
		}
	}

	OnPlayerEnterZone.Broadcast(OtherActor);
}

void ALumeCameraZone::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	const TWeakObjectPtr<AActor> Key(OtherActor);
	if (!OtherActor || !OverlappingPlayers.Contains(Key))
	{
		return;
	}
	// Ainda há outro componente do mesmo ator dentro da zona.
	if (ZoneBox->IsOverlappingActor(OtherActor))
	{
		return;
	}
	OverlappingPlayers.Remove(Key);

	if (CameraToActivate && bReturnToPreviousCameraOnExit)
	{
		if (ULumeCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<ULumeCameraSubsystem>())
		{
			const APawn* Pawn = Cast<APawn>(OtherActor);
			APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
			Subsystem->PopCamera(CameraToActivate.Get(), PC, CameraTransitionTime, CameraTransitionFunction, CameraTransitionExponent);
		}
	}

	OnPlayerExitZone.Broadcast(OtherActor);
}
