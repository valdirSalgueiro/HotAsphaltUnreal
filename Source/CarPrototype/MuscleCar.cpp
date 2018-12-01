// Fill out your copyright notice in the Description page of Project Settings.

#include "MuscleCar.h"
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
AMuscleCar::AMuscleCar()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	RootComponent = RootComponent;

	CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	CameraSpringArm->TargetArmLength = 400;
	CameraSpringArm->SetRelativeLocation(FVector(0, 0, 200));
	CameraSpringArm->AttachTo(RootComponent);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	Camera->AttachTo(CameraSpringArm);

	Wheel_FL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_FL"));
	Wheel_FL->AttachTo(RootComponent);
	Wheel_FL->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Wheel_FR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_FR"));
	Wheel_FR->AttachTo(RootComponent);
	Wheel_FR->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Wheel_RL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_RL"));
	Wheel_RL->AttachTo(RootComponent);
	Wheel_RL->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Wheel_RR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Wheel_RR"));
	Wheel_RR->AttachTo(RootComponent);
	Wheel_RR->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AMuscleCar::BeginPlay()
{
	Super::BeginPlay();
	if (axles.Num() == 2 && !axles[0].wheelVisualLeft) {
		axles[0].wheelVisualLeft = Wheel_FL;
		axles[0].wheelVisualRight = Wheel_FR;
		axles[1].wheelVisualLeft = Wheel_RL;
		axles[1].wheelVisualRight = Wheel_RR;
	}
	Body->SetSimulatePhysics(true);
	Body->SetCenterOfMass(FVector(38, 0, -40));
}

float AMuscleCar::GetSteerAngleLimitInDeg(float speedMetersPerSec)
{
	float speedKmH = speedMetersPerSec * 3.6f;

	// maximum angle limit when hand brake is pressed
	speedKmH *= GetSteeringHandBrakeK();

	float limitDegrees = steerAngleLimit->GetFloatValue(speedKmH);

	return limitDegrees;
}


void AMuscleCar::UpdateInput(float dt)
{
	bool isBrakeNow = false;
	//bool isHandBrakeNow = Input.GetKey(KeyCode.Space) && controllable;
	bool isHandBrakeNow = false;

	float speed = GetSpeed();
	isAcceleration = false;
	isReverseAcceleration = false;
	if (MovementInput.Y > 0.4f)
	{
		if (speed < -0.5f)
		{
			isBrakeNow = true;
		}
		else
		{
			isAcceleration = true;
		}
	}
	else if (MovementInput.Y < -0.4f)
	{
		if (speed > 0.5f)
		{
			isBrakeNow = true;
		}
		else
		{
			isReverseAcceleration = true;
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("isbrake %d %d %f"), isBrake, isReverseAcceleration, speed);

	// Make tires more slippery (for 1 seconds) when player hit brakes
	if (isBrakeNow == true && isBrake == false)
	{
		brakeSlipperyTiresTime = 1.0f;
	}

	// slippery tires while handsbrakes are pressed
	if (isHandBrakeNow == true)
	{
		handBrakeSlipperyTiresTime = FMath::Max(0.1f, handBrakeSlipperyTime);
	}

	isBrake = isBrakeNow;
	

	// hand brake + acceleration = power slide
	isHandBrake = isHandBrakeNow && !isAcceleration && !isReverseAcceleration;

	axles[0].brakeLeft = isBrake;
	axles[0].brakeRight = isBrake;
	axles[1].brakeLeft = isBrake;
	axles[1].brakeRight = isBrake;

	axles[0].handBrakeLeft = isHandBrake;
	axles[0].handBrakeRight = isHandBrake;
	axles[1].handBrakeLeft = isHandBrake;
	axles[1].handBrakeRight = isHandBrake;


	//TODO: axles[0] always used for steering

	if (FMath::Abs(MovementInput.X) > 0.001f)
	{
		float speedKmH = FMath::Abs(speed) * 3.6f;

		// maximum steer speed when hand-brake is pressed
		speedKmH *= GetSteeringHandBrakeK();

		float steerSpeed = steeringSpeed->GetFloatValue(speedKmH);

		float newSteerAngle = axles[0].steerAngle + (MovementInput.X * steerSpeed);
		float sgn = FMath::Sign(newSteerAngle);

		float steerLimit = GetSteerAngleLimitInDeg(speed);

		newSteerAngle = FMath::Min(FMath::Abs(newSteerAngle), steerLimit) * sgn;

		axles[0].steerAngle = newSteerAngle;
	}
	else
	{
		float speedKmH = FMath::Abs(speed) * 3.6f;

		float angleReturnSpeedDegPerSec = steeringResetSpeed->GetFloatValue(speedKmH);

		angleReturnSpeedDegPerSec = FMath::Lerp(0.0f, angleReturnSpeedDegPerSec, FMath::Clamp(speedKmH / 2.0f, 0.f, 1.f));

		float ang = axles[0].steerAngle;
		float sgn = FMath::Sign(ang);

		ang = FMath::Abs(ang);

		ang -= angleReturnSpeedDegPerSec * dt;
		ang = FMath::Max(ang, 0.0f) * sgn;

		axles[0].steerAngle = ang;
	}
}

float AMuscleCar::GetSteeringHandBrakeK()
{
	// 0.4 - pressed
	// 1.0 - not pressed
	float steeringK = FMath::Clamp(0.4f + (1.0f - GetHandBrakeK()) * 0.6f, 0.f, 1.f);
	return steeringK;
}

// Called every frame
void AMuscleCar::Tick(float DeltaTime)
{
	APawn::Tick(DeltaTime);

	UpdateInput(DeltaTime);

	accelerationForceMagnitude = CalcAccelerationForceMagnitude(DeltaTime);
	//UE_LOG(LogTemp, Warning, TEXT("acc %d %f"), isAcceleration, accelerationForceMagnitude);

	// 0.8 - pressed
	// 1.0 - not pressed
	float accelerationK = FMath::Clamp(0.8f + (1.0f - GetHandBrakeK()) * 0.2f, 0.f, 1.f);
	accelerationForceMagnitude *= accelerationK;

	CalculateAckermannSteering();

	int numberOfPoweredWheels = 0;
	for (int axleIndex = 0; axleIndex < axles.Num(); axleIndex++)
	{
		if (axles[axleIndex].isPowered)
		{
			numberOfPoweredWheels += 2;
		}
	}

	int totalWheelsCount = axles.Num() * 2;
	for (int axleIndex = 0; axleIndex < axles.Num(); axleIndex++)
	{
		CalculateAxleForces(&axles[axleIndex], totalWheelsCount, numberOfPoweredWheels, DeltaTime);
	}

	bool allWheelIsOnAir = true;
	for (int axleIndex = 0; axleIndex < axles.Num(); axleIndex++)
	{
		if (axles[axleIndex].wheelDataL.isOnGround || axles[axleIndex].wheelDataR.isOnGround)
		{
			allWheelIsOnAir = false;
			break;
		}
	}

	if (allWheelIsOnAir)
	{
		// set after flight tire slippery time (1 sec)
		afterFlightSlipperyTiresTime = 1.0f;

		// Try to keep vehicle parallel to the ground while jumping
		FVector carUp = UKismetMathLibrary::TransformDirection(GetActorTransform(), FVector(0.0f, 0.0f, 1.0f));

		// Flight stabilization from
		// https://github.com/supertuxkart/stk-code/blob/master/src/physics/btKart.cpp#L455

		// Length of axis depends on the angle - i.e. the further awat
		// the kart is from being upright, the larger the applied impulse
		// will be, resulting in fast changes when the kart is on its
		// side, but not overcompensating (and therefore shaking) when
		// the kart is not much away from being upright.
		FVector axis = FVector::CrossProduct(carUp, FVector::UpVector);
		//axis.Normalize ();

		float mass = Body->GetMass();
		// angular velocity damping
		FVector angVel = Body->GetPhysicsAngularVelocityInRadians();

		FVector angVelDamping = angVel;
		angVelDamping.Y = 0.0f;
		angVelDamping = angVelDamping * FMath::Clamp(flightStabilizationDamping * DeltaTime, 0.f, 1.f);

		//UE_LOG(LogTemp, Warning, TEXT("trying to stabilize Ang %s Damping %s"), *angVel.ToCompactString(), *angVelDamping.ToCompactString());
		Body->SetPhysicsAngularVelocityInRadians((angVel - angVelDamping));

		// in flight roll stabilization
		Body->AddTorque(axis * flightStabilizationForce * mass * 100);
	}
	else
	{
		// downforce
		FVector carDown = -Body->GetUpVector();

		float speed = GetSpeed();
		float speedKmH = FMath::Abs(speed) * 3.6f;

		//float downForceAmount = downForceCurve->GetFloatValue(speedKmH) / 100.0f; //correct?
		float downForceAmount = downForceCurve->GetFloatValue(speedKmH);
		float mass = Body->GetMass();

		//UE_LOG(LogTemp, Warning, TEXT("downForceAmount * downForce %f %f %s"), speedKmH, downForceAmount * downForce, *(carDown * mass * downForceAmount * downForce).ToCompactString());
		Body->AddForce(carDown * mass * downForceAmount * downForce);
	}

	UpdateVisual();

	if (afterFlightSlipperyTiresTime > 0.0f)
	{
		afterFlightSlipperyTiresTime -= DeltaTime;
	}
	else
	{
		afterFlightSlipperyTiresTime = 0.0f;
	}

	if (brakeSlipperyTiresTime > 0.0f)
	{
		brakeSlipperyTiresTime -= DeltaTime;
	}
	else
	{
		brakeSlipperyTiresTime = 0.0f;
	}

	if (handBrakeSlipperyTiresTime > 0.0f)
	{
		handBrakeSlipperyTiresTime -= DeltaTime;
	}
	else
	{
		handBrakeSlipperyTiresTime = 0.0f;
	}
}

void AMuscleCar::CalculateAxleForces(FAxle* Axle, int totalWheelsCount, int numberOfPoweredWheels, float dt)
{
	FVector wsDownDirection = -Body->GetUpVector();
	//FVector wsDownDirection = GetTransform().TransformVector(-FVector::UpVector);
	wsDownDirection.Normalize();

	FVector localL = FVector(Axle->offset.X, Axle->width * -0.5f, Axle->offset.Y);
	FVector localR = FVector(Axle->offset.X, Axle->width * 0.5f, Axle->offset.Y);

	FVector wsL = UKismetMathLibrary::TransformLocation(GetActorTransform(), localL);
	FVector wsR = UKismetMathLibrary::TransformLocation(GetActorTransform(), localR);


	//UE_LOG(LogTemp, Warning, TEXT("wsL %s"), *wsL.ToCompactString());
	//UE_LOG(LogTemp, Warning, TEXT("wsR %s"), *wsR.ToCompactString());
	CalculateWheelForces(Axle, wsDownDirection, &Axle->wheelDataL, wsL, 0, totalWheelsCount, numberOfPoweredWheels, dt);
	CalculateWheelForces(Axle, wsDownDirection, &Axle->wheelDataR, wsR, 1, totalWheelsCount, numberOfPoweredWheels, dt);


	// http://projects.edy.es/trac/edy_vehicle-physics/wiki/TheStabilizerBars
	// Apply "stablizier bar" forces
	float travelL = 1.0f - FMath::Clamp(Axle->wheelDataL.compression, 0.f, 1.f);
	float travelR = 1.0f - FMath::Clamp(Axle->wheelDataR.compression, 0.f, 1.f);

	float antiRollForce = (travelL - travelR) * Axle->antiRollForce;
	//UE_LOG(LogTemp, Warning, TEXT("Axle->wheelDataL.isOnGround Axle->wheelDataR.isOnGround %d %d"), Axle->wheelDataL.isOnGround, Axle->wheelDataR.isOnGround);
	if (Axle->wheelDataL.isOnGround)
	{
		//::DrawDebugLine(GetWorld(), Axle->wheelDataL.touchPoint.Location, Axle->wheelDataL.touchPoint.Location + wsDownDirection * antiRollForce * 100, FColor::Blue, false, 0.0f, 0, 3.f);
		Body->AddForceAtLocation(wsDownDirection * antiRollForce * 200, Axle->wheelDataL.touchPoint.Location);
	}

	if (Axle->wheelDataR.isOnGround)
	{
		//::DrawDebugLine(GetWorld(), Axle->wheelDataR.touchPoint.Location, Axle->wheelDataR.touchPoint.Location + wsDownDirection * -antiRollForce * 100, FColor::Blue, false, 0.0f, 0, 3.f);
		Body->AddForceAtLocation(wsDownDirection * -antiRollForce * 200, Axle->wheelDataR.touchPoint.Location);
	}
}

void AMuscleCar::CalculateWheelForces(FAxle* axle, FVector wsDownDirection, FWheelData* wheelData, FVector wsAttachPoint, int wheelIndex, int totalWheelsCount, int numberOfPoweredWheels, float dt)
{
	FHitResult s1;
	FHitResult s2;
	FCollisionQueryParams TraceParams(true);
	TraceParams.AddIgnoredActor(this);

	FVector start;
	FVector end;

	FQuat localWheelRot = FQuat::MakeFromEuler(FVector(0.0f, 0.0, FMath::RadiansToDegrees(wheelData->yawRad)));
	FQuat wsWheelRot = GetActorRotation().Quaternion() * localWheelRot;

	FVector wsAxleLeft = wsWheelRot * -FVector::RightVector;
	wheelData->isOnGround = false;

	float rayLength = axle->radius + axle->lengthRelaxed;

	start = wsAttachPoint + wsAxleLeft * wheelWidth;
	end = start + wsDownDirection * rayLength;
	bool b1 = GetWorld()->LineTraceSingleByObjectType(s1, start, end, ECC_WorldDynamic | ECC_WorldStatic, TraceParams);

	start = wsAttachPoint - wsAxleLeft * wheelWidth;
	end = start + wsDownDirection * rayLength;
	bool b2 = GetWorld()->LineTraceSingleByObjectType(s2, start, end, ECC_WorldDynamic | ECC_WorldStatic, TraceParams);

	start = wsAttachPoint;
	end = start + wsDownDirection * rayLength;

	bool isCollided = GetWorld()->LineTraceSingleByObjectType(wheelData->touchPoint, start, end, ECC_WorldDynamic | ECC_WorldStatic, TraceParams);

	//::DrawDebugLine(GetWorld(), wsAttachPoint, wsAttachPoint + wsDownDirection * rayLength, FColor::Green, false, 0.0f, 0, 1.f);
	//::DrawDebugLine(GetWorld(), wsAttachPoint + wsAxleLeft * wheelWidth, wsAttachPoint + wsAxleLeft * wheelWidth + FVector(0,0,-rayLength), FColor::Blue, false, 0.0f, 0, 1.f);
	//::DrawDebugLine(GetWorld(), wsAttachPoint - wsAxleLeft * wheelWidth, wsAttachPoint - wsAxleLeft * wheelWidth + wsDownDirection * rayLength, FColor::Red, false, 0.0f, 0, 1.f);

	//UE_LOG(LogTemp, Warning, TEXT("isCollided %d %d %d"), isCollided, b1, b2);

	// No wheel contant found
	if (!isCollided || !b1 || !b2)
	{
		// wheel do not touch the ground (relaxing spring)
		float relaxSpeed = 1.0f;
		wheelData->compressionPrev = wheelData->compression;
		wheelData->compression = FMath::Clamp(wheelData->compression - dt * relaxSpeed, 0.f, 1.f);
		return;
	}

	// Consider wheel radius
	float suspLenNow = wheelData->touchPoint.Distance - axle->radius;

	wheelData->isOnGround = true;

	float suspForceMag = 0.0f;

	// Positive value means that the spring is compressed
	// Negative value means that the spring is elongated.

	wheelData->compression = 1.0f - FMath::Clamp(suspLenNow / axle->lengthRelaxed, 0.f, 1.f);

	// Hooke's law (springs)
	// F = -k x 

	// Spring force (try to reset compression from spring)
	float springForce = wheelData->compression * -axle->stiffness;
	suspForceMag += springForce;

	// Damping force (try to reset velocity to 0)
	float suspCompressionVelocity = (wheelData->compression - wheelData->compressionPrev) / dt;
	wheelData->compressionPrev = wheelData->compression;

	float damperForce = -suspCompressionVelocity * axle->damping;
	suspForceMag += damperForce;

	// Only consider component of force that is along the contact normal.
	float denom = FVector::DotProduct(wheelData->touchPoint.Normal, -wsDownDirection);
	suspForceMag *= denom;

	// Apply suspension force
	FVector suspForce = wsDownDirection * suspForceMag;
	//UE_LOG(LogTemp, Warning, TEXT("suspForceMag %f suspLenNow %f wheelData.compression %f %s"), suspForceMag, suspLenNow, wheelData->compression, *(wheelData->touchPoint.Normal.ToCompactString()));
	Body->AddForceAtLocation(suspForce * 100, wheelData->touchPoint.Location);

	//
	// Calculate friction forces
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// 

	FVector wheelVelocity = Body->GetPhysicsLinearVelocityAtPoint(wheelData->touchPoint.Location);

	//// Contact basis (can be different from wheel basis)
	FVector c_up = wheelData->touchPoint.Normal;
	FVector c_left = (s1.Location - s2.Location).GetSafeNormal();
	FVector c_fwd = FVector::CrossProduct(c_up, c_left);

	//// Calculate sliding velocity (velocity without normal force)
	FVector lvel = FVector::DotProduct(wheelVelocity, c_left) * c_left;
	FVector fvel = FVector::DotProduct(wheelVelocity, c_fwd) * c_fwd;
	FVector slideVelocity = (lvel + fvel) * 0.5f;

	// Calculate current sliding force
	FVector slidingForce = (slideVelocity * Body->GetMass() / dt) / (float)totalWheelsCount;

	float laterialFriction = FMath::Clamp(axle->laterialFriction, 0.f, 1.f);

	float slipperyK = 1.0f;

	//Simulate slippery tires
	if (afterFlightSlipperyTiresTime > 0.0f)
	{
		float slippery = FMath::Lerp(1.0f, axle->afterFlightSlipperyK, FMath::Clamp(afterFlightSlipperyTiresTime, 0.f, 1.f));
		slipperyK = FMath::Min(slipperyK, slippery);
	}

	if (brakeSlipperyTiresTime > 0.0f)
	{
		float slippery = FMath::Lerp(1.0f, axle->brakeSlipperyK, FMath::Clamp(brakeSlipperyTiresTime, 0.f, 1.f));
		slipperyK = FMath::Min(slipperyK, slippery);
	}

	float handBrakeK = GetHandBrakeK();
	if (handBrakeK > 0.0f)
	{
		float slippery = FMath::Lerp(1.0f, axle->handBrakeSlipperyK, handBrakeK);
		slipperyK = FMath::Min(slipperyK, slippery);
	}

	laterialFriction = laterialFriction * slipperyK;

	// Simulate perfect static friction
	FVector frictionForce = -slidingForce * laterialFriction;

	// Remove friction along roll-direction of wheel 
	FVector longitudinalForce = FVector::DotProduct(frictionForce, c_fwd) * c_fwd;

	// Apply braking force or rolling resistance force or nothing
	bool isBrakeEnabled = (wheelIndex == WHEEL_LEFT_INDEX) ? axle->brakeLeft : axle->brakeRight;
	bool isHandBrakeEnabled = (wheelIndex == WHEEL_LEFT_INDEX) ? axle->handBrakeLeft : axle->handBrakeRight;
	if (isBrakeEnabled || isHandBrakeEnabled)
	{
		float clampedMag = FMath::Clamp(axle->brakeForceMag * Body->GetMass(), 0.0f, longitudinalForce.Size()) * 60;
		//UE_LOG(LogTemp, Warning, TEXT("clampedMag %f"), clampedMag);

		FVector brakeForce = longitudinalForce.GetSafeNormal() * clampedMag;
		//UE_LOG(LogTemp, Warning, TEXT("brakeForce %s"), *brakeForce.ToCompactString());

		if (isHandBrakeEnabled)
		{
			// hand brake are not powerful enough ;)
			brakeForce = brakeForce * 0.8f;
		}

		longitudinalForce -= brakeForce;
	}
	else
	{
		// Apply rolling-friction (automatic slow-down) only if player don't press to the accelerator
		if (!isAcceleration && !isReverseAcceleration)
		{
			float rollingK = 1.0f - FMath::Clamp(axle->rollingFriction, 0.f, 1.f);
			longitudinalForce *= rollingK;
		}
	}

	frictionForce -= longitudinalForce;
	//UE_LOG(LogTemp, Warning, TEXT("frictionForce %s %s"), *frictionForce.ToCompactString(), *wheelData->touchPoint.Location.ToCompactString());
	//::DrawDebugLine(GetWorld(), wheelData->touchPoint.Location, wheelData->touchPoint.Location + frictionForce.GetSafeNormal() * 500, FColor::Green, false, 0.0f, 0, 3.f);

	// Apply resulting force
	Body->AddForceAtLocation(frictionForce, wheelData->touchPoint.Location);

	//UE_LOG(LogTemp, Warning, TEXT("isbrake %d axle->isPowered %d %f"), isBrake, axle->isPowered, accelerationForceMagnitude);
	// Engine force
	if (!isBrake && axle->isPowered && FMath::Abs(accelerationForceMagnitude) > 0.01f)
	{
		FVector accForcePoint = wheelData->touchPoint.Location - (wsDownDirection * 0.2f);
		FVector engineForce = c_fwd * accelerationForceMagnitude / (float)numberOfPoweredWheels / dt;
		//::DrawDebugLine(GetWorld(), accForcePoint, accForcePoint + engineForce * 100, FColor::Blue, false, 0.0f, 0, 3.f);
		Body->AddForceAtLocation(engineForce * 100, accForcePoint);
		//UE_LOG(LogTemp, Warning, TEXT("engineforce %s"), *engineForce.ToCompactString());
	}
}

float AMuscleCar::GetAccelerationForceMagnitude(bool isReverse, float speedMetersPerSec, float dt)
{
	float speedKmH = speedMetersPerSec * 3.6f;

	float mass = Body->GetMass();

	FRichCurve& Curve = accCurve->FloatCurve;
	if(isReverse)
		Curve = accReverseCurve->FloatCurve;

	int numKeys = Curve.Keys.Num();

	////brute-force linear search
	float minTime = Curve.Keys[0].Time;
	float maxTime = Curve.Keys[numKeys - 1].Time;

	float step = (maxTime - minTime) / 500.0f;
	float prev_speed = FMath::Min(Curve.Keys[0].Value, speedKmH);

	float timeNow = 0.0f;
	bool isResultFound = false;

	for (float t = minTime; t <= maxTime; t += step)
	{
		float cur_speed = accCurve->GetFloatValue(t);

		if (speedKmH >= prev_speed && speedKmH < cur_speed)
		{
			//found the right value
			isResultFound = true;
			timeNow = t;
		}

		prev_speed = cur_speed;
	}

	if (isResultFound)
	{
		float speed_desired = accCurve->GetFloatValue(timeNow + dt);

		float acc = (speed_desired - speedKmH);
		//to meters per sec
		acc /= 3.6f;
		float forceMag = (acc * mass);
		forceMag = FMath::Max(forceMag, 0.0f);
		return forceMag;
	}

	float _desiredSpeed = Curve.Keys[numKeys - 1].Value;
	float _acc = (_desiredSpeed - speedKmH);
	//to meters per sec
	_acc /= 3.6f;
	float _forceMag = (_acc * mass);
	_forceMag = FMath::Max(_forceMag, 0.0f);
	return _forceMag;
}

float AMuscleCar::GetSpeed()
{
	FVector velocity = GetVelocity();
	FVector wsForward = GetActorForwardVector();
	float vProj = FVector::DotProduct(velocity, wsForward);
	FVector projVelocity = vProj * wsForward;
	float speed = projVelocity.Size() * FMath::Sign(vProj) / 100.f;
	return speed;
}

//TODO: Refactor (remove this func, GetAccelerationForceMagnitude is enough)
float AMuscleCar::CalcAccelerationForceMagnitude(float dt)
{
	if (!isAcceleration && !isReverseAcceleration)
	{
		return 0.0f;
	}

	float speed = GetSpeed();
	//UE_LOG(LogTemp, Warning, TEXT("CalcAccelerationForceMagnitude speed %f"), speed);

	if (isAcceleration)
	{
		float forceMag = GetAccelerationForceMagnitude(false, speed, dt);
		return forceMag;
	}
	else
	{
		float forceMag = GetAccelerationForceMagnitude(true, -speed, dt);
		return -forceMag;
	}

}

float AMuscleCar::GetHandBrakeK()
{
	float x = handBrakeSlipperyTiresTime / FMath::Max(0.1f, handBrakeSlipperyTime);
	// smoother step
	x = x * x * x * (x * (x * 6 - 15) + 10);
	return x;
}

void AMuscleCar::CalculateAckermannSteering()
{
	// Copy desired steering
	for (int axleIndex = 0; axleIndex < axles.Num(); axleIndex++)
	{
		float steerAngleRad = FMath::DegreesToRadians(axles[axleIndex].steerAngle);

		axles[axleIndex].wheelDataL.yawRad = steerAngleRad;
		axles[axleIndex].wheelDataR.yawRad = steerAngleRad;
	}

	FAxle frontAxle = axles[0];
	FAxle rearAxle = axles[1];

	// Calculate our chassis (remove scale)
	FVector axleDiff = GetTransform().TransformPosition(FVector(frontAxle.offset.X, 0, frontAxle.offset.Y)) - GetTransform().TransformPosition(FVector(rearAxle.offset.X, 0, rearAxle.offset.Y));
	float axleseparation = axleDiff.Size();

	FVector wheelDiff = GetTransform().TransformPosition(FVector(frontAxle.offset.X, frontAxle.width * -0.5f, frontAxle.offset.Y)) - GetTransform().TransformPosition(FVector(frontAxle.offset.X, frontAxle.width * 0.5f, frontAxle.offset.Y));
	float wheelsSeparation = wheelDiff.Size();

	//UE_LOG(LogTemp, Warning, TEXT("frontAxle.steerAngle %f wheelsSeparation %f axleseparation %f"), frontAxle.steerAngle, wheelsSeparation, axleseparation);

	if (FMath::Abs(frontAxle.steerAngle) < 0.001f)
	{
		// Sterring wheels are not turned
		return;
	}

	// Simple right-angled triangle math (find cathet if we know another cathet and angle)

	// Find cathet from cathet and angle
	//
	// axleseparation - is first cathet
	// steerAngle - is angle between cathet and hypotenuse
	float rotationCenterOffsetL = axleseparation / FMath::Tan(FMath::DegreesToRadians(frontAxle.steerAngle));

	// Now we have another 2 cathet's (rotationCenterOffsetR and axleseparation for second wheel)
	//  need to find right angle 
	float rotationCenterOffsetR = rotationCenterOffsetL - wheelsSeparation;

	float rightWheelYaw = FMath::Atan(axleseparation / rotationCenterOffsetR);

	frontAxle.wheelDataR.yawRad = rightWheelYaw;
}

TTuple<FVector, FQuat> AMuscleCar::CalculateWheelVisualTransform(FVector wsAttachPoint, FVector wsDownDirection, FAxle* axle, FWheelData* data, int wheelIndex, float visualRotationRad)
{
	float suspCurrentLen = FMath::Clamp(1.0f - data->compression, 0.f, 1.f) * axle->lengthRelaxed;
	//UE_LOG(LogTemp, Warning, TEXT("susp %f %f %f"), suspCurrentLen, data->compression, axle->lengthRelaxed);

	FVector pos = wsAttachPoint + wsDownDirection * suspCurrentLen;
	//UE_LOG(LogTemp, Warning, TEXT("susp pos %s"), *pos.ToCompactString());

	float additionalYaw = 0.0f;
	float additionalMul = 1;
	if (wheelIndex == WHEEL_LEFT_INDEX)
	{
		additionalYaw = 180.0f;
		additionalMul = -1;
	}

	FQuat localWheelRot = FQuat::MakeFromEuler(FVector(FMath::RadiansToDegrees(data->visualRotationRad * additionalMul), additionalYaw + FMath::RadiansToDegrees(data->yawRad), 0.0f));
	return MakeTuple(pos, localWheelRot);
}

void AMuscleCar::UpdateVisual()
{
	//FVector wsDownDirection = transform.TransformVector(-FVector::UpVector);
	//wsDownDirection.Normalize();
	FVector wsDownDirection = -Body->GetUpVector();
	wsDownDirection.Normalize();

	for (int axleIndex = 0; axleIndex < axles.Num(); axleIndex++)
	{
		FAxle axle = axles[axleIndex];

		FVector localL = FVector(axle.offset.X, axle.width * -0.5f, axle.offset.Y);
		FVector localR = FVector(axle.offset.X, axle.width * 0.5f, axle.offset.Y);

		FVector wsL = UKismetMathLibrary::TransformLocation(GetActorTransform(), localL);
		FVector wsR = UKismetMathLibrary::TransformLocation(GetActorTransform(), localR);

		UpdateWheelVisual(&axle, wsL, wsDownDirection, axle.wheelVisualLeft, WHEEL_LEFT_INDEX, &axle.wheelDataL);
		UpdateWheelVisual(&axle, wsR, wsDownDirection, axle.wheelVisualRight, WHEEL_RIGHT_INDEX, &axle.wheelDataR);
	}
}

void AMuscleCar::UpdateWheelVisual(FAxle* axle, const FVector &wsL, const FVector &wsDownDirection, UStaticMeshComponent* wheelVisual, int wheelIndex, FWheelData* wheelData)
{
	if (wheelVisual != NULL)
	{
		auto tuple = CalculateWheelVisualTransform(wsL, wsDownDirection, axle, wheelData, wheelIndex, wheelData->visualRotationRad);
		wheelVisual->SetWorldLocation(tuple.Key);
		wheelVisual->SetRelativeRotation(tuple.Value);
		wheelVisual->SetWorldScale3D(FVector(axle->visualScale));

		if (!isBrake)
		{
			//CalculateWheelRotationFromSpeed(axle, axle.wheelDataL, wsPos);
		}
	}
}

void AMuscleCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Right", this, &AMuscleCar::MoveRight);
	PlayerInputComponent->BindAxis("Forward", this, &AMuscleCar::MoveForward);
}
void AMuscleCar::MoveRight(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}
void AMuscleCar::MoveForward(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

#if WITH_EDITOR
void AMuscleCar::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	//hack
	if (PropertyChangedEvent.Property)
	{
		if (axles.Num() == 2 && !axles[0].wheelVisualLeft) {
			axles[0].wheelVisualLeft = Wheel_FL;
			axles[0].wheelVisualRight = Wheel_FR;
			axles[1].wheelVisualLeft = Wheel_RL;
			axles[1].wheelVisualRight = Wheel_RR;
			axles[0].wheelVisualLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			axles[0].wheelVisualRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			axles[1].wheelVisualLeft->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			axles[1].wheelVisualRight->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		UpdateVisual();
	}
}
#endif // WITH_EDITOR