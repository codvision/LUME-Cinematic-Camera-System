#pragma once

#include "CoreMinimal.h"

class UBoxComponent;

/**
 * Utilidades matemáticas do LUME.
 * SmoothDamp = mola criticamente amortecida, independente de frame rate e sem overshoot:
 * a base de todos os movimentos suaves (sem trancos) do sistema.
 */
namespace LumeMath
{
	inline double SmoothDamp(double Current, double Target, double& Velocity, double SmoothTime, double DeltaTime)
	{
		if (DeltaTime <= 0.0)
		{
			return Current;
		}
		if (SmoothTime <= UE_KINDA_SMALL_NUMBER)
		{
			Velocity = 0.0;
			return Target;
		}

		const double Omega = 2.0 / SmoothTime;
		const double X = Omega * DeltaTime;
		const double Exp = 1.0 / (1.0 + X + 0.48 * X * X + 0.235 * X * X * X);
		const double Change = Current - Target;
		const double Temp = (Velocity + Omega * Change) * DeltaTime;
		Velocity = (Velocity - Omega * Temp) * Exp;
		double Output = Target + (Change + Temp) * Exp;

		// Evita ultrapassar o destino.
		if ((Target - Current > 0.0) == (Output > Target))
		{
			Output = Target;
			Velocity = 0.0;
		}
		return Output;
	}

	inline FVector SmoothDamp(const FVector& Current, const FVector& Target, FVector& Velocity, double SmoothTime, double DeltaTime)
	{
		return FVector(
			SmoothDamp(Current.X, Target.X, Velocity.X, SmoothTime, DeltaTime),
			SmoothDamp(Current.Y, Target.Y, Velocity.Y, SmoothTime, DeltaTime),
			SmoothDamp(Current.Z, Target.Z, Velocity.Z, SmoothTime, DeltaTime));
	}

	inline FVector2D SmoothDamp(const FVector2D& Current, const FVector2D& Target, FVector2D& Velocity, double SmoothTime, double DeltaTime)
	{
		return FVector2D(
			SmoothDamp(Current.X, Target.X, Velocity.X, SmoothTime, DeltaTime),
			SmoothDamp(Current.Y, Target.Y, Velocity.Y, SmoothTime, DeltaTime));
	}

	/** Suaviza ângulos pelo caminho mais curto. Velocity guarda (Pitch, Yaw, Roll). */
	inline FRotator SmoothDampRotator(const FRotator& Current, const FRotator& Target, FVector& Velocity, double SmoothTime, double DeltaTime)
	{
		auto DampAngle = [SmoothTime, DeltaTime](double Cur, double Tgt, double& Vel)
		{
			const double UnwoundTarget = Cur + FMath::FindDeltaAngleDegrees(Cur, Tgt);
			return FRotator::NormalizeAxis(SmoothDamp(Cur, UnwoundTarget, Vel, SmoothTime, DeltaTime));
		};

		return FRotator(
			DampAngle(Current.Pitch, Target.Pitch, Velocity.X),
			DampAngle(Current.Yaw, Target.Yaw, Velocity.Y),
			DampAngle(Current.Roll, Target.Roll, Velocity.Z));
	}

	/** O ponto está dentro da caixa (considerando rotação e escala)? */
	LUMECINEMATICCAMERA_API bool IsPointInsideBox(const UBoxComponent* Box, const FVector& Point, float Margin = 0.f);

	/** Projeta o ponto para dentro da caixa (considerando rotação e escala). Margin em cm. */
	LUMECINEMATICCAMERA_API FVector ClampPointToBox(const UBoxComponent* Box, const FVector& Point, float Margin = 0.f);
}
