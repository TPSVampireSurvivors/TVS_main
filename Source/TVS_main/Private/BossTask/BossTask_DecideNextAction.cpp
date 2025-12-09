#include "BossTask/BossTask_DecideNextAction.h"
#include "BossTask/BossAIController.h"
#include "BossTask/BossCharacter.h"
#include "TVS_main.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus UBossTask_DecideNextAction::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition)
{
	// 1. 필요한 객체 가져오기
	AActor* Owner = Cast<AActor>(Context.GetOwner());
	ABossCharacter* Boss = Cast<ABossCharacter>(Owner);
	if (!Boss) return EStateTreeRunStatus::Failed;

	ABossAIController* BossAI = Cast<ABossAIController>(Boss->GetController());
	if (!BossAI) return EStateTreeRunStatus::Failed;

	// -------------------------------------------------------
	// [1] 후보군 선정
	// -------------------------------------------------------
	TArray<UBossAttackData*> Candidates;

	// A. 콤보 중인가? (이전 기술의 후속타 목록)
	if (BossAI->IsComboActive() && BossAI->LastUsedPattern)
	{
		Candidates = BossAI->LastUsedPattern->LinkedPatterns;
	}
	// B. 콤보 아님 -> 시작 패턴(StartPattern)의 목록
	else
	{
		if (Boss->StartPattern)
		{
			Candidates = Boss->StartPattern->LinkedPatterns;
		}
	}

	// -------------------------------------------------------
	// [2] 예외 처리 (후보 없음)
	// -------------------------------------------------------
	if (Candidates.Num() == 0)
	{
		BossAI->ResetCombo();

		// 출력 비우기
		OutAttackData = nullptr;
		OutCategory = EBossPatternCategory::None;

		// 실패 리턴 -> State Tree가 Idle로 보내도록 유도
		return EStateTreeRunStatus::Failed;
	}

	// -------------------------------------------------------
	// [3] 컨트롤러에게 선택 위임
	// -------------------------------------------------------
	UBossAttackData* BestPick = BossAI->SelectAttackPattern(Candidates);

	//UE_LOG(LogBossTest, Warning, TEXT("Final Decision: %s"), BestPick ? *BestPick->GetName() : TEXT("None"));

	if (BestPick)
	{
		// [성공] 찾았다!
		OutAttackData = BestPick;
		OutCategory = BestPick->PatternCategory;

		// 성공 리턴 -> State Tree가 Category를 보고 분기하도록 유도
		return EStateTreeRunStatus::Succeeded;
	}
	else
	{
		// [실패] 조건(거리 등) 맞는 게 없음
		BossAI->ResetCombo();

		OutAttackData = nullptr;
		OutCategory = EBossPatternCategory::None;

		return EStateTreeRunStatus::Failed;
	}
}