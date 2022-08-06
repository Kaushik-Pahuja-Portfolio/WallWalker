// Fill out your copyright notice in the Description page of Project Settings.


#include "VectorMath.h"

float VectorMath::RadiansBetween(const FVector& v1, const FVector& v2){
	return FMath::Acos(v1.Dot(v2) / v1.Size() / v2.Size());
}
float VectorMath::DistanceInDirection(const FVector& vector, const FVector& direction) {
	return vector.Dot(direction) / direction.Size();
}
