

#include "UI/HUD/SHUD.h"
#include "UI/SUserWidgetBase.h"
#include "UI/WidgetController/OverlayWidgetController.h"

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
