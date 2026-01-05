
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/InteractionInterface.h"
#include "ItemBase.generated.h"

class UItemDataAsset;
class UGameplayEffect;
class USphereComponent;
class UStaticMeshComponent;
class UWidgetComponent;



UCLASS()
class SPROJECT_API AItemBase : public AActor, public IInteractionInterface
{
	GENERATED_BODY()
	
public:	
	AItemBase();

	void Interact_Implementation(AActor* InInteractor);
	void ShowInteractionWidget_Implementation() override;
	void HideInteractionWidget_Implementation() override;
protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
	
	UPROPERTY(EditDefaultsOnly, Category = "Widget")
	TObjectPtr<UWidgetComponent> InteractWidget;

	UPROPERTY(EditAnywhere, Category = "Item Data")
	TArray<TObjectPtr<UItemDataAsset>> ItemInfo;

	UPROPERTY()
	bool bIsPickup = false;
};
