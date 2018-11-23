// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Axle.generated.h"

struct FWheelData
{
public:
	FWheelData() {}

	// is wheel touched ground or not ?
	bool isOnGround = false;

	// wheel ground touch point
	//RaycastHit touchPoint = new RaycastHit();

	// real yaw, after Ackermann steering correction
	float yawRad = 0.0f;

	// visual rotation
	float visualRotationRad = 0.0f;

	// suspension compression
	float compression = 0.0f;

	// suspension compression on previous update
	float compressionPrev = 0.0f;

	FHitResult touchPoint;
};

/**
*
*/
USTRUCT()
struct FAxle
{
	GENERATED_BODY()
public:
	FWheelData wheelDataL;
	FWheelData wheelDataR;

	//Axle width
	UPROPERTY(EditAnywhere)
		float width = 155.0f;

	//Axle offset
	UPROPERTY(EditAnywhere)
		FVector2D offset = FVector2D(150, 0);

	//Current steering angle (in degrees)
	UPROPERTY(EditAnywhere)
		float steerAngle = 0.0f;

	//Wheel radius in meters
	UPROPERTY(EditAnywhere)
		float radius = 30.0f;

	//Tire laterial friction normalized to 0..1
	UPROPERTY(EditAnywhere)
		float laterialFriction = 0.6f;

	//Rolling friction, normalized to 0..1
	UPROPERTY(EditAnywhere)
		float rollingFriction = 0.03f;

	//Brake left
	bool brakeLeft = false;

	//Brake right
	bool brakeRight = false;

	//Hand brake left
	bool handBrakeLeft = false;

	//Hand brake right
	bool handBrakeRight = false;

	//Brake force magnitude]
	UPROPERTY(EditAnywhere)
		float brakeForceMag = 14.0f;

	//Suspension Stiffness (Suspension 'Power'
	UPROPERTY(EditAnywhere)
		float stiffness = 15000.0f;

	//Suspension Damping (Suspension 'Bounce')
	UPROPERTY(EditAnywhere)
		float damping = 3000.0f;

	//Suspension Restitution (Not used now)
	UPROPERTY(EditAnywhere)
		float restitution = 1.0f;


	//Relaxed suspension length
	UPROPERTY(EditAnywhere)
		float lengthRelaxed = 45.f;

	//Stabilizer bar anti-roll force
	UPROPERTY(EditAnywhere)
		float antiRollForce = 10000.0f;

	//Visual scale for wheels
	UPROPERTY(EditAnywhere)
		float visualScale = 0.87f;

	UStaticMeshComponent* wheelVisualLeft;
	UStaticMeshComponent* wheelVisualRight;

	//Is axle powered by engine
	UPROPERTY(EditAnywhere)
		bool isPowered = false;

	//After flight slippery coefficent (0 - no friction)
	UPROPERTY(EditAnywhere)
		float afterFlightSlipperyK = 0.02f;

	//Brake slippery coefficent (0 - no friction)
	UPROPERTY(EditAnywhere)
		float brakeSlipperyK = 0.5f;

	//Hand brake slippery coefficent (0 - no friction)
	UPROPERTY(EditAnywhere)
		float handBrakeSlipperyK = 0.16f;
};
