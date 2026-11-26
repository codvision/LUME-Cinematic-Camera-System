#include "LumeCameraMath.h"
#include "Components/BoxComponent.h"

namespace LumeMath
{
	static FVector GetLocalExtent(const UBoxComponent* Box, const FTransform& Transform, float Margin)
	{
		const FVector Extent = Box->GetUnscaledBoxExtent();
		const FVector Scale = Transform.GetScale3D().GetAbs();
		const double MarginD = Margin;
		const double MinScale = UE_KINDA_SMALL_NUMBER;

		return FVector(
			FMath::Max(0.0, Extent.X - MarginD / FMath::Max(Scale.X, MinScale)),
			FMath::Max(0.0, Extent.Y - MarginD / FMath::Max(Scale.Y, MinScale)),
			FMath::Max(0.0, Extent.Z - MarginD / FMath::Max(Scale.Z, MinScale)));
	}

	bool IsPointInsideBox(const UBoxComponent* Box, const FVector& Point, float Margin)
	{
		if (!Box)
		{
			return false;
		}
		const FTransform& Transform = Box->GetComponentTransform();
		const FVector Local = Transform.InverseTransformPosition(Point);
		const FVector Extent = GetLocalExtent(Box, Transform, Margin);

		return FMath::Abs(Local.X) <= Extent.X
			&& FMath::Abs(Local.Y) <= Extent.Y
			&& FMath::Abs(Local.Z) <= Extent.Z;
	}

	FVector ClampPointToBox(const UBoxComponent* Box, const FVector& Point, float Margin)
	{
		if (!Box)
		{
			return Point;
		}
		const FTransform& Transform = Box->GetComponentTransform();
		const FVector Local = Transform.InverseTransformPosition(Point);
		const FVector Extent = GetLocalExtent(Box, Transform, Margin);

		const FVector Clamped(
			FMath::Clamp(Local.X, -Extent.X, Extent.X),
			FMath::Clamp(Local.Y, -Extent.Y, Extent.Y),
			FMath::Clamp(Local.Z, -Extent.Z, Extent.Z));

		return Transform.TransformPosition(Clamped);
	}
}
