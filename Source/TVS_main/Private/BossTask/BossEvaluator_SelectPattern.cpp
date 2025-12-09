// Fill out your copyright notice in the Description page of Project Settings.


#include "BossTask/BossEvaluator_SelectPattern.h"
#include "BossTask/BossAIController.h"      // 컨트롤러 헤더 경로 확인 필요
#include "BossTask/BossCharacter.h" // 캐릭터 헤더 경로 확인 필요
#include "TVS_main.h"
#include "StateTreeExecutionContext.h"

void UBossEvaluator_SelectPattern::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
    AActor* Owner = Cast<AActor>(Context.GetOwner());
    ABossCharacter* Boss = Cast<ABossCharacter>(Owner);
    if (!Boss) return;

    ABossAIController* BossAI = Cast<ABossAIController>(Boss->GetController());
    if (!BossAI) return;

    // -------------------------------------------------------
    // [1] 후보군 선정: "어떤 리스트를 쓸까?"
    // -------------------------------------------------------
    TArray<UBossAttackData*> Candidates;

    // A. 콤보 중인가? (이전 기술의 후속타 목록 사용)
    if (BossAI->IsComboActive() && BossAI->LastUsedPattern)
    {
        Candidates = BossAI->LastUsedPattern->LinkedPatterns;
    }
    // B. 콤보가 아님 (Idle) -> "StartPattern(스킬셋)" 안의 목록 사용!
    else
    {
        if (Boss->StartPattern)
        {
            // "스타터 팩아, 네 안에 들어있는 1타 기술들 좀 줘"
            Candidates = Boss->StartPattern->LinkedPatterns;
        }
    }

    // -------------------------------------------------------
    // [2] 컨트롤러 호출: "이 중에서 골라줘"
    // -------------------------------------------------------

    // 후보가 없으면(콤보 끝 or 스킬셋 없음) -> 종료
    if (Candidates.Num() == 0)
    {
        BossAI->ResetCombo();
        OutSelectedPatternData = nullptr;
        OutCategory = EBossPatternCategory::None;
        return;
    }

    // 컨트롤러가 필터링 & 랜덤 추첨 수행
    UBossAttackData* BestPick = BossAI->SelectAttackPattern(Candidates);

    // -------------------------------------------------------
    // [3] 결과 반영: State Tree 변수에 넣기
    // -------------------------------------------------------
    if (BestPick)
    {
        OutSelectedPatternData = BestPick;
        OutCategory = BestPick->PatternCategory;
    }
    else
    {
        // 조건 맞는 게 없으면 콤보 끊김
        BossAI->ResetCombo();
        OutSelectedPatternData = nullptr;
        OutCategory = EBossPatternCategory::None;
    }
}