#include "LumeCameraBoundsVolume.h"
#include "LumeCameraMath.h"
#include "LumeCameraSubsystem.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"

ALumeCameraBoundsVolume::ALumeCameraBoundsVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Bounds = CreateDefaultSubobject<UBoxComponent>(TEXT("Bounds"));
	Bounds->SetBoxExtent(FVector(1000.0, 1000.0, 500.0));
	Bounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bounds->SetGenerateOverlapEvents(false);
	Bounds->ShapeColor = FColor(80, 200, 255);
	Bounds->SetCanEverAffectNavigation(false);
	RootComponent = Bounds;
}

void ALumeCameraBoundsVolume::BeginPlay()
{
	Super::BeginPlay();
	if (ULumeCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<ULumeCameraSubsystem>())
	{
		Subsystem->RegisterBoundsVolume(this);
	}
}

void ALumeCameraBoundsVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (ULumeCameraSubsystem* Subsystem = World->GetSubsystem<ULumeCameraSubsystem>())
		{
			Subsystem->UnregisterBoundsVolume(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

bool ALumeCameraBoundsVolume::ContainsPoint(const FVector& Point) const
{
	return LumeMath::IsPointInsideBox(Bounds, Point);
}

FVector ALumeCameraBoundsVolume::ClampPoint(const FVector& Point, float Margin) const
{
	return LumeMath::ClampPointToBox(Bounds, Point, Margin);
}
