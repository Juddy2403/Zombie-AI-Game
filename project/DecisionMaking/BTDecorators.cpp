#include "../stdafx.h"
#include "BTDecorators.h"

using namespace Elite;

BehaviorState BehaviorConditionDecorator::Execute(Blackboard * const pBlackBoard)
{
    if (m_fpConditional == nullptr) return BehaviorState::Failure;
    if (!m_fpConditional(pBlackBoard)) return BehaviorState::Failure;
    return m_pChildBehavior->Execute(pBlackBoard);
}

BehaviorState BehaviorBlackboardCondition::Execute(Blackboard * const pBlackBoard)
{
    bool condition;
    pBlackBoard->GetData(m_BlackboardKey, condition);
    condition = m_Invert ? !condition : condition;
    if (!condition) return BehaviorState::Failure;
    return m_pChildBehavior->Execute(pBlackBoard);
}

BehaviorState Elite::BehaviorInverter::Execute(Blackboard *const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    switch (state)
    {
        case Elite::BehaviorState::Failure:
            return BehaviorState::Success;

        case Elite::BehaviorState::Success:
            return BehaviorState::Failure;
        default:
            return state;
    }
}

BehaviorState BehaviorConditionalInverter::Execute(Blackboard *const pBlackBoard)
{
    if (m_fpConditional == nullptr) return BehaviorState::Failure;
    m_CurrentState = m_fpConditional(pBlackBoard) ? BehaviorState::Failure : BehaviorState::Success;

    return m_CurrentState;
}

BehaviorState BehaviorForceSuccess::Execute(Blackboard *const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    switch (state)
    {
        case Elite::BehaviorState::Failure:
        case Elite::BehaviorState::Success:
            return BehaviorState::Success;
        case Elite::BehaviorState::Running:
            return BehaviorState::Running;
        default:
            return state;
    }
}

BehaviorState BehaviorForceFailure::Execute(Blackboard *const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    switch (state)
    {
        case Elite::BehaviorState::Failure:
        case Elite::BehaviorState::Success:
            return BehaviorState::Failure;
        case Elite::BehaviorState::Running:
            return BehaviorState::Running;
        default:
            return state;
    }
}

BehaviorState BehaviorRepeat::Execute(Blackboard *const pBlackBoard)
{
    if (m_TryToRunInOneFrame)
    {
        const int start = m_CurrentNumCycles;
        for (int i = m_CurrentNumCycles; i < m_NumCycles; ++i)
        {
            ++m_CurrentNumCycles;
            BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
            if (state == Elite::BehaviorState::Failure) return BehaviorState::Failure;
            if (state == Elite::BehaviorState::Running) return BehaviorState::Running;
        }
        m_CurrentNumCycles = 0;
        if (start == 0) return BehaviorState::Success;
    }
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    switch (state)
    {
        case Elite::BehaviorState::Failure:
            return BehaviorState::Failure;
        case Elite::BehaviorState::Success:
        {
            if (m_CurrentNumCycles++ >= m_NumCycles)
            {
                m_CurrentNumCycles = 0;
                return BehaviorState::Success;
            }
            return BehaviorState::Running;
        }
        case Elite::BehaviorState::Running:
            return BehaviorState::Running;
        default:
            return state;
    }
}

BehaviorState BehaviorRepeatBlackboardValue::Execute(Blackboard * const pBlackBoard)
{
    int numCycles;
    pBlackBoard->GetData(m_BlackboardKey, numCycles);
    BehaviorRepeat::SetNumCycles(numCycles);
    return BehaviorRepeat::Execute(pBlackBoard);

}

BehaviorState BehaviorRetryUntilSuccessful::Execute(Blackboard *const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    switch (state)
    {
        case Elite::BehaviorState::Failure:
        {
            ++m_CurrentNumAttempts;
            if (m_CurrentNumAttempts >= m_NumAttempts)
            {
                m_CurrentNumAttempts = 0;
                return BehaviorState::Failure;
            }
            return BehaviorState::Running;
        }
        case Elite::BehaviorState::Success:
            return BehaviorState::Success;

        case Elite::BehaviorState::Running:
            return BehaviorState::Running;
        default:
            return state;
    }
}

BehaviorState BehaviorKeepRunningUntilFailure::Execute(Blackboard *const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    switch (state)
    {
        case Elite::BehaviorState::Running:
        case Elite::BehaviorState::Success:
            return BehaviorState::Running;
        case Elite::BehaviorState::Failure:
            return BehaviorState::Failure;
        default:
            return state;
    }
}

BehaviorState BehaviorRepeatUntil::Execute(Blackboard *const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    if (m_fpConditional == nullptr) return BehaviorState::Failure;

    switch (state)
    {
        case Elite::BehaviorState::Failure:
            return BehaviorState::Failure;
        case Elite::BehaviorState::Success:
            if (m_fpConditional(pBlackBoard))
                return BehaviorState::Success;
            else
                return BehaviorState::Running;
        case Elite::BehaviorState::Running:
            return BehaviorState::Running;
    }
    return BehaviorState::Failure;
}

BehaviorState BehaviorAbortIf::Execute(Blackboard * const pBlackBoard)
{
    BehaviorState state = m_pChildBehavior->Execute(pBlackBoard);
    if (m_fpConditional == nullptr || m_fpConditional(pBlackBoard)) return BehaviorState::Failure;

    return state;
}