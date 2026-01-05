

#include "UI/HUD/SHUD.h"

#include "Character/EnemyCharacter.h"
#include "State/SPlayerState.h"
#include "UI/SUserWidgetBase.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/ItemPickupWidgetController.h" // 헤더 추가 필수!

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
		// 2. PS 안에 이미 만들어두신 함수들을 사용해 ASC와 AS를 가져옵니다.
		UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
		UAttributeSet* AS = PS->GetAttributeSet();

		// 3. 이제 모든 재료가 모였으니 파라미터 세트를 만듭니다.
		const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);

		// 4. 이 파라미터로 컨트롤러를 가져옵니다.
		UItemPickupWidgetController* WidgetController = GetItemPickupWidgetController(WidgetControllerParams);

		// 5. 상자 데이터를 주입합니다.

		// 6. 위젯 생성 및 화면 표시 (이전과 동일)
		LootWidget->SetWidgetController(WidgetController);
		WidgetController->SetLootItems(Items, TargetBox);

		LootWidget->AddToViewport();

		// 7. 마우스 커서 활성화
		PC->SetShowMouseCursor(true);
		PC->SetInputMode(FInputModeGameAndUI());
	}
}


