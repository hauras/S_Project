

#include "Input/SInputConfig.h"

UInputAction* USInputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
	for (const FSInputActionConfig& InputActionConfig  : InputActions)
	{
		if (InputActionConfig.InputTag == InInputTag && InputActionConfig.InputAction)
		{
			return InputActionConfig.InputAction;
		}
	}
	return nullptr;
}
