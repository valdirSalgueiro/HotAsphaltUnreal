// Fill out your copyright notice in the Description page of Project Settings.

#include "Building.h"
#include "Runtime/Engine/Classes/Engine/StaticMesh.h"
#include "Runtime/Engine/Classes/Materials/MaterialInstanceDynamic.h"
#include "Runtime/CoreUObject/Public/UObject/ConstructorHelpers.h"

// Sets default values
ABuilding::ABuilding()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	UStaticMeshComponent* BuildingComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMeshComponent"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/TechArtAid/RandomizingInstances/Models/Block/RandomizingInstances_Model_Block_Wall.RandomizingInstances_Model_Block_Wall'"));
	BuildingComponent->SetStaticMesh(MeshAsset.Object);
	BuildingComponent->SetMobility(EComponentMobility::Static);

	UMaterialInterface* CurrentMat = BuildingComponent->GetMaterial(0);
	UMaterialInstanceDynamic* RandMat = UMaterialInstanceDynamic::Create(CurrentMat, BuildingComponent);


	FLinearColor randomColor = FLinearColor::MakeRandomColor();
	RandMat->SetVectorParameterValue(FName(TEXT("WallPaintColor2")), randomColor);
	//RandMat->SetVectorParameterValue(FName(TEXT("WallPaintColor1")), FLinearColor::MakeRandomColor());
	RandMat->SetScalarParameterValue(FName(TEXT("DirtHeightMax")), 100000);
	RandMat->SetScalarParameterValue(FName(TEXT("DirtHeightMin")), 0);
	RandMat->SetScalarParameterValue(FName(TEXT("DirtTurbulence")), 0);
	BuildingComponent->SetMaterial(0, RandMat);
	RootComponent = BuildingComponent;

	int height = MeshAsset.Object->GetBounds().BoxExtent.Z * 2;


	int extraFloors = FMath::Rand() % 5 + 5;
	for (int i = 0; i < extraFloors; i++)
	{
		BuildingComponent = CreateDefaultSubobject<UStaticMeshComponent>(*FString::FromInt(i));
		BuildingComponent->SetStaticMesh(MeshAsset.Object);
		BuildingComponent->SetMobility(EComponentMobility::Static);

		CurrentMat = BuildingComponent->GetMaterial(0);
		RandMat = UMaterialInstanceDynamic::Create(CurrentMat, BuildingComponent);
		RandMat->SetVectorParameterValue(FName(TEXT("WallPaintColor2")), randomColor);
		//RandMat->SetVectorParameterValue(FName(TEXT("WallPaintColor1")), FLinearColor::MakeRandomColor());
		RandMat->SetScalarParameterValue(FName(TEXT("DirtHeightMax")), 100000);
		RandMat->SetScalarParameterValue(FName(TEXT("DirtHeightMin")), 0);
		RandMat->SetScalarParameterValue(FName(TEXT("DirtTurbulence")), 0);
		BuildingComponent->SetMaterial(0, RandMat);
		BuildingComponent->SetRelativeLocation(FVector(0, 0, height * i));
		BuildingComponent->SetupAttachment(RootComponent);
	}

}

// Called when the game starts or when spawned
void ABuilding::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

