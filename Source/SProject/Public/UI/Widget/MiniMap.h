
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MiniMap.generated.h"

class UImage;
class UCanvasPanel;
/**
 * 
 */
UCLASS()
class SPROJECT_API UMiniMap : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "MiniMap")
	void UpdateMap();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MapCanvas;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> EntityCanvas;
	
	UPROPERTY(EditAnywhere, Category = "Minimap")
	TSubclassOf<UUserWidget> RoomIconClass;

	UPROPERTY(EditAnywhere, Category = "Minimap")
	float SlotSize = 100.f;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> PlayerMarker;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

};
