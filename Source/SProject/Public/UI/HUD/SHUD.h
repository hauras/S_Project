
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SHUD.generated.h"

class UInventoryWidgetController;
class UItemDataAsset;
class UItemPickupWidgetController;
class UAttributeSet;
class UAbilitySystemComponent;
class UOverlayWidgetController;
class USUserWidgetBase;
struct FWidgetControllerParams;

/**
 * 
 */
UCLASS()
class SPROJECT_API ASHUD : public AHUD
{
	GENERATED_BODY()
public:

	UPROPERTY()
	TObjectPtr<USUserWidgetBase> OverlayWidget;

	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& Params);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	UItemPickupWidgetController* GetItemPickupWidgetController(const FWidgetControllerParams& Params);

	void ShowItemPickupWidget(const TArray<UItemDataAsset*>& Items, AActor* TargetBox);

	void InitInventory(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

	// 2. 가방 열고 닫기 (플레이어 컨트롤러에서 호출할 예정)
	void ToggleInventory();

	UInventoryWidgetController* GetInventoryWidgetController(const struct FWidgetControllerParams& Params);

	/*UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void ShowBossHealthBar(AActor* BossActor, const FString& BossName);

	UFUNCTION(BlueprintImplementableEvent, Category = "HUD")
	void HideBossHealthBar();*/
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<USUserWidgetBase> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<USUserWidgetBase> ItemPickupWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UItemPickupWidgetController> ItemPickupWidgetController;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UItemPickupWidgetController> ItemPickupWidgetControllerClass;


	UPROPERTY()
	TObjectPtr<USUserWidgetBase> InventoryWidget;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<USUserWidgetBase> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<class UInventoryWidgetController> InventoryWidgetController;

	UPROPERTY(EditAnywhere, Category = "Widget")
	TSubclassOf<class UInventoryWidgetController> InventoryWidgetControllerClass;
};
