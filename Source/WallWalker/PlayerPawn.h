// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PlayerMovement.h"
#include "PlayerPawn.generated.h"

UCLASS()
class WALLWALKER_API APlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlayerPawn();

	//variables
public:
	UPROPERTY(EditAnywhere, Category = "Camera")
		class UCameraComponent* Camera;

protected:
	UPROPERTY(EditAnywhere, Category = "Collisions")
		class UCapsuleComponent* Capsule;
	UPROPERTY(EditAnywhere, Category = "Collisions")
		class USphereComponent* Sphere;
	UPROPERTY(EditAnywhere, Category = "Movement")
		class UPlayerMovement* MovementComp;

	class UMyUserWidget* pauseMenu;
	bool isPaused;

	//functions
public:
	UFUNCTION()
		virtual void Tick(float deltaTime) override;

	//UFUNCTION(BlueprintCallable)
	//void ResetLevel();

	FVector GetFeetPosition() const;

	class UCapsuleComponent* GetCapsule();

protected:
	//UFUNCTION()
	virtual void SetupPlayerInputComponent(class UInputComponent*) override;
	//added
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	//added

	virtual void BeginPlay() override;
	//added
	//virtual void EndPlay(EEndPlayReason::Type) override;

	//input related stuff
	void MoveForward(float);	//implemented
	void MoveRight(float);		//implemented

	void LookUpAtRate(float);	//implemented
	void TurnAtRate(float);		//implemented

	void Jump();				//implemented

	void StartSprint();			//implemented
	void EndSprint();			//implemented
	void ToggleSprint();		//implemented

	void StartCrouch();			//implemented
	void EndCrouch();			//implemented
	void ToggleCrouch();		//implemented

	//collision related stuff
	UFUNCTION()
		void RootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);
	
	// declare overlap begin function
	UFUNCTION()
		void HeadBonkStart(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// declare overlap end function
	UFUNCTION()
		void HeadBonkEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};