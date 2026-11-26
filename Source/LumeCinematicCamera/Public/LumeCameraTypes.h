#pragma once

#include "CoreMinimal.h"
#include "LumeCameraTypes.generated.h"

class ULumeCinematicCameraComponent;
class UBoxComponent;

/** Ponto do alvo usado como referência de rastreamento. */
UENUM(BlueprintType)
enum class ELumeTrackingReference : uint8
{
	/** Localização do ator (cápsula). Recomendado: totalmente independente das animações do personagem. */
	ActorLocation	UMETA(DisplayName = "Actor Location (recomendado)"),
	/** Localização de um componente específico do alvo, buscado pelo nome. */
	NamedComponent	UMETA(DisplayName = "Named Component"),
	/** Localização de um socket/bone. Atenção: herda o movimento das animações. */
	Socket			UMETA(DisplayName = "Socket / Bone")
};

/** Como o eixo vertical do alvo é seguido. */
UENUM(BlueprintType)
enum class ELumeVerticalTracking : uint8
{
	/** Segue a altura do alvo o tempo todo. */
	Full				UMETA(DisplayName = "Full"),
	/** Ignora pulos e pequenas quedas: mantém a altura do chão enquanto o alvo estiver no ar. */
	StableWhileAirborne	UMETA(DisplayName = "Stable While Airborne")
};

/** Fonte da rotação da câmera. */
UENUM(BlueprintType)
enum class ELumeRotationMode : uint8
{
	/** Rotação absoluta no mundo, definida pelo enquadramento (Framing.Rotation). Clássico cinematográfico. */
	WorldFixed				UMETA(DisplayName = "World Fixed"),
	/** Fica atrás do alvo, seguindo a rotação dele. Framing.Rotation funciona como deslocamento. */
	FollowTargetHeading		UMETA(DisplayName = "Follow Target Heading"),
	/** Alinha-se lentamente com a direção do movimento. Framing.Rotation funciona como deslocamento. */
	FollowMovementDirection	UMETA(DisplayName = "Follow Movement Direction"),
	/** Usa a Control Rotation do controller (órbita livre do jogador). Framing.Rotation funciona como deslocamento. */
	ControlRotation			UMETA(DisplayName = "Control Rotation")
};

/** Etapa do pipeline em que um módulo é executado. */
UENUM(BlueprintType)
enum class ELumeModuleStage : uint8
{
	/** Antes da composição: define pivô, rotação, distância, FOV e deslocamento de tela. */
	PreCompose	UMETA(DisplayName = "Pre Compose"),
	/** Depois da composição: ajusta a localização/rotação final (colisão, limites, DOF, ruído). */
	PostCompose	UMETA(DisplayName = "Post Compose")
};

/** Presets de enquadramento para zonas cinematográficas. */
UENUM(BlueprintType)
enum class ELumeZonePreset : uint8
{
	Custom		UMETA(DisplayName = "Custom"),
	/** Câmera próxima, FOV fechado, pouca antecipação. */
	Corridor	UMETA(DisplayName = "Corredor"),
	/** Câmera afastada e mais alta, FOV aberto, muita antecipação. */
	LargeRoom	UMETA(DisplayName = "Sala Grande"),
	/** Enquadramento descentralizado, ângulo holandês, foco raso, vinheta, granulação e câmera na mão. */
	Horror		UMETA(DisplayName = "Terror")
};

/** Presets de tremor de câmera. */
UENUM(BlueprintType)
enum class ELumeShakePreset : uint8
{
	/** Golpe leve: tremor curto e pequeno. */
	LightHit	UMETA(DisplayName = "Golpe Leve"),
	/** Golpe forte: tremor médio com empurrão. */
	HeavyHit	UMETA(DisplayName = "Golpe Forte"),
	/** Explosão: tremor forte, empurra para longe da explosão. */
	Explosion	UMETA(DisplayName = "Explosão"),
	/** Aterrissagem pesada: tranco para baixo. */
	Landing		UMETA(DisplayName = "Aterrissagem"),
	/** Susto: tranco seco e rápido. */
	Jumpscare	UMETA(DisplayName = "Susto")
};

/**
 * Enquadramento completo da câmera. É o "perfil base" da câmera e o resultado
 * da mistura com todas as zonas/overrides ativos.
 */
USTRUCT(BlueprintType)
struct LUMECINEMATICCAMERA_API FLumeFramingSettings
{
	GENERATED_BODY()

	/** Distância da câmera até o pivô (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (ClampMin = "0", UIMin = "50", UIMax = "3000", Units = "cm"))
	float Distance = 600.f;

	/** Campo de visão horizontal (graus). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (ClampMin = "5", ClampMax = "170", Units = "deg"))
	float FieldOfView = 70.f;

	/** Deslocamento do pivô em relação à referência do alvo, em espaço do mundo (ex.: Z=50 mira no peito). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing")
	FVector PivotOffset = FVector(0.0, 0.0, 50.0);

	/**
	 * Posição do personagem na tela, normalizada (-1..1). X: horizontal (positivo = direita),
	 * Y: vertical (positivo = acima). Ex.: (0.33, 0) = regra dos terços à direita.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (UIMin = "-0.9", UIMax = "0.9"))
	FVector2D ScreenOffset = FVector2D::ZeroVector;

	/** Rotação da câmera. Absoluta no modo World Fixed; deslocamento nos outros modos. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation")
	FRotator Rotation = FRotator(-15.0, 0.0, 0.0);

	/** Distância de antecipação na direção do movimento (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking", meta = (ClampMin = "0", Units = "cm"))
	float LookAheadDistance = 150.f;

	/** Zona morta horizontal (X) e vertical (Y) em cm: dentro dela a câmera não se move. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking", meta = (ClampMin = "0"))
	FVector2D DeadZone = FVector2D(30.0, 20.0);

	/** Tempo de suavização do rastreamento (s). Maior = câmera mais "pesada". */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0", Units = "s"))
	float TrackingSmoothTime = 0.35f;

	/** Tempo de suavização da rotação (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0", Units = "s"))
	float RotationSmoothTime = 0.6f;

	/** Tempo de suavização de zoom/FOV (s). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0", Units = "s"))
	float ZoomSmoothTime = 0.8f;

	/** Abertura (f-stop) para profundidade de campo. Menor = foco mais raso. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (ClampMin = "0.7", ClampMax = "32"))
	float Aperture = 5.6f;

	/** Deslocamento da distância de foco em relação ao alvo (cm). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (Units = "cm"))
	float FocusOffset = 0.f;

	/** Intensidade da vinheta. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (ClampMin = "0", ClampMax = "1"))
	float VignetteIntensity = 0.4f;

	/** Intensidade da granulação de filme. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (ClampMin = "0", ClampMax = "1"))
	float FilmGrainIntensity = 0.f;

	/** Amplitude do efeito "câmera na mão" (graus). 0 = desligado. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (ClampMin = "0", UIMax = "5", Units = "deg"))
	float NoiseAmplitude = 0.f;

	/** Frequência do efeito "câmera na mão" (Hz). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (ClampMin = "0", UIMax = "5"))
	float NoiseFrequency = 0.35f;
};

/**
 * Sobrescritas parciais de enquadramento, usadas por zonas e overrides de script.
 * Apenas os campos marcados são misturados sobre o enquadramento atual.
 */
USTRUCT(BlueprintType)
struct LUMECINEMATICCAMERA_API FLumeFramingOverrides
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (InlineEditConditionToggle))
	bool bOverride_Distance = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (EditCondition = "bOverride_Distance", ClampMin = "0", Units = "cm"))
	float Distance = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (InlineEditConditionToggle))
	bool bOverride_FieldOfView = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (EditCondition = "bOverride_FieldOfView", ClampMin = "5", ClampMax = "170", Units = "deg"))
	float FieldOfView = 70.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (InlineEditConditionToggle))
	bool bOverride_PivotOffset = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (EditCondition = "bOverride_PivotOffset"))
	FVector PivotOffset = FVector(0.0, 0.0, 50.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (InlineEditConditionToggle))
	bool bOverride_ScreenOffset = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Framing", meta = (EditCondition = "bOverride_ScreenOffset", UIMin = "-0.9", UIMax = "0.9"))
	FVector2D ScreenOffset = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (InlineEditConditionToggle))
	bool bOverride_Pitch = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (EditCondition = "bOverride_Pitch", Units = "deg"))
	float Pitch = -15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (InlineEditConditionToggle))
	bool bOverride_Yaw = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (EditCondition = "bOverride_Yaw", Units = "deg"))
	float Yaw = 0.f;

	/** Roll = ângulo holandês (dutch angle). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (InlineEditConditionToggle))
	bool bOverride_Roll = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rotation", meta = (EditCondition = "bOverride_Roll", Units = "deg"))
	float Roll = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking", meta = (InlineEditConditionToggle))
	bool bOverride_LookAheadDistance = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking", meta = (EditCondition = "bOverride_LookAheadDistance", ClampMin = "0", Units = "cm"))
	float LookAheadDistance = 150.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking", meta = (InlineEditConditionToggle))
	bool bOverride_DeadZone = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tracking", meta = (EditCondition = "bOverride_DeadZone", ClampMin = "0"))
	FVector2D DeadZone = FVector2D(30.0, 20.0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (InlineEditConditionToggle))
	bool bOverride_TrackingSmoothTime = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (EditCondition = "bOverride_TrackingSmoothTime", ClampMin = "0", Units = "s"))
	float TrackingSmoothTime = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (InlineEditConditionToggle))
	bool bOverride_RotationSmoothTime = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (EditCondition = "bOverride_RotationSmoothTime", ClampMin = "0", Units = "s"))
	float RotationSmoothTime = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (InlineEditConditionToggle))
	bool bOverride_ZoomSmoothTime = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (EditCondition = "bOverride_ZoomSmoothTime", ClampMin = "0", Units = "s"))
	float ZoomSmoothTime = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (InlineEditConditionToggle))
	bool bOverride_Aperture = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (EditCondition = "bOverride_Aperture", ClampMin = "0.7", ClampMax = "32"))
	float Aperture = 5.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (InlineEditConditionToggle))
	bool bOverride_FocusOffset = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (EditCondition = "bOverride_FocusOffset", Units = "cm"))
	float FocusOffset = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (InlineEditConditionToggle))
	bool bOverride_VignetteIntensity = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (EditCondition = "bOverride_VignetteIntensity", ClampMin = "0", ClampMax = "1"))
	float VignetteIntensity = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (InlineEditConditionToggle))
	bool bOverride_FilmGrainIntensity = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lens", meta = (EditCondition = "bOverride_FilmGrainIntensity", ClampMin = "0", ClampMax = "1"))
	float FilmGrainIntensity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (InlineEditConditionToggle))
	bool bOverride_NoiseAmplitude = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (EditCondition = "bOverride_NoiseAmplitude", ClampMin = "0", UIMax = "5", Units = "deg"))
	float NoiseAmplitude = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (InlineEditConditionToggle))
	bool bOverride_NoiseFrequency = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Handheld", meta = (EditCondition = "bOverride_NoiseFrequency", ClampMin = "0", UIMax = "5"))
	float NoiseFrequency = 0.35f;

	/** Mistura os campos marcados sobre Out, com peso Alpha (0..1). */
	void ApplyTo(FLumeFramingSettings& Out, float Alpha) const;

	/** Cria as sobrescritas correspondentes a um preset. */
	static FLumeFramingOverrides MakePreset(ELumeZonePreset Preset);
};

/** Caixa de restrição fornecida por uma zona, com o peso atual da zona. */
USTRUCT(BlueprintType)
struct LUMECINEMATICCAMERA_API FLumeZoneConstraint
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	TObjectPtr<UBoxComponent> Box = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	float Weight = 0.f;
};

/** Estado da câmera que atravessa o pipeline de módulos a cada frame. */
USTRUCT(BlueprintType)
struct LUMECINEMATICCAMERA_API FLumeCameraState
{
	GENERATED_BODY()

	/** Referência bruta do alvo (sem suavização, sem PivotOffset). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FVector TargetLocation = FVector::ZeroVector;

	/** Velocidade suavizada do alvo (cm/s). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FVector TargetVelocity = FVector::ZeroVector;

	/** O alvo está no ar (pulando/caindo)? */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	bool bTargetAirborne = false;

	/** Ponto rastreado e suavizado que a câmera enquadra. */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FVector PivotLocation = FVector::ZeroVector;

	/** Rotação desejada (antes da composição). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Distância da câmera até o pivô (cm). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	float Distance = 600.f;

	/** Distância extra somada pelo zoom (ex.: pontos de interesse). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	float ExtraDistance = 0.f;

	/** Distância mínima exigida (ex.: para enquadrar múltiplos alvos). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	float MinimumDistance = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	float FieldOfView = 70.f;

	/** Posição normalizada do pivô na tela. */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FVector2D ScreenOffset = FVector2D::ZeroVector;

	/** Ponto para onde a câmera olha (pivô deslocado pelo enquadramento de tela). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FVector LookAtPoint = FVector::ZeroVector;

	/** Localização final da câmera. */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FVector CameraLocation = FVector::ZeroVector;

	/** Rotação final da câmera. */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	FRotator CameraRotation = FRotator::ZeroRotator;

	/** Distância de foco aplicada (cm). */
	UPROPERTY(BlueprintReadWrite, Category = "LUME")
	float FocusDistance = 600.f;
};

/** Dados somente leitura disponíveis para os módulos em cada atualização. */
USTRUCT(BlueprintType)
struct LUMECINEMATICCAMERA_API FLumeCameraContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	TObjectPtr<ULumeCinematicCameraComponent> Camera = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	TObjectPtr<AActor> Target = nullptr;

	/** Enquadramento atual (perfil base misturado com zonas e overrides). */
	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	FLumeFramingSettings Framing;

	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	float DeltaTime = 0.f;

	/** Verdadeiro quando a câmera deve saltar direto para o destino (início, teleporte, troca de alvo). */
	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	bool bSnap = false;

	/** Peso (0..1) de pontos de vista fixos fornecidos por zonas. */
	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	float FixedViewpointAlpha = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	FVector FixedViewpointLocation = FVector::ZeroVector;

	/** Restrições de movimento fornecidas pelas zonas ativas. */
	UPROPERTY(BlueprintReadOnly, Category = "LUME")
	TArray<FLumeZoneConstraint> ZoneConstraints;
};
