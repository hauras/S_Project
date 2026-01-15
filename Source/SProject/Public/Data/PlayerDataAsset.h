#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "PlayerDataAsset.generated.h"

// 전방 선언: 헤더를 가볍게 유지합니다.
class UGameplayAbility;
class APlayerCharacter;
class UAbilityInfo; 
class UTexture2D;

USTRUCT(BlueprintType)
struct FCharacterAttributeInfo
{
	GENERATED_BODY()

	// 1. 캐릭터 본체 블루프린트
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<APlayerCharacter> CharacterClass;

	// 2. 캐릭터 초상화 (화면 상단 아이콘용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> CharacterIcon;

	// 3. 캐릭터 고유 태그 (Character.Hero.Warrior 등)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CharacterTag;

	// 4. 실제로 캐릭터에게 부여할 스킬 리스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayAbility>> Abilities;

	// ★ 5. [추가] 이 캐릭터 전용 스킬 UI 정보 (아이콘, 쿨타임 태그 등)
	// 캐릭터가 바뀌면 UI는 이 사전을 보고 스킬창을 다시 그립니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAbilityInfo> AbilityInfo;
};

UCLASS()
class SPROJECT_API UPlayerDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 캐릭터 상세 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Info")
	FCharacterAttributeInfo CharacterInfo;

	// 에셋 매니저가 이 파일을 식별하는 고유 ID
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId("CharacterData", GetFName());
	}
};