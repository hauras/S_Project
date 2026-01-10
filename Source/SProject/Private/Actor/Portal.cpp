

#include "Actor/Portal.h"

#include "Ability/SAttributeSet.h"
#include "Actor/Component/InventoryComponent.h"
#include "Character/PlayerCharacter.h"
#include "Instance/SGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;
	
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
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(InInteractor);
	USGameInstance* GameInstance = Cast<USGameInstance>(GetGameInstance());

	if (PlayerCharacter && GameInstance)
	{
		UInventoryComponent* InventoryComponent = PlayerCharacter->FindComponentByClass<UInventoryComponent>();
		const USAttributeSet* AS = Cast<USAttributeSet>(PlayerCharacter->GetAttributeSet());

		if (InventoryComponent && AS)
		{
			GameInstance->PlayerData.Inventory = InventoryComponent->GetInventoryList();
			GameInstance->PlayerData.EquippedItems = InventoryComponent->EquippedItems; 
			GameInstance->PlayerData.Health = AS->GetHealth();
			GameInstance->PlayerData.MaxHealth = AS->GetMaxHealth();
			GameInstance->PlayerData.Mana = AS->GetMana();
			GameInstance->PlayerData.MaxMana = AS->GetMaxMana();
			GameInstance->PlayerData.AttackPower = AS->GetAttackPower();

			GameInstance->PlayerData.bIsDataValid = true;

			UGameplayStatics::OpenLevel(this, TargetLevelName);

			UE_LOG(LogTemp, Warning, TEXT("데이터 저장 완료! %s 레벨로 이동합니다."), *TargetLevelName.ToString());

		}
	}
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

