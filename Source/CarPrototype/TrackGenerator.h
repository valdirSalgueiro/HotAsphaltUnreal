// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackGenerator.generated.h"

UCLASS()
class CARPROTOTYPE_API ATrackGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATrackGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void CreateSplinePoint();

	void AddSplinePoint();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void OnConstruction(const FTransform& Transform) override;
private:
	class USplineComponent* Spline;

	UPROPERTY(EditAnywhere, Category = Track, meta = (AllowPrivateAccess = "true"))
	class UStaticMesh* Mesh;

	FVector lastPoint;
	float angleH;
	float angleV;
	int angleHDest;
	int angleVDest;

	void GetHorizontalAngle();
	void GetVerticalAngle();
	int GetNewRandomAngle();

	int GetNewSharpAngle();

	int GetVerySharpAngle();

	void Initialize();
};
