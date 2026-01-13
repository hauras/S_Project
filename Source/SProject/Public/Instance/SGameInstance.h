
#pragma once

#include "CoreMinimal.h"
#include "Data/ItemDataAsset.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "SGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FSPlayerData
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<UItemDataAsset>> Inventory;

	UPROPERTY()
	TMap<EEquipmentSlot, TObjectPtr<UItemDataAsset>> EquippedItems;
	
	UPROPERTY()
	float Health = 0.f;
	
	UPROPERTY()
	float MaxHealth = 0.f;
	
	UPROPERTY()
	float Mana = 0.f;
	
	UPROPERTY()
	float MaxMana = 0.f;
	
	UPROPERTY()
	float AttackPower = 0.f;

	UPROPERTY()
	bool bIsDataValid = false;
};

class FOnlineSessionSearch;
/**
 * 
 */
UCLASS()
class SPROJECT_API USGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	USGameInstance();

	UFUNCTION(BlueprintCallable)
	void HostRoom(); // 방 생성
	UFUNCTION(BlueprintCallable)
	void FindRoom(); // 방 찾기

	UPROPERTY()
	TMap<FString, FSPlayerData> PlayerData;
protected:

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result); 
private:
	IOnlineSessionPtr SessionInterface;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
