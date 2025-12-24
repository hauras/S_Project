
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SHUD.generated.h"

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
	
};
