// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SpecialSneak.h"
#include "StairsMaker.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WALLWALKER_API UStairsMaker : public USpecialSneak
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStairsMaker();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//up vectors for these stairs.
	UPROPERTY(EditInstanceOnly, Category = "stairs")
		TArray<FVector> UpVectors;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual FVector GetDesiredUp(FVector currentUp, FVector hitNormal) override;
		
};
