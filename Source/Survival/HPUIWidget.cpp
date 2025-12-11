#include "HPUIWidget.h"
#include "Components/Image.h"

void UHPUIWidget::UpdateHearts(int32 CurrentHP)
{
    Heart1->SetVisibility(CurrentHP >= 1 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    Heart2->SetVisibility(CurrentHP >= 2 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    Heart3->SetVisibility(CurrentHP >= 3 ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}
