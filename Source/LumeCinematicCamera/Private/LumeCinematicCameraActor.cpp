#include "LumeCinematicCameraActor.h"
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
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ALumeCinematicCamera::ALumeCinematicCamera()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// Depois da física/movimento do personagem: a câmera sempre vê a posição final do frame.
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SceneRoot);
	CameraComponent->bConstrainAspectRatio = false;
	CameraComponent->SetFieldOfView(BaseFraming.FieldOfView);

	// Pipeline padrão (a ordem importa).
	Modules.Add(CreateDefaultSubobject<ULumeTrackingModule>(TEXT("TrackingModule")));
	Modules.Add(CreateDefaultSubobject<ULumeFramingModule>(TEXT("FramingModule")));
	Modules.Add(CreateDefaultSubobject<ULumeRotationModule>(TEXT("RotationModule")));
	Modules.Add(CreateDefaultSubobject<ULumeDynamicZoomModule>(TEXT("DynamicZoomModule")));
	Modules.Add(CreateDefaultSubobject<ULumeCollisionModule>(TEXT("CollisionModule")));
	Modules.Add(CreateDefaultSubobject<ULumeBoundsModule>(TEXT("BoundsModule")));
	Modules.Add(CreateDefaultSubobject<ULumeDepthOfFieldModule>(TEXT("DepthOfFieldModule")));
	Modules.Add(CreateDefaultSubobject<ULumeHandheldNoiseModule>(TEXT("HandheldNoiseModule")));
}

ULumeCameraSubsystem* ALumeCinematicCamera::GetLumeSubsystem() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetSubsystem<ULumeCameraSubsystem>() : nullptr;
}

void ALumeCinematicCamera::BeginPlay()
{
	Super::BeginPlay();

	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->RegisterCamera(this);
	}

	if (IsValid(TargetActor))
	{
		SetTarget(TargetActor, true);
	}

	bNeedsSnap = true;
	bActivationPending = bAutoActivate;
	TryAutoActivate();
}

void ALumeCinematicCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->UnregisterCamera(this);
	}
	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------
// Alvo / ativação
// ---------------------------------------------------------------------------

void ALumeCinematicCamera::SetTarget(AActor* NewTarget, bool bSnap)
{
	AActor* OldTarget = TargetActor;
	if (IsValid(OldTarget) && OldTarget != NewTarget)
	{
		RemoveTickPrerequisiteActor(OldTarget);
	}

	TargetActor = NewTarget;

	if (IsValid(NewTarget) && NewTarget != this)
	{
		AddTickPrerequisiteActor(NewTarget);
	}

	if (bSnap)
	{
		bNeedsSnap = true;
	}
}

void ALumeCinematicCamera::SnapToTarget()
{
	bNeedsSnap = true;
}

void ALumeCinematicCamera::ResolveTarget()
{
	if (TargetActor && !IsValid(TargetActor))
	{
		TargetActor = nullptr;
	}

	if (!TargetActor && bAutoTargetPlayerPawn)
	{
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, PlayerIndex))
		{
			SetTarget(Pawn, true);
		}
	}
}

void ALumeCinematicCamera::TryAutoActivate()
{
	if (!bActivationPending)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, PlayerIndex);
	if (!PC)
	{
		return;
	}
	// Espera a possessão do pawn, senão a possessão substituiria o view target.
	if (bAutoTargetPlayerPawn && !TargetActor && !PC->GetPawn())
	{
		return;
	}

	bActivationPending = false;
	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->ActivateCamera(this, PC, AutoActivateBlendTime, VTBlend_EaseInOut, 2.f);
	}
}

void ALumeCinematicCamera::ActivateCamera(float BlendTime, EViewTargetBlendFunction BlendFunction, float BlendExponent)
{
	bActivationPending = false;
	if (ULumeCameraSubsystem* Subsystem = GetLumeSubsystem())
	{
		Subsystem->ActivateCamera(this, UGameplayStatics::GetPlayerController(this, PlayerIndex), BlendTime, BlendFunction, BlendExponent);
	}
}

USceneComponent* ALumeCinematicCamera::FindReferenceComponent() const
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

FVector ALumeCinematicCamera::GetTrackingReferenceLocation() const
{
	const AActor* Target = TargetActor;
	if (!IsValid(Target))
	{
		return GetActorLocation();
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

bool ALumeCinematicCamera::IsTargetAirborne() const
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

float ALumeCinematicCamera::GetViewAspectRatio() const
{
	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, PlayerIndex))
	{
		int32 SizeX = 0;
		int32 SizeY = 0;
		PC->GetViewportSize(SizeX, SizeY);
		if (SizeX > 0 && SizeY > 0)
		{
			return static_cast<float>(SizeX) / static_cast<float>(SizeY);
		}
	}
	return CameraComponent && CameraComponent->AspectRatio > 0.f ? CameraComponent->AspectRatio : 16.f / 9.f;
}

// ---------------------------------------------------------------------------
// Overrides / zonas
// ---------------------------------------------------------------------------

int32 ALumeCinematicCamera::PushFramingOverride(const FLumeFramingOverrides& Overrides, int32 Priority, float BlendInTime, float BlendOutTime)
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

void ALumeCinematicCamera::PopFramingOverride(int32 Handle)
{
	for (FActiveOverride& Entry : ActiveOverrides)
	{
		if (Entry.Handle == Handle && Handle != INDEX_NONE)
		{
			Entry.bWantsActive = false;
		}
	}
}

void ALumeCinematicCamera::ClearFramingOverrides()
{
	for (FActiveOverride& Entry : ActiveOverrides)
	{
		if (Entry.Handle != INDEX_NONE)
		{
			Entry.bWantsActive = false;
		}
	}
}

float ALumeCinematicCamera::GetZoneWeight(const ALumeCameraZone* Zone) const
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

void ALumeCinematicCamera::UpdateOverrides(float DeltaSeconds, const FVector& ReferenceLocation, bool bSnap)
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

void ALumeCinematicCamera::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ResolveTarget();
	TryAutoActivate();

	if (IsValid(TargetActor))
	{
		UpdateCamera(DeltaSeconds);
	}
}

void ALumeCinematicCamera::RunModules(ELumeModuleStage Stage)
{
	for (ULumeCameraModule* Module : Modules)
	{
		if (Module && Module->bEnabled && Module->GetStage() == Stage)
		{
			Module->UpdateModule(Context, State);
		}
	}
}

void ALumeCinematicCamera::UpdateCamera(float DeltaSeconds)
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

	CameraComponent->SetWorldLocationAndRotation(State.CameraLocation, State.CameraRotation);
	CameraComponent->SetFieldOfView(State.FieldOfView);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		DrawDebugInfo();
	}
#endif
}

void ALumeCinematicCamera::ComposeCamera()
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

// ---------------------------------------------------------------------------
// Módulos
// ---------------------------------------------------------------------------

ULumeCameraModule* ALumeCinematicCamera::FindModuleByClass(TSubclassOf<ULumeCameraModule> ModuleClass) const
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

ULumeCameraModule* ALumeCinematicCamera::AddModule(TSubclassOf<ULumeCameraModule> ModuleClass, int32 Index)
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

void ALumeCinematicCamera::DrawDebugInfo() const
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
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White,
			FString::Printf(TEXT("LUME  %s  dist %.0f  fov %.1f  foco %.0f  vel %.0f%s"),
				*GetActorNameOrLabel(), State.Distance, State.FieldOfView, State.FocusDistance, State.TargetVelocity.Size(),
				State.bTargetAirborne ? TEXT("  [no ar]") : TEXT("")));
	}
#endif
}
