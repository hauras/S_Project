
#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "Input/SInputConfig.h"
#include "SInputComponent.generated.h"

/**
 * 
 */
UCLASS()
class SPROJECT_API USInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()
public:

	template<class UserObject,typename CallbackFunc>
	void BindInputAction(const USInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent,UserObject* ContextObject,CallbackFunc Func);

};

template <class UserObject, typename CallbackFunc>
void USInputComponent::BindInputAction(const USInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserObject* ContextObject, CallbackFunc Func)
{
	checkf(InputConfig,TEXT("Input config data asset is null,can not proceed with binding"));

	if (UInputAction* FoundAction = InputConfig->FindNativeInputActionByTag(InputTag))
	{
		BindAction(FoundAction,TriggerEvent,ContextObject,Func);
	}
}
