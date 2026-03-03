// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/LootTable.h"

UItemDataAsset* ULootTable::RollItem()
{
	if (LootEntries.Num() == 0) return nullptr;

	float TotalWeight = 0.f;
	for (const FLootEntry& Entry : LootEntries)
	{
		TotalWeight += Entry.DropWeight;
	}

	float RandValue = FMath::FRandRange(0.f, TotalWeight);
	float CumulativeWeight = 0.f;

	for (const FLootEntry& Entry : LootEntries)
	{
		CumulativeWeight += Entry.DropWeight;
		if (RandValue <= CumulativeWeight)
		{
			return Entry.ItemData;
		}
	}
	return nullptr;

}
