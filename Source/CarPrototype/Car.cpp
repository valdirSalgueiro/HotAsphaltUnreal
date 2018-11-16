// Fill out your copyright notice in the Description page of Project Settings.

#include "Car.h"
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

//#include "Kismet/GameplayStatics.h"

const FName TraceTag("MyTraceTag");

// Sets default values
ACar::ACar()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = Body;

	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	CameraSpringArm->TargetArmLength = 400;
	CameraSpringArm->SetRelativeLocation(FVector(0, 0, 200));
	CameraSpringArm->AttachTo(RootComponent);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	Camera->AttachTo(CameraSpringArm);

	TopLink_FL = CreateDefaultSubobject<USceneComponent>(TEXT("TopLink_FL"));
	TopLink_FL->AttachTo(RootComponent);
	TopLink_FL->SetRelativeLocation(FVector(125, -60, 80));
	Wheel_FL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_FL"));
	Wheel_FL->SetRelativeLocation(FVector(0, 0, -50));
	Wheel_FL->AttachTo(TopLink_FL);
	Wheels.push_back(Wheel_FL);
	ToplinkComponents.Add(TopLink_FL);

	TopLink_FR = CreateDefaultSubobject<USceneComponent>(TEXT("TopLink_FR"));
	TopLink_FR->AttachTo(RootComponent);
	TopLink_FR->SetRelativeLocation(FVector(125, 60, 80));
	Wheel_FR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_FR"));
	Wheel_FR->SetRelativeLocation(FVector(0, 0, -50));
	Wheel_FR->AttachTo(TopLink_FR);
	Wheels.push_back(Wheel_FR);
	ToplinkComponents.Add(TopLink_FR);

	TopLink_RL = CreateDefaultSubobject<USceneComponent>(TEXT("TopLink_RL"));
	TopLink_RL->AttachTo(RootComponent);
	TopLink_RL->SetRelativeLocation(FVector(-125, -60, 80));
	Wheel_RL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_RL"));
	Wheel_RL->SetRelativeLocation(FVector(0, 0, -50));
	Wheel_RL->AttachTo(TopLink_RL);
	Wheels.push_back(Wheel_RL);
	ToplinkComponents.Add(TopLink_RL);

	TopLink_RR = CreateDefaultSubobject<USceneComponent>(TEXT("TopLink_RR"));
	TopLink_RR->AttachTo(RootComponent);
	TopLink_RR->SetRelativeLocation(FVector(-125, 60, 80));
	Wheel_RR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_RR"));
	Wheel_RR->SetRelativeLocation(FVector(0, 0, -50));
	Wheel_RR->AttachTo(TopLink_RR);
	Wheels.push_back(Wheel_RR);
	ToplinkComponents.Add(TopLink_RR);

	steeringAngle = 0;

	for (int i = 0; i < ToplinkComponents.Num(); i++) {
		FSuspensionStruct suspension;
		suspension.restLength = 50;
		suspension.travel = 10;
		suspension.stiffness = 500;
		suspension.damper = 15;
		suspension.forceMin = -2000;
		suspension.forceMax = 6000;
		SuspensionArray.Add(suspension);

		FWheelStruct wheel;
		wheel.radius = 34;
		wheel.mass = 15;
		WheelArray.Add(wheel);

		auto length = suspension.restLength + suspension.travel;
		Length.push_back(length);
		LastLength.push_back(length);
		Raylength.push_back(wheel.radius + suspension.restLength + suspension.travel);
	}

	GearRatio.Add(-3.615);
	GearRatio.Add(0);
	GearRatio.Add(3.583);
	GearRatio.Add(2.038);
	GearRatio.Add(1.414);
	GearRatio.Add(1.108);
	GearRatio.Add(0.878);

	Throttle = 0;
	ThrottleValue = 0;
	idleRPM = 700;
	maxRPM = 7000;
	inertia = 0.3f;
	backTorque = -100.f;

	engineRPM = 0;

	engineAngularVelocity = 0;

	gear = 1;
	mainGear = 3.82;
	efficiency = 0.8;
	gearChangeTime = 0.5;
}

// Called when the game starts or when spawned
void ACar::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->DebugDrawTraceTag = TraceTag;
	Body->SetCenterOfMass(FVector(35, 0, -30));
}

// Called every frame
void ACar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	auto transform = Body->GetComponentTransform();
	auto up = Body->GetUpVector();
	UWorld* World = GetWorld();
	FHitResult Hit;
	FCollisionQueryParams TraceParams(true);
	TraceParams.AddIgnoredActor(this);
	//TraceParams.TraceTag = TraceTag;

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
	engineRPM = FMath::Clamp(engineRPM, idleRPM, maxRPM);

	float engineTorque = EngineCurve->GetFloatValue(engineRPM) * ThrottleValue;
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
		auto wheel = WheelArray[i];
		auto wheelComponent = Wheels[i];
		FVector toplinkLocation = ToplinkComponent->GetComponentLocation();
		FVector start = toplinkLocation;
		FVector end = start - upVector * Raylength[i];

		bool wheelContact = GetWorld()->LineTraceSingleByObjectType(Hit, start, end, ECC_WorldDynamic | ECC_WorldStatic, TraceParams);
		if (wheelContact) {
			Length[i] = FMath::Clamp((toplinkLocation - (Hit.Location + upVector * WheelArray[i].radius)).Size(), suspension.restLength - suspension.travel, suspension.restLength + suspension.travel);
		}
		else
		{
			Length[i] = suspension.restLength + suspension.travel;
		}

		//springforce = stiffness * (restlength - length)
		auto springForce = suspension.stiffness * (suspension.restLength - Length[i]);

		//damperforce = damper * (lastl - length)/dt;
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
		if (Wheels[i]) {
			Wheels[i]->SetRelativeLocation(FVector(0, 0, -Length[i]));
			Wheels[i]->AddLocalRotation(FRotator(FMath::RadiansToDegrees(wheel.radius * wheelLinearVelocityLocal.X / 100.f * DeltaTime), 0, 0));
		}
		LastLength[i] = Length[i];

		if (debugForces)
		{
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fx / 35.f * forwadVector, FColor::Green, false, 0.0f, 0, 9.f);
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fy / 35.f * rightVector, FColor::Blue, false, 0.0f, 0, 9.f);
			::DrawDebugLine(World, toplinkLocation, toplinkLocation + fz / 35.f * upVector, FColor::Purple, false, 0.0f, 0, 9.f);
			//	::DrawDebugPoint(World, HitResult.ImpactPoint, 16.0f, FColor::Red, false, 0.0f);
		}
	}
	TopLink_FL->SetRelativeRotation(FRotator(0, steeringAngle, 0));
	TopLink_FR->SetRelativeRotation(FRotator(0, steeringAngle, 0));
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Right", this, &ACar::MoveRight);
	PlayerInputComponent->BindAxis("Forward", this, &ACar::MoveForward);
	PlayerInputComponent->BindAxis("Throttle", this, &ACar::HandleThrottle);
	PlayerInputComponent->BindAction("Debug", IE_Released, this, &ACar::Debug);
}

//Input functions
void ACar::MoveRight(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACar::MoveForward(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACar::HandleThrottle(float AxisValue)
{
	Throttle = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void ACar::Debug()
{
	if (Body) {
		Body->SetVisibility(!Body->IsVisible());
		debugForces = !Body->IsVisible();
	}
}

