// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
namespace VectorMath {
	/*calculates the angle between two vectors in radians*/
	float RadiansBetween(const FVector& v1, const FVector& v2);

	/*calculates the magnitude of vector projected in direction*/
	float DistanceInDirection(const FVector& vector, const FVector &direction);
};