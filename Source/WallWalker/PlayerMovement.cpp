// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerMovement.h"
#include "PlayerPawn.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Runtime/Engine/Public/DrawDebugHelpers.h"
#include "SneakOverride.h"
#include "SneakCamOverride.h"
#include "SpecialSneak.h"
#include "VectorMath.h"


void UPlayerMovement::BeginPlay() {
    Super::BeginPlay();
    Pawn = Cast<APlayerPawn>(PawnOwner);
    Capsule = Pawn->GetCapsule();
    if (Pawn) Camera = Pawn->Camera;
    StartJump = false;
    EndJump = false;
    GetJumpSpeed();
    UpdatedComponent = PawnOwner->GetRootComponent();
    SetMoveType(MovementType::Walk);
    SetSpeed();
}

void UPlayerMovement::TickComponent(float deltaTime, enum ELevelTick tickType, FActorComponentTickFunction* thisTickFunction) {
    Super::TickComponent(deltaTime, tickType, thisTickFunction);

    if (!Pawn || !UpdatedComponent || ShouldSkipUpdate(deltaTime)) return;
    //rotate
    //SetMoveType(MovementType::Sneak);
    FloorNormal.Normalize();

    if (DontSetSpeed) {
        --DontSetSpeed;
    }
    PrintMoveType(FColor::Cyan, deltaTime);
    //process lateral movement
    LateralVel = ConsumeInputVector().GetClampedToMaxSize(MovementSpeed);
    FVector tmpLatVel = LateralVel;
    if ((LateralVel | FloorNormal) != 0) {
        FVector idk = LateralVel ^ FloorNormal;
        LateralVel = idk ^ FloorNormal;
        if (VectorMath::DistanceInDirection(LateralVel, tmpLatVel) < 0) LateralVel *= -1;
        LateralVel.Normalize();
        LateralVel *= tmpLatVel.Size();
    }

    if (SneakBuffer < SneakBufferWindow) SneakBuffer += deltaTime;

    //jumping and gravity
    if (GroundNum) {
        if (SneakBuffer <= SneakBufferWindow) {
            //SetMoveType(MovementType::Sneak);
        }
        if (EndJump) {
            StartJump = EndJump = false;
            JumpVel = FVector::ZeroVector;
        }
        //if they haven't started to jump, set the movement speed here too.
        if (!StartJump) SetSpeed();
        NotGroundedTime = 0;
    }
    else {
        if (StartJump) {
            EndJump = true;
            if(DownVel.Size() > JumpVel.Size()) EndJump = true;
        }
        DownVel += DesiredUp/*(MoveType == MovementType::Sneak ? DesiredUp : FVector::UpVector) */ *-Gravity * deltaTime * 200;
        DownVel = DesiredUp * VectorMath::DistanceInDirection(DownVel, DesiredUp);
        /*if (MoveType == MovementType::Sneak && DontSetSpeed <= 0) {
            NotGroundedTime += deltaTime;
            if (NotGroundedTime > StopSneakingTime) SetMoveType(MovementType::Walk, TEXT("Not Grounded"));
        }*/
    }

    FVector desiredMovementThisFrame = (LateralVel + DownVel + JumpVel) * deltaTime + Capsule->GetUpVector() * AdjustCapsule(deltaTime) + StepVel;
    FVector vertMovementThisFrame = (DownVel + JumpVel) * deltaTime + Capsule->GetUpVector() * AdjustCapsule(deltaTime) + StepVel;
    if (!desiredMovementThisFrame.IsNearlyZero()) {
        FHitResult outHit;
        SafeMoveUpdatedComponent(desiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);
        if (outHit.IsValidBlockingHit())
        {
            SlideAlongSurface(desiredMovementThisFrame, 1.f - outHit.Time, outHit.Normal, outHit);
        }
        //SafeMoveUpdatedComponent(vertMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, outHit);
    }
    ProcessFloorHits();
    AdjustCapsule(deltaTime);
    Rotate(deltaTime);
    GroundedLastFrame = GroundNum != 0;
}

float UPlayerMovement::GetMovementSpeed() const {
    return MovementSpeed;
}

void UPlayerMovement::SetMoveType(MovementType toSet) {
    if (toSet == MoveType) return;
    //if it's already sneaking, since we know toSet won't be sneak, set the desired up to global up.
    /*if (MoveType == MovementType::Sneak) {
        SetDesiredUp(FVector::UpVector);
    }*/

    //if we want to sneak, check if grounded. if we are, set it, otherwise buffer it.
    /*if (toSet == MovementType::Sneak) {
        if (!GroundNum) SneakBuffer = 0;
        else {
            MoveType = toSet;
            SneakBuffer = SneakBufferWindow + 1;
        }
    }*/
    //if we aren't sneaking there's nothing to worry about (for now)
    ///will need to implement uncrouch checking later, but it works for now because they will recrouch if they can't uncrouch.
    else {
        MoveType = toSet;
        SneakBuffer = SneakBufferWindow + 1;
    }
    //setting desired height
    switch (MoveType) {
    case MovementType::Sneak:
        //DesiredHeight = SneakHeight;
        break;
    case MovementType::Crouch:
        DesiredHeight = CrouchHeight;
        break;
    case MovementType::Walk:
        DesiredHeight = NormalHeight;
        break;
    case MovementType::Sprint:
        DesiredHeight = NormalHeight;
        break;
    default:
        DesiredHeight = NormalHeight;
        break;
    }
}

void UPlayerMovement::SetMoveType(MovementType toSet, FString endSneakReason) {
    if (toSet == MoveType) return;
    //if it's already sneaking, since we know toSet won't be sneak, set the desired up to global up.
    /*if (MoveType == MovementType::Sneak) {
        SetDesiredUp(FVector::UpVector);
        GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, endSneakReason);
    }*/

    //if we want to sneak, check if grounded. if we are, set it, otherwise buffer it.
    /*if (toSet == MovementType::Sneak) {
        if (!GroundNum) SneakBuffer = 0;
        else {
            MoveType = toSet;
            SneakBuffer = SneakBufferWindow + 1;
        }
    }*/
    //if we aren't sneaking there's nothing to worry about (for now)
    ///will need to implement uncrouch checking later, but it works for now because they will recrouch if they can't uncrouch.
    else {
        MoveType = toSet;
        SneakBuffer = SneakBufferWindow + 1;
    }
    //setting desired height
    switch (MoveType) {
    case MovementType::Sneak:
        //DesiredHeight = SneakHeight;
        break;
    case MovementType::Crouch:
        DesiredHeight = CrouchHeight;
        break;
    case MovementType::Walk:
        DesiredHeight = NormalHeight;
        break;
    case MovementType::Sprint:
        DesiredHeight = NormalHeight;
        break;
    default:
        DesiredHeight = NormalHeight;
        break;
    }
}

UPlayerMovement::MovementType UPlayerMovement::GetMoveType() const {
    return MoveType;
}

UPlayerMovement::MovementType UPlayerMovement::PrintMoveType(FColor color, float time) const {
    switch (MoveType) {
    case MovementType::Sneak:
        GEngine->AddOnScreenDebugMessage(-1, time, color, "Sneak");
        break;
    case MovementType::Walk:
        GEngine->AddOnScreenDebugMessage(-1, time, color, "Walk");
        break;
    case MovementType::Crouch:
        GEngine->AddOnScreenDebugMessage(-1, time, color, "Crouch");
        break;
    case MovementType::Sprint:
        GEngine->AddOnScreenDebugMessage(-1, time, color, "Sprint");
        break;
    }
    return MoveType;
}

void UPlayerMovement::SetSpeed() {
    if (DontSetSpeed) return;
    switch (MoveType) {
    case MovementType::Walk:
        MovementSpeed = WalkSpeed;
        break;
    case MovementType::Crouch:
        MovementSpeed = CrouchSpeed;
        break;
    case MovementType::Sprint:
        MovementSpeed = SprintSpeed;
        break;
    case MovementType::Sneak:
        MovementSpeed = SneakSpeed;
        break;
    default: MovementSpeed = WalkSpeed;
    }
}

void UPlayerMovement::Jump() {
    if (CanJump()) {
        DownVel = FVector::ZeroVector;
        JumpVel = DesiredUp * JumpSpeed;
        StartJump = true;
        EndJump = false;
        GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Purple, TEXT("JUMP"));
        //FloorNormal = FVector::UpVector;
        //if (MoveType == MovementType::Sneak) SetMoveType(MovementType::Walk, TEXT("Jump"));
    }
}

bool UPlayerMovement::CanJump() const {
    if (Capsule) {
        GEngine->AddOnScreenDebugMessage(-1, 1, FColor::Red, FString::SanitizeFloat(GroundNum));
        FVector start = Capsule->GetComponentLocation();
        FVector end = start - Capsule->GetUpVector() * (Capsule->GetScaledCapsuleHalfHeight() + StepHeight);
        FCollisionQueryParams Params;
        FHitResult OutHit;
        Params.AddIgnoredActor(Pawn);
        GetWorld()->LineTraceSingleByChannel(OutHit, start, end, ECC_Visibility, Params);
        return OutHit.bBlockingHit || GroundNum > 0;
    }
    return false;
}

bool UPlayerMovement::IsGrounded() {
    return GroundNum > 0;
}

void UPlayerMovement::AddHit(HitInfo toAdd) {
    //if (VectorMath::DistanceInDirection(toAdd.Hit.ImpactPoint - toAdd.Location, toAdd.Up) < 0.1) {
    if(VectorMath::RadiansBetween(toAdd.Up, toAdd.Hit.ImpactPoint - toAdd.Location) <= PI + 0.1 / 2) 
        FloorHitInfos.Add(toAdd);
    //if ((MoveType == MovementType::Sneak || SneakBuffer <= SneakBufferWindow) ? !HittingTop(toAdd, 89) : HittingBottom(toAdd, MaxAngle)) FloorHitInfos.Add(toAdd);
    if (HittingTop(toAdd, 45)) {
        if (HeightAdjustment > 0 && !EndJump && !StartJump) {
            if (Capsule->GetScaledCapsuleHalfHeight() > CrouchHeight) {
                SetMoveType(MovementType::Crouch);
            }
            /*else if (Capsule->GetScaledCapsuleHalfHeight() > SneakHeight) {
                SetMoveType(MovementType::Sneak);
            }*/
        }
        if (StartJump || EndJump) {
            GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Red, TEXT("hit top"));
            if (VectorMath::DistanceInDirection(DownVel + JumpVel, FVector::UpVector) > 0) {
                DownVel = FVector::ZeroVector;
                JumpVel = FVector::ZeroVector;
            }
        }
    }
}

/* processes all the floor hits. basically it calculates the new up direction, floor normal, etc.
*  assumes that all floor hits are valid.
*/
void UPlayerMovement::ProcessFloorHits() {
    GroundNum = FloorHitInfos.Num();
    float distance = -Capsule->GetScaledCapsuleRadius() * 2;
    float floorAngle = 2 * PI;
    bool steppingUp = false;
    SneakCamInfluence = DefaultSneakCamInfluence;
    for (int i = 0; i < GroundNum; ++i) {
        HitInfo& hit = FloorHitInfos[i];
        float angle = VectorMath::RadiansBetween(hit.Hit.ImpactNormal, DesiredUp);
        UPrimitiveComponent* component = hit.Hit.GetComponent();
        USneakOverride* sneakOverride = NULL;
        USneakCamOverride* camOverride = NULL;
        USpecialSneak* specialSneak = NULL;
        if (component) {
            TArray<USceneComponent*> children;
            hit.Hit.GetComponent()->GetChildrenComponents(false, children);
            for (int j = 0; j < children.Num(); ++j) {
                sneakOverride = Cast<USneakOverride>(children[j]);
                if (sneakOverride) break;
            }
            for (int j = 0; j < children.Num(); ++j) {
                camOverride = Cast<USneakCamOverride>(children[j]);
                if (camOverride) break;
            }
            for (int j = 0; j < children.Num(); ++j) {
                specialSneak = Cast<USpecialSneak>(children[j]);
                if (specialSneak) break;
            }
        }
        if (camOverride) SneakCamInfluence = camOverride->Influence;
        //if the player isn't moving or is only grounded once and it's an acceptable angle
        //OR
        //if the player is moving more towards this impact point than all the previous ones
        if (angle <= floorAngle + 0.1 && (LateralVel.IsNearlyZero() || GroundNum == 1) || VectorMath::DistanceInDirection(hit.Hit.ImpactPoint - hit.Location, LateralVel) >= distance) {
            distance = VectorMath::DistanceInDirection(hit.Hit.ImpactPoint - hit.Location, LateralVel);
            //see IsStep function definition for more info on steps.
            if (!IsStep(hit)) {
                FloorNormal = hit.Hit.ImpactNormal;
                floorAngle = angle;
                DrawDebugLine(GetWorld(), hit.Hit.ImpactPoint, hit.Hit.ImpactPoint + 20 * hit.Hit.ImpactNormal, FColor::Green, false, 3);
            }
            //if it's a step upward
            else if (VectorMath::DistanceInDirection(hit.Hit.ImpactPoint - hit.Location, LateralVel) > 0) {
                steppingUp = true;
                DownVel = FVector::ZeroVector;
                DrawDebugLine(GetWorld(), hit.Hit.ImpactPoint, hit.Hit.ImpactPoint + 50 * hit.Hit.ImpactNormal, FColor::Red, false, 3);
            }
            //i wonder what this could be?
            else {
                floorAngle = angle;
                FVector start = hit.Location - hit.Up * (hit.Height + hit.Radius);
                FVector end = start - (hit.Hit.ImpactNormal).GetSafeNormal() * StepHeight;
                FHitResult outHit;
                FCollisionQueryParams params;
                params.AddIgnoredActor(PawnOwner);
                GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECC_Visibility, params);
                DrawDebugLine(GetWorld(), hit.Hit.ImpactPoint, hit.Hit.ImpactPoint + 50 * hit.Hit.ImpactNormal, FColor::Purple, false, 3);
                //if there's ground to step down onto (so it's not a ledge)
                if (outHit.bBlockingHit) {
                    FloorNormal = hit.Hit.ImpactNormal;
                    floorAngle = angle;
                }
                else {//if (MoveType == MovementType::Sneak) {// && (hit.Hit.ImpactPoint - (hit.Location - hit.Up * hit.Height))VectorMath::RadiansBetween(-hit.Up) >= PI/6){
                    //see which direction i need to cast in
                    //this works because basic geometry (radius from the center to a point on the surface is perpendicular to the tangent at that point)
                    //from there we just flatten it in the upward direction and normalize
                    FVector direction = hit.Location - hit.Hit.ImpactPoint;
                    direction -= VectorMath::DistanceInDirection(direction, DesiredUp) * DesiredUp;
                    direction.Normalize();
                    FloorNormal = direction + hit.Hit.ImpactNormal;
                    SetDesiredUp(FloorNormal);
                    DontSetSpeed = 4;
                    MovementSpeed = 200;
                    floorAngle = angle;
                    //DrawDebugLine(GetWorld(), hit.Hit.ImpactPoint, hit.Hit.ImpactPoint + 100 * hit.Hit.ImpactNormal, FColor::Black, false, 3);
                    DrawDebugLine(GetWorld(), start, end, FColor::Black, false, 3);
                    /*//place the start just a little below and away from the ledge
                    start = hit.Hit.ImpactPoint - direction - hit.Hit.ImpactNormal/2;
                    //cast towards the ledge
                    end = start + direction * hit.Radius;
                    GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECC_Visibility, params);
                    //if i hit something, set the floor normal to whatever was hit, otherwise just figure it's nothing
                    if(outHit.bBlockingHit){
                        FloorNormal = outHit.ImpactNormal;
                        floorAngle = angle;
                        DrawDebugLine(GetWorld(), hit.Hit.ImpactPoint, hit.Hit.ImpactPoint + 100 * hit.Hit.ImpactNormal, FColor::Black, false, 3);
                    }*/
                }
            }

            if (true){//MoveType == MovementType::Sneak) {
                if (!IsStep(hit)) {
                    FVector up = specialSneak ? specialSneak->GetDesiredUp(hit.Up, hit.Hit.ImpactNormal) : FloorNormal;
                    if (specialSneak && ((up == FVector::ZeroVector ? DesiredUp : up) | FloorNormal) != 0) {
                        SetDesiredUp(up);
                    }
                    else {
                        SetDesiredUp(FloorNormal);
                    }
                }
            }
            else {
                //SetDesiredUp(FVector::UpVector);
            }
            LastFloorHit = hit;
        }
    }
    if (GroundNum && !StartJump && !steppingUp) DownVel = -1000 * FloorNormal;
    if (!GroundNum && !StartJump && !steppingUp){//&& MoveType == MovementType::Sneak) {
        DownVel = -1200 * DesiredUp;
    }
    FloorHitInfos.Empty();
}

bool UPlayerMovement::IsStep(const HitInfo& hit) {
    FVector center = hit.Location;// -DesiredUp * hit.Height;
    bool isStep = VectorMath::RadiansBetween(center - hit.Hit.ImpactPoint, hit.Hit.ImpactNormal) > 12 * PI / 180;
    /*&& (
        hit.Hit.ImpactNormalVectorMath::RadiansBetween(DesiredUp) > MaxAngle * PI / 180
        ||
        (hit.Hit.ImpactPoint - hit.Location)VectorMath::DistanceInDirection(LateralVel) <= 0
    );*/
    //GEngine->AddOnScreenDebugMessage(-1, 1 / 60, FColor::Green, FString::SanitizeFloat(VectorMath::RadiansBetween(center - hit.Hit.ImpactPoint, hit.Hit.ImpactNormal) * 180 / PI));
    /*
    //if it's a step down, and that the step angle is less than 90 because we want limits on that.
    if(isStep && LateralVelVectorMath::DistanceInDirection(hit.Hit.ImpactNormal) > 0){
        //check if there's something to step down onto.
        FVector start = hit.Location - hit.Up * (hit.Radius + hit.Height);
        FVector end = start - hit.Up * StepHeight;
        FHitResult outHit;
        FCollisionQueryParams params;
        params.AddIgnoredActor(Pawn);
        GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECC_Visibility, params);
        isStep = outHit.bBlockingHit;
    }*/
    return isStep;
}

UPlayerMovement::StepType UPlayerMovement::GetStepType(HitInfo& hit) {
    FVector center = hit.Location;// -hit.Up * hit.Height;
    StepType step = StepType::None;
    //it's hitting an edge on the bottom and the corner is steeper than what's walkable or they're moving away from it
    //nifty little thing about this is that since it assumes it's hitting the bottom, hitting normal corners will never trigger this.
    bool isStep = VectorMath::RadiansBetween(center - hit.Hit.ImpactPoint, hit.Hit.ImpactNormal) > 5 * PI / 180;
    //if it's a step down, and that the step angle is less than 90 because we want limits on that.
    if (isStep) {
        if (VectorMath::DistanceInDirection(hit.Hit.ImpactPoint - hit.Location, LateralVel) <= 0) {
            step = StepType::Down;
            //check if there's something to step down onto.
            FVector start = hit.Location - hit.Up * (hit.Radius);// +hit.Height);
            FVector end = start - hit.Up * StepHeight;
            FHitResult outHit;
            FCollisionQueryParams params;
            params.AddIgnoredActor(Pawn);
            GetWorld()->LineTraceSingleByChannel(outHit, start, end, ECC_Visibility, params);
            if (!outHit.bBlockingHit) step = StepType::Ledge;
        }
        else step = StepType::Up;
    }
    return step;
}

FVector UPlayerMovement::GetStepUp(const HitInfo& hit) {
    return FVector::ZeroVector;
}

FVector UPlayerMovement::GetStepDown(const HitInfo& hit) {
    return FVector::ZeroVector;
}

bool UPlayerMovement::HittingBottom(const HitInfo& hit, float acceptableAngle) {
    FVector center = hit.Location - hit.Up * hit.Height;
    return VectorMath::RadiansBetween(center - hit.Hit.ImpactPoint, DesiredUp) < acceptableAngle * PI / 180;
}

bool UPlayerMovement::HittingTop(const HitInfo& hit, float acceptableAngle) {
    FVector center = hit.Location + hit.Up * hit.Height;
    return VectorMath::RadiansBetween(center - hit.Hit.ImpactPoint , -DesiredUp) < acceptableAngle * PI / 180;
}

bool UPlayerMovement::HittingSides(const HitInfo& hit) {
    return abs(VectorMath::DistanceInDirection(hit.Location - hit.Hit.ImpactPoint, hit.Up)) <= hit.Height;
}

void UPlayerMovement::Rotate(float deltaTime) {
    if (!FVector::Coincident(DesiredUp, Capsule->GetUpVector())) {
        FVector camForward;
        if (Camera) camForward = Camera->GetForwardVector();
        FVector newUp = DesiredUp;
        FVector newRight;
        FVector newForward;
        if (VectorMath::DistanceInDirection(newUp ^ camForward, Capsule->GetRightVector()) > 0) {// && (newUpVectorMath::DistanceInDirection(Capsule->GetForwardVector()) > 0)){
            newRight = newUp ^ camForward;
            newForward = newUp ^ newRight;
        }
        else {
            if (!FVector::Coincident(Capsule->GetForwardVector(), newUp)) {
                newRight = newUp ^ Capsule->GetForwardVector();
                newForward = newUp ^ newRight;
            }
            else {
                newForward = newUp ^ Capsule->GetRightVector();
                newRight = newUp ^ newForward;
            }
        }
        //if((newUp ^ camForward)VectorMath::DistanceInDirection(Capsule->GetRightVector()) > 0)
        newUp.Normalize();
        newRight.Normalize();
        newForward.Normalize();
        FQuat q = FTransform(newForward, newRight, newUp, FVector::ZeroVector).GetRotation();
        float angDist = q.AngularDistance(Capsule->GetComponentQuat());
        if (angDist <= MaxRotateSpeed * PI / 180 * deltaTime * InitRotateScale * (ActualDiffCurve ? ActualDiffCurve->GetFloatValue(angDist * 2 / PI) : 1)) {
        }
        else {
            q = FQuat::Slerp(Capsule->GetComponentQuat(), q, MaxRotateSpeed * PI / 180 * deltaTime * InitRotateScale * (ActualDiffCurve ? ActualDiffCurve->GetFloatValue(angDist * 2 / PI) : 1) / angDist);
        }
        PawnOwner->SetActorRotation(q);
        //preserve camera look rotation
        if (Camera) {
            float camLerp = SneakCamInfluence;
            if (VectorMath::DistanceInDirection(camForward, Capsule->GetForwardVector()) < 0) {
                camForward = FMath::Sign(VectorMath::DistanceInDirection(camForward, Capsule->GetUpVector())) * Capsule->GetUpVector();
                camLerp = 1;
            }
            if (camLerp < 1) {
                newUp = camForward ^ Capsule->GetRightVector();
                q = FTransform(camForward, Capsule->GetRightVector(), newUp, FVector::ZeroVector).GetRotation();
                Camera->SetWorldRotation(FQuat::Slerp(q, Camera->GetComponentQuat(), camLerp));

            }
        }
    }
}

float UPlayerMovement::GetHeightAdjustment(float deltaTime) {
    if (!Capsule) return 0;
    HeightAdjustment = HeightAdjustSpeed * deltaTime;
    //float angle = Capsule->GetUpVector()VectorMath::RadiansBetween(FloorNormal);
    return HeightAdjustment;
}

float UPlayerMovement::AdjustCapsule(float deltaTime) {
    if (!Capsule) return HeightAdjustment = 0;
    float height = Capsule->GetUnscaledCapsuleHalfHeight();
    float radius = Capsule->GetScaledCapsuleRadius();
    if (FMath::Abs(height - DesiredHeight) <= HeightAdjustSpeed * deltaTime) {
        HeightAdjustment = DesiredHeight - height;
        height = DesiredHeight;
    }
    else {
        HeightAdjustment = FMath::Sign(DesiredHeight - height) * HeightAdjustSpeed * deltaTime;
        height += HeightAdjustment;
    }
    radius = NormalRadius;
    if (radius > height) radius = height;
    Capsule->SetCapsuleRadius(radius);
    Capsule->SetCapsuleHalfHeight(height);
    if (Camera) Camera->SetRelativeLocation(FVector(0, 0, Pawn->BaseEyeHeight * (MoveType == MovementType::Sneak ? height / NormalHeight : (height - SneakHeight) / (NormalHeight - SneakHeight))));
    if (GroundNum == 0) HeightAdjustment = 0;
    return HeightAdjustment;
}

float UPlayerMovement::GetJumpSpeed() {
    JumpSpeed = FMath::Sqrt(100 * JumpHeight * 4 * 100 * Gravity);
    return JumpSpeed;
}

void UPlayerMovement::SetDesiredUp(FVector up) {
    up.Normalize();
    if (!(FVector::Coincident(up, DesiredUp) || up == FVector::ZeroVector)) {
        InitRotateScale = InitialDiffCurve ? InitialDiffCurve->GetFloatValue(VectorMath::RadiansBetween(up, Capsule->GetUpVector()) * 2 / PI) : 1;
        DesiredUp = up;
    }
}

FVector UPlayerMovement::GetDesiredUp() const {
    return DesiredUp;
}

float UPlayerMovement::GetNormalHeight() const {
    return NormalHeight;
}

float UPlayerMovement::GetNormalRadius() const {
    return NormalRadius;
}