// Fill out your copyright notice in the Description page of Project Settings.


#include "SpecialSneak.h"

// Sets default values for this component's properties
USpecialSneak::USpecialSneak()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpecialSneak::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USpecialSneak::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FVector USpecialSneak::GetDesiredUp(FVector currentUp, FVector hitNormal) {
	return hitNormal;
}


FVector USpecialSneak::ToLocal(FVector v) {
	if (!local) return v;
	return v.X * GetForwardVector() + v.Y * GetRightVector() + v.Z * GetUpVector();
}