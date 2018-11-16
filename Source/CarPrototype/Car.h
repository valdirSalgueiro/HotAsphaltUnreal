// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <vector>
#include "Car.generated.h"

USTRUCT()
struct FSuspensionStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float restLength;
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float travel;
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float stiffness;
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float damper;
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float forceMin;
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float forceMax;
};

USTRUCT()
struct FWheelStruct
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float radius;
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		float mass;
};

UCLASS()
class CARPROTOTYPE_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/*Car body*/
	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* Body;
	
	/*Camera*/
	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Player", meta = (AllowPrivateAccess = "true"))
		class USpringArmComponent* CameraSpringArm;

	/*Suspension structs*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		TArray<FWheelStruct> Wheels;

	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		TArray<FSuspensionStruct> SuspensionArray;
	
	/*Transmission*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		TArray<float> GearRatio;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		TArray<float> DriveTorque;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		TArray<float> TorqueRatio;
		
	/*Car Scene Components*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		USceneComponent* TopLink_FL;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		USceneComponent* TopLink_FR;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		USceneComponent* TopLink_RL;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		USceneComponent* TopLink_RR;
	UPROPERTY(BlueprintReadOnly, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		TArray<USceneComponent*> ToplinkComponents;

	/*Car Mesh Components*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Wheel_FL;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Wheel_FR;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Wheel_RL;
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* Wheel_RR;

	/*Toque Curve*/
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		UCurveFloat* EngineCurve;

	/*Input methods*/
	void MoveRight(float AxisValue);
	void MoveForward(float AxisValue);
	void HandleThrottle(float AxisValue);

	/*Transmission methods*/
	void GearUp();
	void GearDown();
	
	void Debug();

	std::vector<UStaticMeshComponent*> WheelComponents;

	/*Input*/
	FVector2D MovementInput;

	/*Suspension*/
	std::vector<float> Length;
	std::vector<float> LastLength;
	std::vector<float> Raylength;

	/*Steer*/
	float steeringAngle;

	/*Torque*/
	float Throttle;
	float ThrottleValue;

	int idleRPM;
	int maxRPM;
	float inertia;
	float backTorque;

	float engineRPM;
	float engineAngularVelocity;

	/*Transmission*/
	std::vector<float> wheelInertia;
	std::vector<float> wheelAngularVelocity;
	int gear;
	int gearTarget;
	float mainGear;
	float totalGearRatio; //mainGear*gear
	float efficiency;
	float gearChangeTime;

	const float RPM_TO_RADPS = PI * 2.f / 60.f;
	const float RADPS_TO_RPM = 1.f/ RPM_TO_RADPS;

	bool debugForces;
};
