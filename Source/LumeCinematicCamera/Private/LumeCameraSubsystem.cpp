#include "LumeCameraSubsystem.h"
#include "LumeCinematicCamera.h"
#include "LumeCameraZone.h"
#include "LumeCameraBoundsVolume.h"
#include "LumeCinematicCameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

void ULumeCameraSubsystem::RegisterZone(ALumeCameraZone* Zone)
{
	if (Zone)
	{
		Zones.AddUnique(Zone);
	}
}

void ULumeCameraSubsystem::UnregisterZone(ALumeCameraZone* Zone)
{
	Zones.Remove(Zone);
}

void ULumeCameraSubsystem::RegisterBoundsVolume(ALumeCameraBoundsVolume* Volume)
{
	if (Volume)
	{
		BoundsVolumes.AddUnique(Volume);
	}
}

void ULumeCameraSubsystem::UnregisterBoundsVolume(ALumeCameraBoundsVolume* Volume)
{
	BoundsVolumes.Remove(Volume);
}

void ULumeCameraSubsystem::RegisterCamera(ULumeCinematicCameraComponent* Camera)
{
	if (Camera)
	{
		Cameras.AddUnique(Camera);
	}
}

void ULumeCameraSubsystem::UnregisterCamera(ULumeCinematicCameraComponent* Camera)
{
	Cameras.Remove(Camera);
}

TArray<ULumeCinematicCameraComponent*> ULumeCameraSubsystem::GetAllCameras() const
{
	TArray<ULumeCinematicCameraComponent*> Result;
	for (ULumeCinematicCameraComponent* Camera : Cameras)
	{
		if (IsValid(Camera))
		{
			Result.Add(Camera);
		}
	}
	return Result;
}

TArray<ALumeCameraZone*> ULumeCameraSubsystem::GetAllZones() const
{
	TArray<ALumeCameraZone*> Result;
	for (ALumeCameraZone* Zone : Zones)
	{
		if (IsValid(Zone))
		{
			Result.Add(Zone);
		}
	}
	return Result;
}

APlayerController* ULumeCameraSubsystem::ResolvePlayerController(APlayerController* PlayerController) const
{
	if (PlayerController)
	{
		return PlayerController;
	}
	const UWorld* World = GetWorld();
	return World ? World->GetFirstPlayerController() : nullptr;
}

void ULumeCameraSubsystem::SetViewTarget(APlayerController* PC, AActor* CameraActor, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	if (!PC || !IsValid(CameraActor))
	{
		return;
	}

	// Câmera no próprio personagem: deixa a Unreal gerenciar (respawn volta sozinho para o novo personagem).
	// Câmera solta no mundo: impede que a possessão do pawn roube a visão.
	PC->bAutoManageActiveCameraTarget = (CameraActor == PC->GetPawn());

	if (PC->GetViewTarget() != CameraActor)
	{
		PC->SetViewTargetWithBlend(CameraActor, FMath::Max(0.f, BlendTime), BlendFunction, BlendExponent);
	}
	OnActiveCameraChanged.Broadcast(PC, CameraActor);

	UE_LOG(LogLumeCamera, Verbose, TEXT("LUME: '%s' ativa para '%s' (blend %.2fs)"), *CameraActor->GetName(), *PC->GetName(), BlendTime);
}

void ULumeCameraSubsystem::ActivateCamera(AActor* CameraActor, APlayerController* PlayerController, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC || !IsValid(CameraActor))
	{
		return;
	}
	FCameraStack& Stack = Stacks.FindOrAdd(TWeakObjectPtr<APlayerController>(PC));
	Stack.Actors.Reset();
	Stack.Actors.Add(TWeakObjectPtr<AActor>(CameraActor));
	SetViewTarget(PC, CameraActor, BlendTime, BlendFunction, BlendExponent);
}

void ULumeCameraSubsystem::PushCamera(AActor* CameraActor, APlayerController* PlayerController, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC || !IsValid(CameraActor))
	{
		return;
	}
	FCameraStack& Stack = Stacks.FindOrAdd(TWeakObjectPtr<APlayerController>(PC));
	Stack.Compact();

	// Garante que a visão atual (ex.: o personagem) fique na base da pilha para o "voltar".
	if (Stack.Actors.Num() == 0)
	{
		if (AActor* Current = PC->GetViewTarget())
		{
			if (Current != CameraActor)
			{
				Stack.Actors.Add(TWeakObjectPtr<AActor>(Current));
			}
		}
	}

	if (Stack.Actors.Num() > 0 && Stack.Actors.Last().Get() == CameraActor)
	{
		return;
	}
	Stack.RemoveActor(CameraActor);
	Stack.Actors.Add(TWeakObjectPtr<AActor>(CameraActor));
	SetViewTarget(PC, CameraActor, BlendTime, BlendFunction, BlendExponent);
}

void ULumeCameraSubsystem::PopCamera(AActor* CameraActor, APlayerController* PlayerController, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC || !CameraActor)
	{
		return;
	}
	FCameraStack* Stack = Stacks.Find(TWeakObjectPtr<APlayerController>(PC));
	if (!Stack)
	{
		return;
	}
	Stack->Compact();
	const bool bWasActive = Stack->Actors.Num() > 0 && Stack->Actors.Last().Get() == CameraActor;
	Stack->RemoveActor(CameraActor);

	if (!bWasActive)
	{
		return;
	}

	if (Stack->Actors.Num() > 0)
	{
		SetViewTarget(PC, Stack->Actors.Last().Get(), BlendTime, BlendFunction, BlendExponent);
	}
	else if (APawn* Pawn = PC->GetPawn())
	{
		// Nada na pilha: volta para o personagem.
		Stack->Actors.Add(TWeakObjectPtr<AActor>(Pawn));
		SetViewTarget(PC, Pawn, BlendTime, BlendFunction, BlendExponent);
	}
}

AActor* ULumeCameraSubsystem::GetActiveCameraActor(APlayerController* PlayerController) const
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	return PC ? PC->GetViewTarget() : nullptr;
}

ULumeCinematicCameraComponent* ULumeCameraSubsystem::GetActiveCamera(APlayerController* PlayerController) const
{
	const AActor* ViewTarget = GetActiveCameraActor(PlayerController);
	return ViewTarget ? ViewTarget->FindComponentByClass<ULumeCinematicCameraComponent>() : nullptr;
}

void ULumeCameraSubsystem::Deinitialize()
{
	Zones.Reset();
	BoundsVolumes.Reset();
	Cameras.Reset();
	Stacks.Reset();
	Super::Deinitialize();
}
