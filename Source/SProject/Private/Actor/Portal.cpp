

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
	USGameInstance* GI = Cast<USGameInstance>(GetGameInstance());

	if (PlayerCharacter && GI)
	{
		ASPlayerState* PS = Cast<ASPlayerState>(PlayerCharacter->GetPlayerState());
		const USAttributeSet* AS = Cast<USAttributeSet>(PlayerCharacter->GetAttributeSet());
		// ★ 중요: 순수 기본값을 가져오기 위해 ASC가 필요합니다.
		UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();

		if (PS && PS->Inventory && AS && ASC)
		{
			GI->PlayerData.Inventory = PS->Inventory->GetInventoryList();
			GI->PlayerData.EquippedItems = PS->Inventory->EquippedItems; 

			GI->PlayerData.MaxHealth = ASC->GetNumericAttributeBase(AS->GetMaxHealthAttribute());
			GI->PlayerData.MaxMana = ASC->GetNumericAttributeBase(AS->GetMaxManaAttribute());
			GI->PlayerData.AttackPower = ASC->GetNumericAttributeBase(AS->GetAttackPowerAttribute());

			GI->PlayerData.Health = AS->GetHealth();
			GI->PlayerData.Mana = AS->GetMana();

			GI->PlayerData.bIsDataValid = true;

			UGameplayStatics::OpenLevel(this, TargetLevelName);
			UE_LOG(LogTemp, Warning, TEXT("데이터 저장 완료 (순수 Base 스탯 보존)"));
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

