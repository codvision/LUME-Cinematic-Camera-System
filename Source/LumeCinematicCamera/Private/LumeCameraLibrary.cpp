#include "LumeCameraLibrary.h"
#include "LumeCameraSubsystem.h"
#include "LumeCinematicCameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

ULumeCinematicCameraComponent* ULumeCameraLibrary::GetActiveLumeCamera(const UObject* WorldContextObject, int32 PlayerIndex)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(World, PlayerIndex);
	if (const ULumeCameraSubsystem* Subsystem = World->GetSubsystem<ULumeCameraSubsystem>())
	{
		if (ULumeCinematicCameraComponent* Active = Subsystem->GetActiveCamera(PC))
		{
			return Active;
		}
	}

	// Sem view target LUME: tenta a câmera do personagem do jogador.
	if (const APawn* Pawn = PC ? PC->GetPawn() : nullptr)
	{
		return Pawn->FindComponentByClass<ULumeCinematicCameraComponent>();
	}
	return nullptr;
}

void ULumeCameraLibrary::LumeShakeCamera(const UObject* WorldContextObject, ELumeShakePreset Preset, float Scale, FVector Direction, int32 PlayerIndex)
{
	if (ULumeCinematicCameraComponent* Camera = GetActiveLumeCamera(WorldContextObject, PlayerIndex))
	{
		Camera->PlayShake(Preset, Scale, Direction);
	}
}

void ULumeCameraLibrary::LumeShakeAtLocation(const UObject* WorldContextObject, ELumeShakePreset Preset, FVector Location, float InnerRadius, float OuterRadius, float Scale)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	const ULumeCameraSubsystem* Subsystem = World ? World->GetSubsystem<ULumeCameraSubsystem>() : nullptr;
	if (!Subsystem)
	{
		return;
	}

	const double Inner = FMath::Max(0.f, InnerRadius);
	const double Outer = FMath::Max(Inner + 1.0, static_cast<double>(OuterRadius));

	for (ULumeCinematicCameraComponent* Camera : Subsystem->GetAllCameras())
	{
		const FVector Listener = Camera->GetTrackingReferenceLocation();
		const double Distance = FVector::Dist(Listener, Location);
		if (Distance >= Outer)
		{
			continue;
		}
		const double Falloff = Distance <= Inner ? 1.0 : 1.0 - (Distance - Inner) / (Outer - Inner);
		const FVector Away = (Listener - Location).GetSafeNormal();
		Camera->PlayShake(Preset, Scale * static_cast<float>(Falloff), Away.IsNearlyZero() ? FVector::UpVector : Away);
	}
}

void ULumeCameraLibrary::LumeCameraImpulse(const UObject* WorldContextObject, FVector WorldImpulse, int32 PlayerIndex)
{
	if (ULumeCinematicCameraComponent* Camera = GetActiveLumeCamera(WorldContextObject, PlayerIndex))
	{
		Camera->AddCameraImpulse(WorldImpulse);
	}
}

void ULumeCameraLibrary::LumeSetContinuousShake(const UObject* WorldContextObject, float Amount, int32 PlayerIndex)
{
	if (ULumeCinematicCameraComponent* Camera = GetActiveLumeCamera(WorldContextObject, PlayerIndex))
	{
		Camera->SetContinuousShake(Amount);
	}
}

void ULumeCameraLibrary::LumeSetLookInput(const UObject* WorldContextObject, FVector2D Input, int32 PlayerIndex)
{
	if (ULumeCinematicCameraComponent* Camera = GetActiveLumeCamera(WorldContextObject, PlayerIndex))
	{
		Camera->SetLookInput(Input);
	}
}
