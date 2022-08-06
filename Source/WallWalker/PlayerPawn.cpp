// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerPawn.h"
#include "Playermovement.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "VectorMath.h"

// Sets default values
APlayerPawn::APlayerPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Player0;

	MovementComp = CreateDefaultSubobject<UPlayerMovement>(TEXT("Movement Component"));
	
	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Root Comp"));
	Sphere->SetSphereRadius(MovementComp->GetNormalRadius() + 1);
	Sphere->SetCollisionProfileName(TEXT("Pawn"));
	Sphere->SetSphereRadius(MovementComp->GetNormalRadius());

	RootComponent = Sphere;

	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetCollisionProfileName(TEXT("Pawn"));
	Capsule->SetCapsuleSize(MovementComp->GetNormalRadius(), MovementComp->GetNormalHeight());
	Capsule->SetupAttachment(RootComponent);
	
	FScriptDelegate del;
	del.BindUFunction(this, "RootHit");
	//Capsule->OnComponentHit.AddDynamic(this, &APlayerPawn::RootHit);
	Sphere->OnComponentHit.Add(del);

	FScriptDelegate bonkdel;
	bonkdel.BindUFunction(this, "HeadBonkStart");
	Capsule->OnComponentHit.Add(bonkdel);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(Capsule);
	Camera->SetRelativeLocation(FVector(0, 0, BaseEyeHeight));
	Camera->SetRelativeRotation(FRotator(0, 0, 0));

	MovementComp->SetUpdatedComponent(RootComponent);
}

// Called when the game starts or when spawned
void APlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	

}

// Called every frame
void APlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FCollisionQueryParams Params;
	FHitResult outHit;
	Params.AddIgnoredActor(this);
	FVector start = GetActorLocation() + GetActorUpVector() * Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere();
	bool bonked = GetWorld()->SweepSingleByChannel(outHit, start, start, GetActorQuat(),
		ECC_Visibility, 
		FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), 
		Capsule->GetScaledCapsuleHalfHeight()), Params);
	GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Green, AActor::GetDebugName(outHit.GetActor()));
	;
	if (bonked) {
		StartCrouch();
	}
}

UCapsuleComponent* APlayerPawn::GetCapsule() {
	return Capsule;
}

// Called to bind functionality to input
void APlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &APlayerPawn::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &APlayerPawn::MoveRight);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APlayerPawn::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APlayerPawn::TurnAtRate);

	PlayerInputComponent->BindAction("Jump", EInputEvent::IE_Pressed, MovementComp, &UPlayerMovement::Jump);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &APlayerPawn::StartSprint);
	PlayerInputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &APlayerPawn::EndSprint);
	//PlayerInputComponent->BindAction("Sneak", EInputEvent::IE_Pressed, this, &APlayerPawn::ToggleSneak);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerPawn::ToggleCrouch);
	//PlayerInputComponent->BindAction("Reset", IE_Pressed, this, &APlayerPawn::ResetLevel);
	//PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &APlayerPawn::Pause).bExecuteWhenPaused = true;
}

void APlayerPawn::MoveForward(float scale) {
	MovementComp->AddInputVector(GetActorForwardVector() * scale * MovementComp->GetMovementSpeed());
}

void APlayerPawn::MoveRight(float scale) {
	MovementComp->AddInputVector(GetActorRightVector() * scale * MovementComp->GetMovementSpeed());
}

void APlayerPawn::LookUpAtRate(float rate) {
	float addrot = rate;
	FRotator rot = Camera->GetRelativeRotation() + FRotator(-addrot, 0, 0);
	rot = FRotator(FMath::Clamp(rot.Pitch, -90.0f, 90.0f), rot.Yaw, rot.Roll);
	Camera->SetRelativeRotation(rot.Quaternion());
	if (Camera->GetRightVector() != GetActorRightVector()) {
		FVector newForward = Camera->GetForwardVector();
		FVector newUp = newForward ^ GetActorRightVector();
		newUp.Normalize();
		if (VectorMath::DistanceInDirection(newUp, GetActorUpVector()) < 0) newUp = -FMath::Sign(VectorMath::DistanceInDirection(Camera->GetForwardVector(), GetActorUpVector())) * GetActorForwardVector();
		//FQuat q = FTransform(newForward, GetActorRightVector(), newUp, MyCamera->GetComponentLocation()).GetRotation();
		Camera->SetWorldTransform(FTransform(newForward, GetActorRightVector(), newUp, Camera->GetComponentLocation()));
	}
}

void APlayerPawn::TurnAtRate(float rate) {
	FVector newRight = RootComponent->GetRightVector().RotateAngleAxis(rate, RootComponent->GetUpVector());
	FVector newForward = RootComponent->GetForwardVector().RotateAngleAxis(rate, RootComponent->GetUpVector());
	RootComponent->SetWorldTransform(FTransform(newForward, newRight, RootComponent->GetUpVector(), GetActorLocation()));
}

void APlayerPawn::ToggleSprint() {
	MovementComp->GetMoveType() == UPlayerMovement::MovementType::Sprint ? EndSprint() : StartSprint();
}

void APlayerPawn::StartSprint() {
	MovementComp->SetMoveType(UPlayerMovement::MovementType::Sprint);
	//UGameplayStatics::SetGlobalTimeDilation(this, 0.2);
}

void APlayerPawn::EndSprint() {
	if (MovementComp->GetMoveType() == UPlayerMovement::MovementType::Sprint) MovementComp->SetMoveType(UPlayerMovement::MovementType::Walk);
	UGameplayStatics::SetGlobalTimeDilation(this, 1);
}

void APlayerPawn::ToggleCrouch() {
	(MovementComp->GetMoveType() == UPlayerMovement::MovementType::Crouch) ? EndCrouch() : StartCrouch();
}

void APlayerPawn::StartCrouch() {
	MovementComp->SetMoveType(UPlayerMovement::MovementType::Crouch);
}

void APlayerPawn::EndCrouch() {
	//if (MovementComp->GetMoveType() == UPlayerMovement::MovementType::Crouch) MovementComp->SetMoveType(UPlayerMovement::MovementType::Walk);
	MovementComp->SetMoveType(UPlayerMovement::MovementType::Walk);
}

void APlayerPawn::RootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& SweepResult) {
	//construct HitInfo from current information and add it.
	MovementComp->AddHit(UPlayerMovement::HitInfo(SweepResult, GetActorLocation(), Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight_WithoutHemisphere(), GetActorUpVector()));
}

void APlayerPawn::HeadBonkStart(class UPrimitiveComponent* HitComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Orange, "BONK");
	StartCrouch();
}

void APlayerPawn::HeadBonkEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}



UPawnMovementComponent* APlayerPawn::GetMovementComponent() const {
	return MovementComp;
}

FVector APlayerPawn::GetFeetPosition() const {
	return GetActorLocation() + GetActorUpVector() * Capsule->GetScaledCapsuleHalfHeight();
}