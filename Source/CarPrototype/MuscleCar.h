// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Axle.h"
#include "GameFramework/Pawn.h"
#include "MuscleCar.generated.h"

/**
 *
 */
UCLASS()
class CARPROTOTYPE_API AMuscleCar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMuscleCar();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = MuscleCar)
		TArray<FAxle> axles;

	/*Car body*/
	UPROPERTY(Category = MuscleCar, VisibleDefaultsOnly, BlueprintReadOnly)
		class UStaticMeshComponent* Body;

	/*Camera*/
	UPROPERTY(EditAnywhere, Category = "Camera")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	class USpringArmComponent* CameraSpringArm;

	UPROPERTY(EditAnywhere, Category = VehicleSetup)
		UStaticMeshComponent* Wheel_FL;
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
		UStaticMeshComponent* Wheel_FR;
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
		UStaticMeshComponent* Wheel_RL;
	UPROPERTY(EditAnywhere, Category = VehicleSetup)
		UStaticMeshComponent* Wheel_RR;

	UPROPERTY(EditAnywhere, Category = VehicleSetup)
	float pivotOffsetZ = 0;

#if WITH_EDITOR
	/// Clears and updates the instantiators.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR

private:

	//x - time in seconds
	//y - speed in km/h
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
		class UCurveFloat* accCurve;

	//x - time in seconds
	//y - speed in km/h
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
		class UCurveFloat* accReverseCurve;

	// x - speed in km/h
	// y - angle in degrees
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
		class UCurveFloat* steerAngleLimit;
	// x - speed in km/h
	// y - angle in degrees
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
		class UCurveFloat* steeringResetSpeed;
	// x - speed in km/h
	// y - angle in degrees
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
		class UCurveFloat* steeringSpeed;

	// x - speed in km/h
	// y - Downforce percentage
	//Y - Downforce (percentage 0%..100%). X - Vehicle speed (km/h)
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
		class UCurveFloat* downForceCurve;

	const int WHEEL_LEFT_INDEX = 0;
	const int WHEEL_RIGHT_INDEX = 1;

	const float wheelWidth = 8.5f;

	//Stabilization in flight (torque)
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
	float flightStabilizationForce = 6.0f;

	//Stabilization in flight (Ang velocity damping)
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
	float flightStabilizationDamping = 0.7f;

	//Hand brake slippery time in seconds
	UPROPERTY(EditAnywhere, Category = MuscleCar, meta = (AllowPrivateAccess = "true"))
	float handBrakeSlipperyTime = 4.0f;

	//Downforce
	float downForce = 10.0f;

	//////////////////////////////////////////////////////////////////////////////////////////////////////

	float afterFlightSlipperyTiresTime = 0.0f;
	float brakeSlipperyTiresTime = 0.0f;
	float handBrakeSlipperyTiresTime = 0.0f;
	bool isBrake = false;
	bool isHandBrake = false;
	bool isAcceleration = false;
	bool isReverseAcceleration = false;
	float accelerationForceMagnitude = 0.0f;	

	FVector2D MovementInput;

	float GetAccelerationForceMagnitude(bool isReverse, float speedMetersPerSec, float dt);
	float GetSpeed();
	float CalcAccelerationForceMagnitude(float dt);
	void CalculateAxleForces(FAxle* axle, int totalWheelsCount, int numberOfPoweredWheels, float dt);
	void CalculateWheelForces(FAxle* axle, FVector wsDownDirection, FWheelData* wheelData, FVector wsAttachPoint, int wheelIndex, int totalWheelsCount, int numberOfPoweredWheels, float dt);
	TTuple<FVector, FQuat> CalculateWheelVisualTransform(FVector wsAttachPoint, FVector wsDownDirection, FAxle* axle, FWheelData* data, int wheelIndex, float visualRotationRad);

	void UpdateVisual();

	void UpdateWheelVisual(FAxle* axle, const FVector &wsL, const FVector &wsDownDirection, UStaticMeshComponent* wheelVisual, int wheelIndex, FWheelData* wheelData);

	void CalculateAckermannSteering();

	float GetHandBrakeK();
	float GetSteeringHandBrakeK();

	float GetSteerAngleLimitInDeg(float speedMetersPerSec);

	void MoveRight(float AxisValue);
	void MoveForward(float AxisValue);
	void UpdateInput(float dt);
};
