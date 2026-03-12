

#include "UI/Widget/RoomIconWidget.h"
#include "Components/Image.h"
#include "Actor/Map/DungeonGenerator.h"

void URoomIconWidget::SetRoomTypeVisuals(ERoomType Type)
{
	if (!RoomImage) return;

	UTexture2D* SelectedTexture = NormalTexture;

	switch (Type)
	{
		case ERoomType::Start:    SelectedTexture = StartTexture; break;
		case ERoomType::Boss:     SelectedTexture = BossTexture; break;
		case ERoomType::Treasure: SelectedTexture = TreasureTexture; break;
	}

	// 시작 지점은 투명하게
	if (Type == ERoomType::Start)
	{
		RoomImage->SetRenderOpacity(0.0f); 
	}
	else
	{
		RoomImage->SetRenderOpacity(1.0f); 
	}
	
	if (SelectedTexture)
	{
		RoomImage->SetBrushFromTexture(SelectedTexture);
	}

	
}
