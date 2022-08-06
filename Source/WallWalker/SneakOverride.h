// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SpecialSneak.h"
#include "SneakOverride.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WALLWALKER_API USneakOverride : public USpecialSneak
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USneakOverride();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
		FVector Normal = FVector::UpVector;
	UPROPERTY(EditAnywhere)
		bool Relative = false;
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", ClampMax = "90"))
		float Tolerance = 90;

public:
	virtual FVector GetDesiredUp(FVector currentUp, FVector hitNormal) override;

		
};
