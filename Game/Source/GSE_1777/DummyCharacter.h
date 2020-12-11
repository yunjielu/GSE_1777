// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GSE_1777Character.h"
#include "DummyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class GSE_1777_API ADummyCharacter : public AGSE_1777Character
{
	GENERATED_BODY()
	
public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ReplicateIntInitialOnly();

	UFUNCTION()
	void OnRep_ReplicateInt();

	void SerializeInitialOnlyData(FBufferArchive& OutData);
	void DeserializeInitialOnlyData(FBufferArchive& InData);

	// virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

public:
	UPROPERTY(ReplicatedUsing = OnRep_ReplicateIntInitialOnly)
	int									ReplicateIntInitialOnly = 0;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicateInt)
	int									ReplicateInt = 0;
};
