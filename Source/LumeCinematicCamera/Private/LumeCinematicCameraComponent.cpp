#include "LumeCinematicCameraComponent.h"
#include "LumeCinematicCamera.h"
#include "LumeCameraSubsystem.h"
#include "LumeCameraZone.h"
#include "Modules/LumeCameraModule.h"
#include "Modules/LumeTrackingModule.h"
#include "Modules/LumeFramingModule.h"
#include "Modules/LumeRotationModule.h"
#include "Modules/LumeDynamicZoomModule.h"
#include "Modules/LumeCollisionModule.h"
#include "Modules/LumeBoundsModule.h"
#include "Modules/LumeDepthOfFieldModule.h"
#include "Modules/LumeHandheldNoiseModule.h"
#include "Modules/LumeInterestModule.h"
#include "Modules/LumeShakeModule.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ULumeCinematicCameraComponent::ULumeCinematicCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	// Depois da física/movimento do personagem: a câmera sempre vê a posição final do frame.
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// Independência total do dono: cápsula, mesh e animações não movem a câmera.
	SetUsingAbsoluteLocation(true);
	SetUsingAbsoluteRotation(true);
	SetUsingAbsoluteScale(true);

	bUsePawnControlRotation = false;
	bConstrainAspectRatio = false;
	SetFieldOfView(BaseFraming.FieldOfView);

	// Pipeline padrão (a ordem importa).
	Modules.Add(CreateDefaultSubobject<ULumeTrackingModule>(TEXT("TrackingModule")));
	Modules.Add(CreateDefaultSubobject<ULumeFramingModule>(TEXT("FramingModule")));
	Modules.Add(CreateDefaultSubobject<ULumeInterestModule>(TEXT("InterestModule")));
	Modules.Add(CreateDefaultSubobject<ULumeRotationModule>(TEXT("RotationModule")));
	Modules.Add(CreateDefaultSubobject<ULumeDynamicZoomModule>(TEXT("DynamicZoomModule")));
	Modules.Add(CreateDefaultSubobject<ULumeCollisionModule>(TEXT("CollisionModule")));
	Modules.Add(CreateDefaultSubobject<ULumeBoundsModule>(TEXT("BoundsModule")));
	Modules.Add(CreateDefaultSubobject<ULumeDepthOfFieldModule>(TEXT("DepthOfFieldModule")));
	Modules.Add(CreateDefaultSubobject<ULumeHandheldNoiseModule>(TEXT("HandheldNoiseModule")));
	Modules.Add(CreateDefaultSubobject<ULumeShakeModule>(TEXT("ShakeModule")));
}

void ULumeCinematicCameraComponent::EnsureBuiltInModules()
{
	// Ordem canônica do pipeline.
	const UClass* Order[] =
	{
		ULumeTrackingModule::StaticClass(),
		ULumeFramingModule::StaticClass(),
		ULumeInterestModule::StaticClass(),
		ULumeRotationModule::StaticClass(),
		ULumeDynamicZoomModule::StaticClass(),
		ULumeCollisionModule::StaticClass(),
		ULumeBoundsModule::StaticClass(),
		ULumeDepthOfFieldModule::StaticClass(),
		ULumeHandheldNoiseModule::StaticClass(),
		ULumeShakeModule::StaticClass(),
	};

	int32 InsertAt = 0;
	for (const UClass* ModuleClass : Order)
	{
		const int32 Found = Modules.IndexOfByPredicate([ModuleClass](const TObjectPtr<ULumeCameraModule>& Module)
		{
			return Module && Module->IsA(ModuleClass);
		});

		if (Found != INDEX_NONE)
		{
			InsertAt = Found + 1;
		}
		else
		{
			AddModule(const_cast<UClass*>(ModuleClass), InsertAt);
			++InsertAt;
		}
	}
}

void ULumeCinematicCameraComponent::PlayShake(ELumeShakePreset Preset, float Scale, FVector Direction)
{
	if (ULumeShakeModule* Shake = FindOrAddModule<ULumeShakeModule>())
	{
		Shake->PlayShake(Preset, Scale, Direction);
	}
}

void ULumeCinematicCameraComponent::AddTrauma(float Amount)
{
	if (ULumeShakeModule* Shake = FindOrAddModule<ULumeShakeModule>())
	{
		Shake->AddTrauma(Amount);
	}
}

void ULumeCinematicCameraComponent::AddCameraImpulse(FVector WorldImpulse)
{
	if (ULumeShakeModule* Shake = FindOrAddModule<ULumeShakeModule>())
	{
		Shake->AddImpulse(WorldImpulse);
	}
}

void ULumeCinematicCameraComponent::SetContinuousShake(float Amount)
{
	if (ULumeShakeModule* Shake = FindOrAddModule<ULumeShakeModule>())
	{
		Shake->SetContinuousShake(Amount);
	}
}

void ULumeCinematicCameraComponent::SetLookInput(FVector2D Input)
{
	if (ULumeInterestModule* Interest = FindOrAddModule<ULumeInterestModule>())
	{
		Interest->SetLookInput(Input);
	}
}

ULumeCameraSubsystem* ULumeCinematicCameraComponent::GetLumeSubsystem() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<ULumeCameraSubsystem>() : nullptr;
}

void ULumeCinematicCameraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->RegisterCamera(this);
	}

	if (bDeactivateOtherCameras)
	{
		DeactivateOtherCameras();
	}

	if (bAutoAddMissingModules)
	{
		EnsureBuiltInModules();
	}

	// A rotação vem do LUME, nunca da Control Rotation do pawn (mesmo que marcado no Details).
	bUsePawnControlRotation = false;

	if (IsValid(TargetActor))
	{
		SetTarget(TargetActor, true);
	}
	ResolveTarget();

	// Posiciona a câmera já no primeiro frame (evita um frame vindo da origem do mundo).
	bNeedsSnap = true;
	if (IsValid(TargetActor))
	{
		UpdateCamera(0.f);
		bNeedsSnap = true;
	}

	bTakeOverPending = bTakeOverPlayerView;
	TryTakeOverView();
}

void ULumeCinematicCameraComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->UnregisterCamera(this);
	}
	Super::EndPlay(EndPlayReason);
}

void ULumeCinematicCameraComponent::DeactivateOtherCameras()
{
	if (AActor* Owner = GetOwner())
	{
		TInlineComponentArray<UCameraComponent*> Cameras(Owner);
		for (UCameraComponent* Camera : Cameras)
		{
			if (Camera && Camera != this)
			{
				Camera->Deactivate();
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Alvo / ativação
// ---------------------------------------------------------------------------

void ULumeCinematicCameraComponent::SetTarget(AActor* NewTarget, bool bSnap)
{
	AActor* OldTarget = TargetActor;
	if (IsValid(OldTarget) && OldTarget != NewTarget && OldTarget != GetOwner())
	{
		RemoveTickPrerequisiteActor(OldTarget);
	}

	TargetActor = NewTarget;

	if (IsValid(NewTarget) && NewTarget != GetOwner())
	{
		AddTickPrerequisiteActor(NewTarget);
	}

	if (bSnap)
	{
		bNeedsSnap = true;
	}
}

void ULumeCinematicCameraComponent::SnapToTarget()
{
	bNeedsSnap = true;
}

void ULumeCinematicCameraComponent::ResolveTarget()
{
	if (TargetActor && !IsValid(TargetActor))
	{
		TargetActor = nullptr;
	}

	if (!TargetActor && bAutoTarget)
	{
		AActor* Owner = GetOwner();
		if (APawn* OwnerPawn = Cast<APawn>(Owner))
		{
			SetTarget(OwnerPawn, true);
		}
		else if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, PlayerIndex))
		{
			SetTarget(PlayerPawn, true);
		}
	}
}

APlayerController* ULumeCinematicCameraComponent::GetServedPlayerController() const
{
	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		return Cast<APlayerController>(OwnerPawn->GetController());
	}
	return UGameplayStatics::GetPlayerController(this, PlayerIndex);
}

void ULumeCinematicCameraComponent::TryTakeOverView()
{
	if (!bTakeOverPending)
	{
		return;
	}

	APlayerController* PC = GetServedPlayerController();
	if (!PC)
	{
		// Personagem ainda não possuído (ou é uma IA): tenta de novo no próximo frame.
		return;
	}
	if (!PC->IsLocalController())
	{
		bTakeOverPending = false;
		return;
	}
	// Câmera no mundo: espera o pawn do jogador existir, senão a possessão roubaria a visão.
	if (!Cast<APawn>(GetOwner()) && bAutoTarget && !TargetActor && !PC->GetPawn())
	{
		return;
	}

	bTakeOverPending = false;
	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->ActivateCamera(GetOwner(), PC, TakeOverBlendTime, VTBlend_EaseInOut, 2.f);
	}
}

void ULumeCinematicCameraComponent::ActivateCamera(float BlendTime, EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	bTakeOverPending = false;
	Activate();
	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->ActivateCamera(GetOwner(), GetServedPlayerController(), BlendTime, BlendFunction, BlendExponent);
	}
}

USceneComponent* ULumeCinematicCameraComponent::FindReferenceComponent() const
{
	const AActor* Target = TargetActor;
	if (!IsValid(Target))
	{
		return nullptr;
	}

	if (!ReferenceComponentName.IsNone())
	{
		TInlineComponentArray<USceneComponent*> Components(Target);
		for (USceneComponent* Component : Components)
		{
			if (Component && Component->GetFName() == ReferenceComponentName)
			{
				return Component;
			}
		}
	}

	if (TrackingReference == ELumeTrackingReference::Socket)
	{
		if (USkeletalMeshComponent* Mesh = Target->FindComponentByClass<USkeletalMeshComponent>())
		{
			return Mesh;
		}
	}

	return Target->GetRootComponent();
}

FVector ULumeCinematicCameraComponent::GetTrackingReferenceLocation() const
{
	const AActor* Target = TargetActor;
	if (!IsValid(Target))
	{
		return GetComponentLocation();
	}

	if (TrackingReference != ELumeTrackingReference::ActorLocation)
	{
		if (const USceneComponent* Component = FindReferenceComponent())
		{
			if (TrackingReference == ELumeTrackingReference::Socket && !ReferenceSocketName.IsNone())
			{
				return Component->GetSocketLocation(ReferenceSocketName);
			}
			return Component->GetComponentLocation();
		}
	}

	FVector Location = Target->GetActorLocation();

	// Agachar reduz a cápsula e desloca o ator para baixo: compensa para a câmera não "afundar".
	if (bCompensateCrouch)
	{
		if (const ACharacter* Character = Cast<ACharacter>(Target))
		{
			const UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
			const ACharacter* Defaults = Character->GetClass()->GetDefaultObject<ACharacter>();
			const UCapsuleComponent* DefaultCapsule = Defaults ? Defaults->GetCapsuleComponent() : nullptr;
			if (Capsule && DefaultCapsule)
			{
				const double StandingHalfHeight = DefaultCapsule->GetUnscaledCapsuleHalfHeight() * Capsule->GetShapeScale();
				Location.Z += StandingHalfHeight - Capsule->GetScaledCapsuleHalfHeight();
			}
		}
	}

	return Location;
}

bool ULumeCinematicCameraComponent::IsTargetAirborne() const
{
	if (const ACharacter* Character = Cast<ACharacter>(TargetActor))
	{
		if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
		{
			return Movement->IsFalling();
		}
	}
	return false;
}

float ULumeCinematicCameraComponent::GetViewAspectRatio() const
{
	if (const APlayerController* PC = GetServedPlayerController())
	{
		int32 SizeX = 0;
		int32 SizeY = 0;
		PC->GetViewportSize(SizeX, SizeY);
		if (SizeX > 0 && SizeY > 0)
		{
			return static_cast<float>(SizeX) / static_cast<float>(SizeY);
		}
	}
	return AspectRatio > 0.f ? AspectRatio : 16.f / 9.f;
}

// ---------------------------------------------------------------------------
// Overrides / zonas
// ---------------------------------------------------------------------------

int32 ULumeCinematicCameraComponent::PushFramingOverride(const FLumeFramingOverrides& Overrides, int32 Priority, float BlendInTime, float BlendOutTime)
{
	FActiveOverride& Entry = ActiveOverrides.AddDefaulted_GetRef();
	Entry.Handle = NextHandle++;
	Entry.Overrides = Overrides;
	Entry.Priority = Priority;
	Entry.Order = NextOrder++;
	Entry.BlendInTime = BlendInTime;
	Entry.BlendOutTime = BlendOutTime;
	Entry.bWantsActive = true;
	return Entry.Handle;
}

void ULumeCinematicCameraComponent::PopFramingOverride(int32 Handle)
{
	for (FActiveOverride& Entry : ActiveOverrides)
	{
		if (Entry.Handle == Handle && Handle != INDEX_NONE)
		{
			Entry.bWantsActive = false;
		}
	}
}

void ULumeCinematicCameraComponent::ClearFramingOverrides()
{
	for (FActiveOverride& Entry : ActiveOverrides)
	{
		if (Entry.Handle != INDEX_NONE)
		{
			Entry.bWantsActive = false;
		}
	}
}

float ULumeCinematicCameraComponent::GetZoneWeight(const ALumeCameraZone* Zone) const
{
	for (const FActiveOverride& Entry : ActiveOverrides)
	{
		if (Zone && Entry.Zone.Get() == Zone)
		{
			return FMath::SmoothStep(0.f, 1.f, Entry.Weight);
		}
	}
	return 0.f;
}

void ULumeCinematicCameraComponent::UpdateOverrides(float DeltaSeconds, const FVector& ReferenceLocation, bool bSnap)
{
	// --- Detecta em quais zonas o alvo está ---
	if (bUseCameraZones)
	{
		if (const ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
		{
			for (ALumeCameraZone* Zone : Subsystem->GetZones())
			{
				if (!IsValid(Zone) || !Zone->AffectsCamera(this))
				{
					continue;
				}

				const bool bInside = Zone->bEnabled && Zone->ContainsPoint(ReferenceLocation);

				FActiveOverride* Entry = ActiveOverrides.FindByPredicate([Zone](const FActiveOverride& E)
				{
					return E.Handle == INDEX_NONE && E.Zone.Get() == Zone;
				});

				if (!Entry)
				{
					if (!bInside)
					{
						continue;
					}
					Entry = &ActiveOverrides.AddDefaulted_GetRef();
					Entry->Zone = Zone;
					Entry->Order = NextOrder++;
				}

				// Atualizado a cada frame: editar a zona em PIE tem efeito imediato.
				Entry->bWantsActive = bInside;
				Entry->Overrides = Zone->GetEffectiveOverrides();
				Entry->Priority = Zone->Priority;
				Entry->BlendInTime = Zone->BlendInTime;
				Entry->BlendOutTime = Zone->BlendOutTime;
			}
		}
	}

	// --- Atualiza pesos ---
	for (FActiveOverride& Entry : ActiveOverrides)
	{
		const bool bIsZone = Entry.Handle == INDEX_NONE;
		if (bIsZone && (!bUseCameraZones || !Entry.Zone.IsValid()))
		{
			Entry.bWantsActive = false;
		}

		const float TargetWeight = Entry.bWantsActive ? 1.f : 0.f;
		const float BlendTime = Entry.bWantsActive ? Entry.BlendInTime : Entry.BlendOutTime;

		if (bSnap || BlendTime <= UE_KINDA_SMALL_NUMBER)
		{
			Entry.Weight = TargetWeight;
		}
		else
		{
			Entry.Weight = FMath::FInterpConstantTo(Entry.Weight, TargetWeight, DeltaSeconds, 1.f / BlendTime);
		}
	}

	ActiveOverrides.RemoveAll([](const FActiveOverride& E) { return !E.bWantsActive && E.Weight <= 0.f; });

	ActiveOverrides.Sort([](const FActiveOverride& A, const FActiveOverride& B)
	{
		return A.Priority != B.Priority ? A.Priority < B.Priority : A.Order < B.Order;
	});

	// --- Mistura: base -> zonas/overrides por prioridade ---
	Context.Framing = BaseFraming;
	Context.FixedViewpointAlpha = 0.f;
	Context.FixedViewpointLocation = FVector::ZeroVector;
	Context.ZoneConstraints.Reset();

	for (const FActiveOverride& Entry : ActiveOverrides)
	{
		const float Alpha = FMath::SmoothStep(0.f, 1.f, Entry.Weight);
		if (Alpha <= 0.f)
		{
			continue;
		}

		Entry.Overrides.ApplyTo(Context.Framing, Alpha);

		if (const ALumeCameraZone* Zone = Entry.Zone.Get())
		{
			if (Zone->bUseFixedViewpoint)
			{
				const FVector ZoneViewpoint = Zone->GetFixedViewpointLocation();
				Context.FixedViewpointLocation = Context.FixedViewpointAlpha <= 0.f
					? ZoneViewpoint
					: FMath::Lerp(Context.FixedViewpointLocation, ZoneViewpoint, static_cast<double>(Alpha));
				Context.FixedViewpointAlpha = FMath::Lerp(Context.FixedViewpointAlpha, 1.f, Alpha);
			}
			if (Zone->bConstrainCameraToZone)
			{
				FLumeZoneConstraint& Constraint = Context.ZoneConstraints.AddDefaulted_GetRef();
				Constraint.Box = Zone->GetZoneBox();
				Constraint.Weight = Alpha;
			}
		}
	}
}

// ---------------------------------------------------------------------------
// Pipeline
// ---------------------------------------------------------------------------

void ULumeCinematicCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ResolveTarget();
	TryTakeOverView();

	if (IsValid(TargetActor))
	{
		UpdateCamera(DeltaTime);
	}
}

void ULumeCinematicCameraComponent::RunModules(ELumeModuleStage Stage)
{
	for (ULumeCameraModule* Module : Modules)
	{
		if (Module && Module->bEnabled && Module->GetStage() == Stage)
		{
			Module->UpdateModule(Context, State);
		}
	}
}

void ULumeCinematicCameraComponent::UpdateCamera(float DeltaSeconds)
{
	const bool bSnap = bNeedsSnap;
	bNeedsSnap = false;

	const FVector Reference = GetTrackingReferenceLocation();
	State.TargetLocation = Reference;
	State.bTargetAirborne = IsTargetAirborne();

	UpdateOverrides(DeltaSeconds, Reference, bSnap);

	Context.Camera = this;
	Context.Target = TargetActor;
	Context.DeltaTime = DeltaSeconds;
	Context.bSnap = bSnap;

	// Valores desejados "crus": cada módulo substitui pela sua versão suavizada.
	// Assim qualquer módulo pode ser removido sem quebrar a câmera.
	const FLumeFramingSettings& F = Context.Framing;
	State.PivotLocation = Reference + F.PivotOffset;
	State.Rotation = F.Rotation;
	State.Distance = F.Distance;
	State.MinimumDistance = 0.f;
	State.ExtraDistance = 0.f;
	State.FieldOfView = F.FieldOfView;
	State.ScreenOffset = F.ScreenOffset;
	if (bSnap)
	{
		State.CameraRotation = F.Rotation;
		State.TargetVelocity = FVector::ZeroVector;
	}

	RunModules(ELumeModuleStage::PreCompose);
	ComposeCamera();
	State.FocusDistance = FVector::Dist(State.CameraLocation, State.PivotLocation);
	RunModules(ELumeModuleStage::PostCompose);

	SetWorldLocationAndRotation(State.CameraLocation, State.CameraRotation);
	SetFieldOfView(State.FieldOfView);

	if (bAlignControlsToCamera)
	{
		AlignControlRotation();
	}

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugInfo();
	}
#endif
}

void ULumeCinematicCameraComponent::ComposeCamera()
{
	const FQuat ViewQuat = State.Rotation.Quaternion();
	const FVector Forward = ViewQuat.GetForwardVector();
	const FVector Right = ViewQuat.GetRightVector();
	const FVector Up = ViewQuat.GetUpVector();

	// Converte a posição de tela normalizada em deslocamento no mundo, à distância do pivô.
	const double Distance = FMath::Max(0.0, static_cast<double>(State.Distance));
	const double HalfFovRad = FMath::DegreesToRadians(FMath::Clamp(static_cast<double>(State.FieldOfView), 5.0, 170.0) * 0.5);
	const double HalfWidth = Distance * FMath::Tan(HalfFovRad);
	const double HalfHeight = HalfWidth / FMath::Max(0.1, static_cast<double>(GetViewAspectRatio()));

	State.LookAtPoint = State.PivotLocation
		- Right * (State.ScreenOffset.X * HalfWidth)
		- Up * (State.ScreenOffset.Y * HalfHeight);

	FVector Location = State.LookAtPoint - Forward * Distance;
	FRotator Rotation = State.Rotation;

	// Ponto de vista fixo (zonas): a câmera vai até o ponto e gira para acompanhar o jogador.
	const double FixedAlpha = FMath::Clamp(static_cast<double>(Context.FixedViewpointAlpha), 0.0, 1.0);
	if (FixedAlpha > 0.0)
	{
		const FVector FixedLocation = Context.FixedViewpointLocation;
		FRotator LookRotation = (State.PivotLocation - FixedLocation).Rotation();
		LookRotation.Roll = State.Rotation.Roll;

		Location = FMath::Lerp(Location, FixedLocation, FixedAlpha);
		Rotation = FQuat::Slerp(Rotation.Quaternion(), LookRotation.Quaternion(), FixedAlpha).Rotator();
		State.LookAtPoint = FMath::Lerp(State.LookAtPoint, State.PivotLocation, FixedAlpha);
	}

	State.CameraLocation = Location;
	State.CameraRotation = Rotation;
}

void ULumeCinematicCameraComponent::AlignControlRotation()
{
	// Só em World Fixed: nos outros modos a câmera segue o personagem e isso criaria um ciclo.
	const ULumeRotationModule* RotationModule = FindModule<ULumeRotationModule>();
	if (!RotationModule || !RotationModule->bEnabled || RotationModule->Mode != ELumeRotationMode::WorldFixed)
	{
		return;
	}

	const APawn* Pawn = Cast<APawn>(TargetActor);
	APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
	if (!PC || !PC->IsLocalController() || PC->GetViewTarget() != GetOwner())
	{
		return;
	}

	FRotator Control = PC->GetControlRotation();
	Control.Yaw = State.CameraRotation.Yaw;
	PC->SetControlRotation(Control);
}

// ---------------------------------------------------------------------------
// Módulos
// ---------------------------------------------------------------------------

ULumeCameraModule* ULumeCinematicCameraComponent::FindModuleByClass(TSubclassOf<ULumeCameraModule> ModuleClass) const
{
	if (!ModuleClass)
	{
		return nullptr;
	}
	for (ULumeCameraModule* Module : Modules)
	{
		if (Module && Module->IsA(ModuleClass))
		{
			return Module;
		}
	}
	return nullptr;
}

ULumeCameraModule* ULumeCinematicCameraComponent::AddModule(TSubclassOf<ULumeCameraModule> ModuleClass, int32 Index)
{
	if (!ModuleClass || ModuleClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}
	ULumeCameraModule* Module = NewObject<ULumeCameraModule>(this, ModuleClass);
	if (Index < 0 || Index >= Modules.Num())
	{
		Modules.Add(Module);
	}
	else
	{
		Modules.Insert(Module, Index);
	}
	return Module;
}

// ---------------------------------------------------------------------------
// Depuração
// ---------------------------------------------------------------------------

void ULumeCinematicCameraComponent::DrawDebugInfo() const
{
#if ENABLE_DRAW_DEBUG
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	DrawDebugSphere(World, State.TargetLocation, 10.f, 8, FColor::Green, false, -1.f, 0, 1.f);
	DrawDebugSphere(World, State.PivotLocation, 16.f, 10, FColor::Yellow, false, -1.f, 0, 1.5f);
	DrawDebugPoint(World, State.LookAtPoint, 10.f, FColor::Cyan, false, -1.f);
	DrawDebugLine(World, State.PivotLocation, State.LookAtPoint, FColor::Cyan, false, -1.f, 0, 1.f);

	for (const ULumeCameraModule* Module : Modules)
	{
		if (Module && Module->bEnabled)
		{
			Module->DrawDebug(Context, State);
		}
	}

	if (GEngine)
	{
		for (const FActiveOverride& Entry : ActiveOverrides)
		{
			const FString Name = Entry.Zone.IsValid() ? Entry.Zone->GetActorNameOrLabel() : FString::Printf(TEXT("Override #%d"), Entry.Handle);
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange,
				FString::Printf(TEXT("LUME  %s  peso %.2f  prioridade %d"), *Name, FMath::SmoothStep(0.f, 1.f, Entry.Weight), Entry.Priority));
		}
		const AActor* Owner = GetOwner();
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White,
			FString::Printf(TEXT("LUME  %s  dist %.0f  fov %.1f  foco %.0f  vel %.0f%s"),
				Owner ? *Owner->GetActorNameOrLabel() : TEXT("?"), State.Distance, State.FieldOfView, State.FocusDistance,
				State.TargetVelocity.Size(), State.bTargetAirborne ? TEXT("  [no ar]") : TEXT("")));
	}
#endif
}
