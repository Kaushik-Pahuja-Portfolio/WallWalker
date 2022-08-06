// Fill out your copyright notice in the Description page of Project Settings.


#include "SneakOverride.h"

// Sets default values for this component's properties
USneakOverride::USneakOverride()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USneakOverride::BeginPlay()
{
	Super::BeginPlay();
	Tolerance *= PI / 180;
	GetDesiredUp(FVector::ZeroVector, FVector::ZeroVector);
	// ...
	
}

FVector USneakOverride::GetDesiredUp(FVector currentUp, FVector hitNormal) {
	//if (Normal == FVector::ZeroVector) Normal = GetUpVector();
	FVector norm = FVector::ZeroVector;
	if (Normal != FVector::ZeroVector) {
		if (Relative) norm = GetForwardVector() * Normal.X + GetRightVector() * Normal.Y + GetUpVector() * Normal.Z;
		norm.Normalize();
	}
	return norm;
}


