#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HPUIWidget.generated.h"

UCLASS()
class SURVIVAL_API UHPUIWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    class UImage* Heart1;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart2;

    UPROPERTY(meta = (BindWidget))
    class UImage* Heart3;

public:
    void UpdateHearts(int32 CurrentHP);
};