// Fill out your copyright notice in the Description page of Project Settings.


#include "DummyCharacter.h"
#include "UnrealNetwork.h"
#include "SpatialStatics.h"

void ADummyCharacter::OnRep_ReplicateIntInitialOnly()
{
	UE_LOG(LogGSE_1777Character, Warning, TEXT("%s - ReplicateIntInitialOnly:[%d]"), *FString(__FUNCTION__), ReplicateIntInitialOnly);
}

void ADummyCharacter::OnRep_ReplicateInt()
{
	UE_LOG(LogGSE_1777Character, Warning, TEXT("%s - ReplicatinInt:[%d]"), *FString(__FUNCTION__), ReplicateInt);
}

void ADummyCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ADummyCharacter, ReplicateIntInitialOnly, COND_InitialOnly);
	DOREPLIFETIME(ADummyCharacter, ReplicateInt);
}

/*
bool ADummyCharacter::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	if (const ADummyCharacter* Char = Cast<ADummyCharacter>(ViewTarget))
	{
		if (Char->ReplicateInt - 1 == this->ReplicateInt)
		{
			return false;
		}
	}

	return Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);
}
*/

void ADummyCharacter::SerializeInitialOnlyData(FBufferArchive& OutData)
{
	OutData << ReplicateIntInitialOnly;
}

void ADummyCharacter::DeserializeInitialOnlyData(FBufferArchive& InData)
{
	InData << ReplicateIntInitialOnly;
}

