// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Car.h"
#include "ArcadeCar.generated.h"

UCLASS()
class CARPROTOTYPE_API AArcadeCar : public ACar
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AArcadeCar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		float Thrust;

	UPROPERTY(EditAnywhere, Category = VehicleSetup, meta = (AllowPrivateAccess = "true"))
		float TurnStrength;
};
