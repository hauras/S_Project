

#include "Actor/ItemBase.h"

#include "Actor/Component/InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "UI/HUD/SHUD.h"

AItemBase::AItemBase()
{
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	ItemMesh->SetupAttachment(RootComponent);

	InteractWidget = CreateDefaultSubobject<UWidgetComponent>("InteractWidget");
	InteractWidget->SetupAttachment(RootComponent);

	InteractWidget->SetVisibility(false);
}

void AItemBase::Interact_Implementation(AActor* InInteractor)
 {
 	// 1. [안전장치] 이미 열려있는 상자라면 다시 실행하지 않음
 	if (bIsPickup) return;

 	// 2. 나를 만진 액터(InInteractor)로부터 플레이어 컨트롤러를 찾습니다.
 	if (APawn* InteractorPawn = Cast<APawn>(InInteractor))
 	{
 		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
 		{
 			// 3. 컨트롤러가 가진 우리 팀 배달부(ASHUD)를 가져옵니다.
 			if (ASHUD* HUD = Cast<ASHUD>(PC->GetHUD()))
 			{
 				// 4. [핵심] HUD에게 "내 아이템 목록(ItemInfo)"과 "내 주소(this)"를 넘겨주며 창을 띄우라고 합니다.
 				HUD->ShowItemPickupWidget(ItemInfo, this);

 				// 5. [상태 변화] 이제 이 상자는 '열림' 상태입니다. (중복 방지)
 				bIsPickup = true;
				
 				// 로그 확인용
 				UE_LOG(LogTemp, Warning, TEXT("상자 오픈! HUD에게 데이터 전달 완료."));
 			}
 		}
 	}
 }

void AItemBase::ShowInteractionWidget_Implementation()
{
 	InteractWidget->SetVisibility(true);

}

void AItemBase::HideInteractionWidget_Implementation()
{
 	InteractWidget->SetVisibility(false);

}

