// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SpecialSneak.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WALLWALKER_API USpecialSneak : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpecialSneak();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	//are these up vectors in local space?
	UPROPERTY(EditAnywhere)
		bool local;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual FVector GetDesiredUp(FVector currentUp, FVector hitNormal);
	virtual FVector ToLocal(FVector v);
		
};
