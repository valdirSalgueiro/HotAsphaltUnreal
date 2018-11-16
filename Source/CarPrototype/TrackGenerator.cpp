// Fill out your copyright notice in the Description page of Project Settings.

#include "TrackGenerator.h"
#include "Runtime/Engine/Classes/Components/SplineComponent.h"
#include "Runtime/Engine/Classes/Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Components/StaticMeshComponent.h"

// Sets default values
ATrackGenerator::ATrackGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	Spline->SetMobility(EComponentMobility::Type::Static);
	RootComponent = Spline;
}

// Called every frame
void ATrackGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
// Called when the game starts or when spawned
void ATrackGenerator::BeginPlay()
{
	Super::BeginPlay();

}

void ATrackGenerator::Initialize()
{
	SetActorLocation(FVector::ZeroVector);
	SetActorRotation(FQuat::Identity);
	FVector locStart;
	FVector tanStart;
	FVector locEnd;
	FVector tanEnd;

	Spline->ClearSplinePoints();
	lastPoint = FVector::ZeroVector;


	angleH = 0;
	angleV = 0;
	angleHDest = 0;
	angleVDest = 0;

	//start line
	for (int i = 0; i < 10; i++)
	{
		AddSplinePoint();
	}

	for (int i = 0; i < 300; i++)
	{
		CreateSplinePoint();
	}

	for (int i = 0; i < 310; i++)
	{
		USplineMeshComponent *splineMesh = NewObject<USplineMeshComponent>(this);
		splineMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		splineMesh->RegisterComponent();
		splineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		splineMesh->SetMobility(EComponentMobility::Type::Static);
		splineMesh->SetForwardAxis(ESplineMeshAxis::X);
		splineMesh->SetStaticMesh(Mesh);
		splineMesh->AttachTo(Spline);
		//splineMesh->AttachToComponent(Spline, FAttachmentTransformRules::FAttachmentTransformRules(EAttachmentRule::KeepRelative, true));

		Spline->GetLocationAndTangentAtSplinePoint(i, locStart, tanStart, ESplineCoordinateSpace::Local);
		Spline->GetLocationAndTangentAtSplinePoint(i + 1, locEnd, tanEnd, ESplineCoordinateSpace::Local);
		splineMesh->SetStartAndEnd(locStart, tanStart, locEnd, tanEnd);
	}
}

void ATrackGenerator::OnConstruction(const FTransform& Transform) {
	Super::OnConstruction(Transform);
	Initialize();
}

void ATrackGenerator::CreateSplinePoint()
{
	float angleHVar = 5.f;
	float angleVVar = 5.f;
	//UE_LOG(LogTemp, Warning, TEXT("angleH %f %d"), angleH, angleHDest);

	if (angleH < angleHDest) {
		angleH += angleHVar;
	}
	else if (angleH > angleHDest) {
		angleH -= angleHVar;
	}
	else
	{
		GetHorizontalAngle();
	}

	if (angleV < angleVDest) {
		angleV += angleVVar;
	}
	else if (angleV > angleVDest) {
		angleV -= angleVVar;
	}
	else
	{
		GetVerticalAngle();
	}
	AddSplinePoint();
}

void ATrackGenerator::AddSplinePoint()
{
	float newX = FMath::Cos(FMath::DegreesToRadians(angleH)) * 600.f;
	float newY = FMath::Sin(FMath::DegreesToRadians(angleH)) * 600.f;
	float newZ = FMath::Sin(FMath::DegreesToRadians(angleV)) * 600.f;
	newZ = 0;
	lastPoint += FVector(newX, newY, newZ);
	Spline->AddSplinePoint(lastPoint, ESplineCoordinateSpace::World);
}

void ATrackGenerator::GetHorizontalAngle()
{
	int randomDirection;
	//angleHDest = GetNewRandomAngle();
	angleHDest = GetVerySharpAngle();
	if (lastPoint.Y < 0 && angleHDest < 0) {
		angleHDest = -angleHDest;
	}
	else if (lastPoint.Y > 500 && angleHDest > 0) {
		angleHDest = -angleHDest;
	}
	else if (lastPoint.Y > 0 && lastPoint.Y < 500) {
		randomDirection = FMath::Rand() % 2;
		angleHDest = randomDirection ? angleHDest : -angleHDest;
	}
}

void ATrackGenerator::GetVerticalAngle()
{
	int randomDirection;
	angleVDest = GetNewRandomAngle();

	if (lastPoint.Z < 0 && angleVDest < 0) {
		angleVDest = -angleVDest;
	}
	else if (lastPoint.Z > 200 && angleVDest > 0) {
		angleVDest = -angleVDest;
	}
	else if (lastPoint.Z > 0 && lastPoint.Z < 200)
	{
		randomDirection = FMath::Rand() % 2;
		angleVDest = randomDirection ? angleVDest : -angleVDest;
	}
}

int ATrackGenerator::GetNewRandomAngle()
{
	int random = FMath::Rand() % 7;
	float angle = 0;
	switch (random) {
	case 0:
		angle = 0;
		break;
	case 1:
		angle = 15;
		break;
	case 2:
		angle = 30;
		break;
	case 3:
		angle = 45;
		break;
	case 4:
		angle = 60;
		break;
	case 5:
		angle = 75;
		break;
	case 6:
		angle = 90;
		break;
	case 7:
		angle = 270;
		break;
	case 8:
		angle = 315;
		break;
	case 9:
		angle = 360;
		break;
	}
	return angle;
}

int ATrackGenerator::GetNewSharpAngle()
{
	int random = FMath::Rand() % 12;
	float angle = 0;
	switch (random) {
	case 0:
		angle = 0;
		break;
	case 1:
		angle = 30;
		break;
	case 2:
		angle = 60;
		break;
	case 3:
		angle = 90;
		break;
	case 4:
		angle = 120;
		break;
	case 5:
		angle = 150;
		break;
	case 6:
		angle = 180;
		break;
	case 7:
		angle = 210;
		break;
	case 8:
		angle = 240;
		break;
	case 9:
		angle = 270;
		break;
	case 10:
		angle = 300;
		break;
	case 11:
		angle = 330;
		break;
	}
	return angle;
}

int ATrackGenerator::GetVerySharpAngle()
{
	int random = FMath::Rand() % 4;
	float angle = 0;
	switch (random) {
	case 0:
		angle = 0;
		break;
	case 1:
		angle = 45;
		break;
	case 2:
		angle = 90;
		break;
	case 3:
		angle = 135;
		break;
	case 4:
		angle = 180;
		break;
	case 5:
		angle = 225;
		break;
	case 6:
		angle = 270;
		break;
	case 10:
		angle = 315;
		break;
	}
	return angle;
}
