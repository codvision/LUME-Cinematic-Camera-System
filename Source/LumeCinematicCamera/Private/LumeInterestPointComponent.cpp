#include "LumeInterestPointComponent.h"
#include "LumeCameraSubsystem.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"

ULumeInterestPointComponent::ULumeInterestPointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULumeInterestPointComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ULumeCameraSubsystem* Subsystem = GetWorld()->GetSubsystem<ULumeCameraSubsystem>())
	{
		Subsystem->RegisterInterestPoint(this);
	}
}

void ULumeInterestPointComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (ULumeCameraSubsystem* Subsystem = World->GetSubsystem<ULumeCameraSubsystem>())
		{
			Subsystem->UnregisterInterestPoint(this);
		}
	}
	Super::EndPlay(EndPlayReason);
}

float ULumeInterestPointComponent::GetInfluenceAt(const FVector& PlayerLocation) const
{
	if (!bInterestEnabled || ActivationRadius <= 0.f)
	{
		return 0.f;
	}
	const float Distance = static_cast<float>(FVector::Dist(GetComponentLocation(), PlayerLocation));
	if (Distance >= ActivationRadius)
	{
		return 0.f;
	}
	const float Inner = FMath::Min(FullInfluenceRadius, ActivationRadius - 1.f);
	const float Falloff = 1.f - FMath::SmoothStep(Inner, ActivationRadius, Distance);
	return Falloff * Influence;
}

// ---------------------------------------------------------------------------

ALumeCameraInterestPoint::ALumeCameraInterestPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	InterestPoint = CreateDefaultSubobject<ULumeInterestPointComponent>(TEXT("InterestPoint"));
	RootComponent = InterestPoint;

#if WITH_EDITORONLY_DATA
	ActivationSphere = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("ActivationSphere"));
	if (ActivationSphere)
	{
		ActivationSphere->SetupAttachment(InterestPoint);
		ActivationSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		ActivationSphere->SetGenerateOverlapEvents(false);
		ActivationSphere->SetCanEverAffectNavigation(false);
		ActivationSphere->ShapeColor = FColor(120, 255, 120);
		ActivationSphere->SetHiddenInGame(true);
	}

	FullInfluenceSphere = CreateEditorOnlyDefaultSubobject<USphereComponent>(TEXT("FullInfluenceSphere"));
	if (FullInfluenceSphere)
	{
		FullInfluenceSphere->SetupAttachment(InterestPoint);
		FullInfluenceSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		FullInfluenceSphere->SetGenerateOverlapEvents(false);
		FullInfluenceSphere->SetCanEverAffectNavigation(false);
		FullInfluenceSphere->ShapeColor = FColor(255, 230, 60);
		FullInfluenceSphere->SetHiddenInGame(true);
	}
#endif
}

void ALumeCameraInterestPoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	if (InterestPoint)
	{
		if (ActivationSphere)
		{
			ActivationSphere->SetSphereRadius(InterestPoint->ActivationRadius);
		}
		if (FullInfluenceSphere)
		{
			FullInfluenceSphere->SetSphereRadius(FMath::Min(InterestPoint->FullInfluenceRadius, InterestPoint->ActivationRadius));
		}
	}
#endif
}
