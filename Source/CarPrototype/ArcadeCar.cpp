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

	Thrust = 10000000;
	TurnStrength = 10000;
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

	for (int32 i = 0; i < ToplinkComponents.Num(); i++)
	{
		auto ToplinkComponent = ToplinkComponents[i];
		auto upVector = ToplinkComponent->GetUpVector();
		auto suspension = SuspensionArray[i];
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

		///springforce = stiffness * (restlength - length)
		auto springForce = suspension.stiffness * (suspension.restLength - Length[i]);

		///damperforce = damper * (lastl - length)/dt;
		auto damperForce = suspension.damper * ((LastLength[i] - Length[i]) / DeltaTime);

		auto fz = FMath::Clamp(damperForce + springForce, suspension.forceMin, suspension.forceMax);
		Body->AddForceAtLocation(fz * 100 * upVector, toplinkLocation);

		if (wheelComponent) {
			wheelComponent->SetRelativeLocation(FVector(0, 0, -Length[i]));
		}

		if (debugForces)
		{		
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fz / 35.f * upVector, FColor::Purple, false, 0.0f, 0, 9.f);
		}
		LastLength[i] = Length[i];
	}

	FVector fx = FVector::ZeroVector;
	FVector fy = FVector::ZeroVector;
	if (FMath::Abs(MovementInput.X) > 0)
		fx = GetActorForwardVector() * Thrust * MovementInput.X;
	if (FMath::Abs(MovementInput.Y) > 0)
		fy = GetActorUpVector() * TurnStrength * MovementInput.Y;

	Body->AddForce(fx);
	Body->AddTorque(fy);

	TopLink_FL->SetRelativeRotation(FRotator(0, steeringAngle, 0));
	TopLink_FR->SetRelativeRotation(FRotator(0, steeringAngle, 0));
}
