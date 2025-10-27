

#include "UI/SUserWidgetBase.h"

void USUserWidgetBase::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	WidgetControllerSet();
}

