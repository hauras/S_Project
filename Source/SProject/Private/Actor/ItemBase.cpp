

#include "Actor/ItemBase.h"

#include "Actor/Component/InventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "State/SPlayerState.h"
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
 	if (bIsPickup) return;

 	if (APawn* InteractorPawn = Cast<APawn>(InInteractor))
 	{
 		
 		if (APlayerController* PC = Cast<APlayerController>(InteractorPawn->GetController()))
 		{
 			if (ASHUD* HUD = Cast<ASHUD>(PC->GetHUD()))
 			{
 				HUD->ShowItemPickupWidget(ItemInfo, this);

 				bIsPickup = true;
				
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

