

#include "UI/HUD/SHUD.h"

#include "Character/EnemyCharacter.h"
#include "State/SPlayerState.h"
#include "UI/SUserWidgetBase.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/ItemPickupWidgetController.h" // 헤더 추가 필수!
#include "UI/WidgetController/InventoryWidgetController.h"
#include "Controller/SPlayerController.h"

UOverlayWidgetController* ASHUD::GetOverlayWidgetController(const FWidgetControllerParams& Params)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(Params);
		OverlayWidgetController->BindCallbacksToDependencies();
		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

void ASHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	OverlayWidget = CreateWidget<USUserWidgetBase>(GetWorld(), OverlayWidgetClass);

	// 2. 위젯 컨트롤러를 생성하고 의존성을 바인딩합니다.
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	// 3. 위젯에게 위젯 컨트롤러를 설정합니다. (여기까지는 동일)
	OverlayWidget->SetWidgetController(WidgetController);

	// 4. 위젯을 먼저 화면에 추가합니다.
	OverlayWidget->AddToViewport();
	
	// ★★★ 5. 그리고 '가장 마지막에' 초기값 방송을 요청합니다! ★★★
	WidgetController->BroadcastInitialValues();
}

UItemPickupWidgetController* ASHUD::GetItemPickupWidgetController(const FWidgetControllerParams& Params)
{
	if (ItemPickupWidgetController == nullptr)
	{
		ItemPickupWidgetController = NewObject<UItemPickupWidgetController>(this, ItemPickupWidgetControllerClass);
		ItemPickupWidgetController->SetWidgetControllerParams(Params);
		ItemPickupWidgetController->BindCallbacksToDependencies();
	}
	return ItemPickupWidgetController;
}

void ASHUD::ShowItemPickupWidget(const TArray<UItemDataAsset*>& Items, AActor* TargetBox)
{
	USUserWidgetBase* LootWidget = CreateWidget<USUserWidgetBase>(GetWorld(), ItemPickupWidgetClass);

	// 2. 컨트롤러 파라미터 준비 (이미 있는 변수들 활용)
	// HUD는 이미 PlayerController 등을 알고 있습니다.
	APlayerController* PC = GetOwningPlayerController();
	ASPlayerState* PS = PC ? PC->GetPlayerState<ASPlayerState>() : nullptr;

	if (PS)
	{
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		UAttributeSet* AS = PS->GetAttributeSet();

		const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

		UItemPickupWidgetController* WidgetController = GetItemPickupWidgetController(WidgetControllerParams);


		LootWidget->SetWidgetController(WidgetController);
		WidgetController->SetLootItems(Items, TargetBox);

		LootWidget->AddToViewport();

		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void ASHUD::InitInventory(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	if (InventoryWidgetClass == nullptr) return;

	// 위젯 생성
	InventoryWidget = CreateWidget<USUserWidgetBase>(GetWorld(), InventoryWidgetClass);

	// 컨트롤러 설정
	const FWidgetControllerParams Params(PC, PS, ASC, AS);
	UInventoryWidgetController* WidgetController = GetInventoryWidgetController(Params);
    
	InventoryWidget->SetWidgetController(WidgetController);
    
	// 처음에 가방은 숨겨둔 상태로 화면에 붙여만 둡니다.
	InventoryWidget->AddToViewport();
	InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
    
	// [중요] 초기 아이템 목록 방송!
	WidgetController->BroadcastInitialValues();
}

void ASHUD::ToggleInventory()
{

	if (!InventoryWidget) return;

	ASPlayerController* PC = Cast<ASPlayerController>(GetOwningPlayerController());
    
	// 현재 가방이 보이는지 확인
	bool bIsVisible = InventoryWidget->GetVisibility() == ESlateVisibility::Visible;

	if (bIsVisible)
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
		PC->SetShowMouseCursor(false);
		PC->SetInputMode(FInputModeGameOnly()); 
		PC->SetCrosshairVisibility(true);
	}
	else
	{
		InventoryWidget->SetVisibility(ESlateVisibility::Visible);
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
		PC->SetCrosshairVisibility(false);
	}
}

UInventoryWidgetController* ASHUD::GetInventoryWidgetController(const struct FWidgetControllerParams& Params)
{
	if (InventoryWidgetController == nullptr)
	{
		InventoryWidgetController = NewObject<UInventoryWidgetController>(this, InventoryWidgetControllerClass);
		InventoryWidgetController->SetWidgetControllerParams(Params);
		InventoryWidgetController->BindCallbacksToDependencies();
	}
	return InventoryWidgetController;
}


