#include "../stdafx.h"
#include "BehaviorTree.h"

Elite::BehaviorTree::BehaviorTree(Blackboard * const pBlackBoard, IBehavior * const pRootBehavior): m_pBlackBoard(pBlackBoard), m_pRootBehavior(pRootBehavior){}

Elite::BehaviorTree::~BehaviorTree()
{
    SAFE_DELETE(m_pRootBehavior);
    SAFE_DELETE(m_pBlackBoard); //Takes ownership of passed blackboard!
}

void Elite::BehaviorTree::Update()
{
    if (m_pRootBehavior == nullptr)
    {
        m_CurrentState = BehaviorState::Failure;
        return;
    }

    m_CurrentState = m_pRootBehavior->Execute(m_pBlackBoard);
}

Elite::BehaviorConditional::BehaviorConditional(std::function<bool(Blackboard *)> fp): m_fpConditional(std::move(fp)){}

Elite::BehaviorState Elite::BehaviorConditional::Execute(Blackboard * const pBlackBoard)
{
    if (m_fpConditional == nullptr) return BehaviorState::Failure;
    m_CurrentState = m_fpConditional(pBlackBoard) ? BehaviorState::Success : BehaviorState::Failure;
    return m_CurrentState;
}

Elite::BehaviorAction::BehaviorAction(std::function<BehaviorState(Blackboard *)> fp): m_fpAction(std::move(fp)){}

Elite::BehaviorState Elite::BehaviorAction::Execute(Blackboard * const pBlackBoard)
{
    if (m_fpAction == nullptr)
        return BehaviorState::Failure;

    m_CurrentState = m_fpAction(pBlackBoard);
    return m_CurrentState;
}
