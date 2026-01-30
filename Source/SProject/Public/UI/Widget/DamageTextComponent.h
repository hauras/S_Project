
#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DamageTextComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API UDamageTextComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage);

	UFUNCTION(BlueprintImplementableEvent)
	void SetDamageColor(FLinearColor NewColor);
};
