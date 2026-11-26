#include "LumeCameraSubsystem.h"
#include "LumeCinematicCamera.h"
#include "LumeCameraZone.h"
#include "LumeCameraBoundsVolume.h"
#include "LumeCinematicCameraActor.h"
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

void ULumeCameraSubsystem::RegisterCamera(ALumeCinematicCamera* Camera)
{
	if (Camera)
	{
		Cameras.AddUnique(Camera);
	}
}

void ULumeCameraSubsystem::UnregisterCamera(ALumeCinematicCamera* Camera)
{
	Cameras.Remove(Camera);
	for (TPair<TWeakObjectPtr<APlayerController>, FCameraStack>& Pair : Stacks)
	{
		Pair.Value.Cameras.Remove(Camera);
	}
}

TArray<ALumeCinematicCamera*> ULumeCameraSubsystem::GetAllCameras() const
{
	TArray<ALumeCinematicCamera*> Result;
	for (ALumeCinematicCamera* Camera : Cameras)
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

void ULumeCameraSubsystem::SetViewTarget(APlayerController* PC, ALumeCinematicCamera* Camera, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	if (!PC || !IsValid(Camera))
	{
		return;
	}
	// Impede que a possessão do pawn roube o view target.
	PC->bAutoManageActiveCameraTarget = false;
	PC->SetViewTargetWithBlend(Camera, FMath::Max(0.f, BlendTime), BlendFunction, BlendExponent);
	OnActiveCameraChanged.Broadcast(PC, Camera);

	UE_LOG(LogLumeCamera, Verbose, TEXT("LUME: '%s' ativa para '%s' (blend %.2fs)"), *Camera->GetName(), *PC->GetName(), BlendTime);
}

void ULumeCameraSubsystem::ActivateCamera(ALumeCinematicCamera* Camera, APlayerController* PlayerController, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC || !IsValid(Camera))
	{
		return;
	}
	FCameraStack& Stack = Stacks.FindOrAdd(PC);
	Stack.Cameras.Reset();
	Stack.Cameras.Add(Camera);
	SetViewTarget(PC, Camera, BlendTime, BlendFunction, BlendExponent);
}

void ULumeCameraSubsystem::PushCamera(ALumeCinematicCamera* Camera, APlayerController* PlayerController, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC || !IsValid(Camera))
	{
		return;
	}
	FCameraStack& Stack = Stacks.FindOrAdd(PC);
	Stack.Compact();
	if (Stack.Cameras.Num() > 0 && Stack.Cameras.Last().Get() == Camera)
	{
		return;
	}
	Stack.Cameras.Remove(Camera);
	Stack.Cameras.Add(Camera);
	SetViewTarget(PC, Camera, BlendTime, BlendFunction, BlendExponent);
}

void ULumeCameraSubsystem::PopCamera(ALumeCinematicCamera* Camera, APlayerController* PlayerController, float BlendTime,
	EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC || !Camera)
	{
		return;
	}
	FCameraStack* Stack = Stacks.Find(PC);
	if (!Stack)
	{
		return;
	}
	Stack->Compact();
	const bool bWasActive = Stack->Cameras.Num() > 0 && Stack->Cameras.Last().Get() == Camera;
	Stack->Cameras.Remove(Camera);

	if (bWasActive && Stack->Cameras.Num() > 0)
	{
		SetViewTarget(PC, Stack->Cameras.Last().Get(), BlendTime, BlendFunction, BlendExponent);
	}
}

ALumeCinematicCamera* ULumeCameraSubsystem::GetActiveCamera(APlayerController* PlayerController) const
{
	APlayerController* PC = ResolvePlayerController(PlayerController);
	if (!PC)
	{
		return nullptr;
	}
	if (const FCameraStack* Stack = Stacks.Find(PC))
	{
		for (int32 Index = Stack->Cameras.Num() - 1; Index >= 0; --Index)
		{
			if (ALumeCinematicCamera* Camera = Stack->Cameras[Index].Get())
			{
				return Camera;
			}
		}
	}
	return nullptr;
}

void ULumeCameraSubsystem::Deinitialize()
{
	Zones.Reset();
	BoundsVolumes.Reset();
	Cameras.Reset();
	Stacks.Reset();
	Super::Deinitialize();
}
