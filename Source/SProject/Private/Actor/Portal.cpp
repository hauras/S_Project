

#include "Actor/Portal.h"

#include "Ability/SAttributeSet.h"
#include "Actor/Component/InventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Instance/SGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
#include "State/SPlayerState.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = false;
	
	PortalCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("PortalCollision"));
	SetRootComponent(PortalCollision);
	PortalCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	PortalCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 레이저 차단!

	// 2. 메쉬 생성 및 부착
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PortalMesh"));
	PortalMesh->SetupAttachment(RootComponent);
}

void APortal::Interact_Implementation(AActor* InInteractor)
{
	// 1. 서버(방장)만 이 문을 열 수 있습니다.
	if (!HasAuthority()) return;

	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());
	if (!GI) return;

	// 기존에 혹시 남아있을지 모르는 쓰레기 데이터 청소 (선택 사항)
	GI->PlayerData.Empty();

	// 2. [핵심 ⭐] 현재 월드에 접속한 모든 플레이어를 찾아서 각각 짐을 쌉니다.
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC) continue;

		ASPlayerState* PS = PC->GetPlayerState<ASPlayerState>();
		APlayerCharacter* Character = Cast<APlayerCharacter>(PC->GetPawn());
		
		if (PS && Character)
		{
			const USAttributeSet* AS = Cast<USAttributeSet>(Character->GetAttributeSet());
			UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();

			if (PS && PS->Inventory && AS && ASC)
			{
				FSPlayerData Data;
				Data.Inventory = PS->Inventory->GetInventoryList();

				// ★ 수정 포인트: 배열을 순회하며 맵으로 변환하여 저장합니다. ⭐
				// PS->Inventory->GetEquippedItemsArray() 를 사용합니다.
				for (const FEquippedItemInfo& EquipInfo : PS->Inventory->GetEquippedItemsArray())
				{
					// 금고(Data.EquippedItems)는 TMap이므로 .Add(Key, Value)를 사용합니다.
					Data.EquippedItems.Add(EquipInfo.Slot, EquipInfo.ItemData);
				}

				// 스탯 저장 로직 (기존과 동일)
				Data.MaxHealth = ASC->GetNumericAttributeBase(AS->GetMaxHealthAttribute());
				Data.MaxMana = ASC->GetNumericAttributeBase(AS->GetMaxManaAttribute());
				Data.AttackPower = ASC->GetNumericAttributeBase(AS->GetAttackPowerAttribute());
				Data.Health = AS->GetHealth();
				Data.Mana = AS->GetMana();
				Data.bIsDataValid = true;

				// 개인 사물함에 저장
				FString PlayerName = PS->GetPlayerName();
				GI->PlayerData.Add(PlayerName, Data);

				UE_LOG(LogTemp, Warning, TEXT("[%s] 데이터 변환 및 저장 완료"), *PlayerName);
			}
		}
	}

	// 4. [핵심 ⭐] 다 같이 이동 (ServerTravel)
	// OpenLevel은 나 혼자 가는 것이고, ServerTravel은 클라이언트들을 데리고 갑니다.
	FString TransitionMap = TargetLevelName.ToString() + TEXT("?listen");
	GetWorld()->ServerTravel(TransitionMap);
}
void APortal::ShowInteractionWidget_Implementation()
{
	IInteractionInterface::ShowInteractionWidget_Implementation();
}

void APortal::HideInteractionWidget_Implementation()
{
	IInteractionInterface::HideInteractionWidget_Implementation();
}

void APortal::BeginPlay()
{
	Super::BeginPlay();
	
}

