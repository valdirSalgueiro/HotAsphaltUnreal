// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

/**
 * 
 */
struct FWheelData
{
public:
	FWheelData(){}

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
