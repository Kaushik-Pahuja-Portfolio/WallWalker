// Fill out your copyright notice in the Description page of Project Settings.


#include "StairsMaker.h"
#include "VectorMath.h"

// Sets default values for this component's properties
UStairsMaker::UStairsMaker()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UStairsMaker::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UStairsMaker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FVector	 UStairsMaker::GetDesiredUp(FVector currentUp, FVector hitNormal) {
	FVector closestUp = hitNormal;
	float smallestAngle = PI;
	for (int i = 0; i < UpVectors.Num(); ++i) {
		FVector& v = UpVectors[i];
		float upAngle = VectorMath::RadiansBetween(currentUp, ToLocal(v));
		float normAngle = VectorMath::RadiansBetween(hitNormal, ToLocal(v));
		if (normAngle < 65 * PI / 180 &&  upAngle < smallestAngle) {
			closestUp = v.X * GetForwardVector() + v.Y * GetRightVector() + v.Z * GetUpVector();
			smallestAngle = upAngle;
		}
	}
	return closestUp;
}