// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "PlayerMovement.generated.h"

/**
 *
 */
UCLASS()
class WALLWALKER_API UPlayerMovement : public UPawnMovementComponent
{
	GENERATED_BODY()
		//variables
public:
	enum MovementType { Walk, Crouch, Sprint, Sneak };
	struct HitInfo {
		FHitResult Hit;
		FVector Location;
		float Height;
		float Radius;
		FVector Up;
		FVector Velocity;
		HitInfo() {}
		HitInfo(FHitResult inHit, FVector inLocation, float inRadius, float inHeight, FVector inUp, FVector inVel = FVector::ZeroVector) {
			Hit = inHit;
			Location = inLocation;
			Radius = inRadius;
			Height = inHeight;
			inUp.Normalize();
			Up = inUp;
			Velocity = inVel;
		}
	};

	enum StepType { None, Up, Down, Ledge };
protected:

	//allows us to avoid calling SetSpeed for this many frames. important for convex angles;
	int DontSetSpeed = 0;

	MovementType MoveType;

	UPROPERTY(EditAnywhere, Category = "Standard Movement")
		float Gravity = 9.81;

	UPROPERTY(EditAnywhere, Category = "Standard Movement")
		float JumpHeight;

	UPROPERTY(EditAnywhere, Category = "Standard Movement")
		float StepHeight;

	UPROPERTY(EditAnywhere, Category = "Standard Movement")
		float MaxAngle = 65;

	float JumpSpeed;

	float HeightAdjustment;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Size")
		float HeightAdjustSpeed = 1;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Size")
		float NormalHeight = 100;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Size")
		float CrouchHeight;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Size")
		float NormalRadius = 50;

	float DesiredHeight = 100;

	float MovementSpeed;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Speed")
		float WalkSpeed = 400;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Speed")
		float SprintSpeed = 750;

	UPROPERTY(EditAnywhere, Category = "Standard Movement|Speed")
		float CrouchSpeed = 300;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		float SneakHeight;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		float SneakSpeed = 600;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak", meta = (ClampMin = "0", ClampMax = "1"));
	float DefaultSneakCamInfluence;

	float SneakCamInfluence;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		class UCurveFloat* InitialDiffCurve;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		class UCurveFloat* ActualDiffCurve;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		float MaxRotateSpeed = 90;

	/**the amount of time to not be grounded before it stops shadow sneaking*/
	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		float StopSneakingTime = .2;

	float NotGroundedTime;
	HitInfo LastFloorHit;
	bool GroundedLastFrame;

	UPROPERTY(EditAnywhere, Category = "Shadow Sneak")
		float SneakBufferWindow = 0.25;

	float SneakBuffer;

	float InitRotateScale;

	TArray<FHitResult> FloorHits;
	TArray<HitInfo> FloorHitInfos;
	int GroundNum;

	FVector FloorNormal = FVector::UpVector;
	FVector DesiredUp = FVector::UpVector;
	FVector LateralVel;
	FVector DownVel;
	FVector JumpVel;
	FVector StepVel;

	bool StartJump;
	bool EndJump;

	class UCapsuleComponent* Capsule;
	class APlayerPawn* Pawn;
	class UCameraComponent* Camera;
	//functions
public:
	virtual void TickComponent(float deltaTime, enum ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) override;
	//void AddHit(FHitResult);
	void AddHit(HitInfo);
	void SetMoveType(MovementType);
	void SetMoveType(MovementType, FString);
	MovementType GetMoveType() const;
	MovementType PrintMoveType(FColor, float) const;
	void Jump();
	float GetNormalRadius() const;
	float GetNormalHeight() const;
	float GetMovementSpeed() const;

	bool HittingBottom(const FHitResult&, float);
	bool HittingBottom(const HitInfo&, float);
	bool HittingTop(const FHitResult&, float);
	bool HittingTop(const HitInfo&, float);
	bool HittingSides(const FHitResult&);
	bool HittingSides(const HitInfo&);
	FVector GetLateralVel() const;
	FVector GetDesiredUp() const;

	bool IsGrounded();


protected:
	UFUNCTION()
		virtual void BeginPlay() override;
	void ProcessFloorHits();
	void Rotate(float deltaTime);
	void SetDesiredUp(FVector);
	float GetJumpSpeed();
	void SetSpeed();
	bool CanJump() const;
	float GetHeightAdjustment(float);
	float AdjustCapsule(float);
	bool IsStep(const HitInfo& hit);
	StepType GetStepType(HitInfo& hit);
	FVector GetStepDown(const HitInfo& hit);
	FVector GetStepUp(const HitInfo& hit);
};
