// Fill out your copyright notice in the Description page of Project Settings.

#include "ArcadeCar.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"
#include "Runtime/Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Runtime/Engine/Classes/Components/BoxComponent.h"
#include "Runtime/Engine/Classes/Camera/CameraComponent.h"
#include "Runtime/Engine/Classes/GameFramework/SpringArmComponent.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Curves/CurveFloat.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/GameEngine.h"

// Sets default values
AArcadeCar::AArcadeCar()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AArcadeCar::BeginPlay()
{
	Super::BeginPlay();
	Body->SetCenterOfMass(FVector(35, 0, -30));
}

// Called every frame
void AArcadeCar::Tick(float DeltaTime)
{
	APawn::Tick(DeltaTime);

	auto transform = Body->GetComponentTransform();
	auto up = Body->GetUpVector();
	UWorld* World = GetWorld();
	FHitResult Hit;
	FCollisionQueryParams TraceParams(true);
	TraceParams.AddIgnoredActor(this);

	//MovementInput = MovementInput.GetSafeNormal();
	if (MovementInput.Y != 0)
	{
		steeringAngle += MovementInput.Y * 120 * DeltaTime;
		steeringAngle = FMath::Clamp(steeringAngle, -45.f, 45.f);
	}
	else
	{
		steeringAngle *= 0.7f;
	}

	if (Throttle != 0)
	{
		ThrottleValue += DeltaTime;
	}
	else {
		ThrottleValue -= DeltaTime * 2.f;
	}

	ThrottleValue = FMath::Min(1.f, ThrottleValue);
	ThrottleValue = FMath::Max(0.f, ThrottleValue);

	engineRPM += DeltaTime * FMath::Lerp(-3000, 5000, ThrottleValue);
	engineRPM = FMath::Clamp(engineRPM, (float)idleRPM, (float)maxRPM);

	float engineTorque = 0;
	if (EngineCurve)
		engineTorque = EngineCurve->GetFloatValue(engineRPM) * ThrottleValue;
	float torque = FMath::Lerp(backTorque, engineTorque, ThrottleValue);

	//angularAcceleration = torque/inertia
	auto angularAcceleration = torque / inertia;
	engineAngularVelocity = FMath::Clamp(engineAngularVelocity + angularAcceleration * DeltaTime, RPM_TO_RADPS * idleRPM, RPM_TO_RADPS * maxRPM);

	engineRPM = engineAngularVelocity * RADPS_TO_RPM;

	for (int32 i = 0; i < ToplinkComponents.Num(); i++)
	{
		auto ToplinkComponent = ToplinkComponents[i];
		auto upVector = ToplinkComponent->GetUpVector();
		auto ToplinkComponentTransform = ToplinkComponent->GetComponentTransform();
		auto suspension = SuspensionArray[i];
		auto wheel = Wheels[i];
		auto wheelComponent = WheelComponents[i];
		FVector toplinkLocation = ToplinkComponent->GetComponentLocation();
		FVector start = toplinkLocation;
		FVector end = start - upVector * Raylength[i];

		bool wheelContact = GetWorld()->LineTraceSingleByObjectType(Hit, start, end, ECC_WorldDynamic | ECC_WorldStatic, TraceParams);
		if (wheelContact) {
			Length[i] = FMath::Clamp((toplinkLocation - (Hit.Location + upVector * Wheels[i].radius)).Size(), suspension.restLength - suspension.travel, suspension.restLength + suspension.travel);
		}
		else
		{
			Length[i] = suspension.restLength + suspension.travel;
		}

		engineTorque = FMath::Max(0.f, engineTorque);
		totalGearRatio = mainGear * gear;
		auto currentTorqueRation = 0;
		if (i < 2) {
			currentTorqueRation = TorqueRatio[0];
		}
		else {
			currentTorqueRation = TorqueRatio[1];
		}
		DriveTorque[i] = currentTorqueRation * engineTorque * totalGearRatio * 0.5f;

		///angularAcceleration = torque/inertia
		float angularAcceleration = DriveTorque[i] / wheelInertia[i];

		///max wheelspeed on current gear
		float maxWheelSpeed = 99999.f;
		if (gear != 1) {
			maxWheelSpeed = engineAngularVelocity / totalGearRatio;
		}

		///angularvelocity += angularacceleration*dt
		wheelAngularVelocity[i] = wheelAngularVelocity[i] + angularAcceleration * DeltaTime;
		wheelAngularVelocity[i] = FMath::Min(FMath::Abs(wheelAngularVelocity[i]), FMath::Abs(maxWheelSpeed)) * FMath::Sign(maxWheelSpeed);


		///springforce = stiffness * (restlength - length)
		auto springForce = suspension.stiffness * (suspension.restLength - Length[i]);

		///damperforce = damper * (lastl - length)/dt;
		auto damperForce = suspension.damper * ((LastLength[i] - Length[i]) / DeltaTime);

		auto fz = FMath::Clamp(damperForce + springForce, suspension.forceMin, suspension.forceMax);
		float fx = 0;
		float fy = 0;
		FVector point = wheelComponent->GetComponentLocation() - wheel.radius * upVector;
		auto velocity = Body->GetPhysicsLinearVelocityAtPoint(point);
		auto wheelLinearVelocityLocal = UKismetMathLibrary::InverseTransformDirection(ToplinkComponentTransform, velocity / 100.0f);
		if (wheelContact) {

			fy = FMath::Clamp(fz * -wheelLinearVelocityLocal.Y, -fz, fz);
			fx = MovementInput.X * fz * 0.5f;

			if (GEngine) {
				GEngine->AddOnScreenDebugMessage(i, 15.0f, FColor::Yellow, FString::Printf(TEXT("%d %s"), i, *wheelLinearVelocityLocal.ToCompactString()));
				GEngine->AddOnScreenDebugMessage(i + 4, 15.0f, FColor::Yellow, FString::Printf(TEXT("fz: %d %s"), i, *(fz * 100 * upVector).ToCompactString()));
			}
		}

		auto forwadVector = ToplinkComponent->GetForwardVector();
		auto rightVector = ToplinkComponent->GetRightVector();
		auto tireForce = fx * forwadVector + fy * rightVector;
		Body->AddForceAtLocation(fz * 100 * upVector, toplinkLocation);
		Body->AddForceAtLocation(tireForce * 100, Hit.Location);
		if (wheelComponent) {
			wheelComponent->SetRelativeLocation(FVector(0, 0, -Length[i]));
			//wheelComponent->AddLocalRotation(FRotator(FMath::RadiansToDegrees(wheel.radius * wheelLinearVelocityLocal.X / 100.f * DeltaTime), 0, 0));
			float rotation = FMath::RadiansToDegrees(wheelAngularVelocity[i] / 100.f * DeltaTime);
			wheelComponent->AddLocalRotation(FRotator(rotation, 0, 0));
		}

		if (debugForces)
		{
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fx / 35.f * forwadVector, FColor::Green, false, 0.0f, 0, 9.f);
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fy / 35.f * rightVector, FColor::Blue, false, 0.0f, 0, 9.f);
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fz / 35.f * upVector, FColor::Purple, false, 0.0f, 0, 9.f);
			//	::DrawDebugPoint(World, HitResult.ImpactPoint, 16.0f, FColor::Red, false, 0.0f);
		}
		LastLength[i] = Length[i];
	}

	TopLink_FL->SetRelativeRotation(FRotator(0, steeringAngle, 0));
	TopLink_FR->SetRelativeRotation(FRotator(0, steeringAngle, 0));
}
