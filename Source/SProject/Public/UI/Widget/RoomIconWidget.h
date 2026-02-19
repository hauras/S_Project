
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RoomIconWidget.generated.h"

enum class ERoomType : uint8;
class UImage;

/**
 * 
 */
UCLASS()
class SPROJECT_API URoomIconWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetRoomTypeVisuals(ERoomType Type);

protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> RoomImage;

	// 에디터에서 설정할 아이콘 이미지들
	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> NormalTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> BossTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> TreasureTexture;

	UPROPERTY(EditDefaultsOnly, Category = "Minimap")
	TObjectPtr<UTexture2D> StartTexture;
};
