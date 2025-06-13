#include "../stdafx.h"
#include "BTComposites.h"

using namespace Elite;
//SELECTOR
BehaviorState BehaviorSelector::Execute(Blackboard *const pBlackBoard)
{
    // BT TODO:
    //TODO: Fill in this code
    // Loop over all children in m_ChildBehaviors
    for (int i{m_PrevRunningIdx}; i < static_cast<int>(m_ChildBehaviors.size()); ++i)
    {
        auto *childBehavior = m_ChildBehaviors[i];
        //Check the current state and apply the selector Logic:
        m_CurrentState = childBehavior->Execute(pBlackBoard);

        switch (m_CurrentState)
        {
            //The selector fails if all children failed.
            case BehaviorState::Failure:
                m_PrevRunningIdx = 0;
                continue;
            //if a child returns Running:
            //Running: stop looping and return Running
            case BehaviorState::Running:
                m_PrevRunningIdx = i;
                return m_CurrentState;
            //if a child returns Success:
            //stop looping over all children and return Success
            case BehaviorState::Success:
                m_PrevRunningIdx = 0;
                return m_CurrentState;
        }
    }
    //All children failed
    m_PrevRunningIdx = 0;
    m_CurrentState = BehaviorState::Failure;
    return m_CurrentState;
}

//SEQUENCE
BehaviorState BehaviorSequence::Execute(Blackboard *const pBlackBoard)
{
    // BT TODO:
    //TODO: FIll in this code
    //Loop over all children in m_ChildBehaviors
    for (int i{m_PrevRunningIdx}; i < static_cast<int>(m_ChildBehaviors.size()); ++i)
    {
        auto *childBehavior = m_ChildBehaviors[i];
        //Every Child: Execute and store the result in m_CurrentState
        //Check the current state and apply the sequence Logic:
        m_CurrentState = childBehavior->Execute(pBlackBoard);
        switch (m_CurrentState)
        {
            //if a child returns Failed:
            //stop looping over all children and return Failed
            case BehaviorState::Failure:
                m_PrevRunningIdx = 0;
                return m_CurrentState;
            //if a child returns Running:
            //Running: stop looping and return Running
            case BehaviorState::Running:
                m_PrevRunningIdx = i;
                return m_CurrentState;
            default: break;
        }
    }
    //All children succeeded
    m_PrevRunningIdx = 0;
    m_CurrentState = BehaviorState::Success;
    return m_CurrentState;
}

//PARTIAL SEQUENCE
BehaviorState BehaviorPartialSequence::Execute(Blackboard *const pBlackBoard)
{
    while (m_CurrentBehaviorIndex < static_cast<int>(m_ChildBehaviors.size()))
    {
        m_CurrentState = m_ChildBehaviors[m_CurrentBehaviorIndex]->Execute(pBlackBoard);
        switch (m_CurrentState)
        {
            case BehaviorState::Failure:
                m_CurrentBehaviorIndex = 0;
                return m_CurrentState;
            case BehaviorState::Success:
                ++m_CurrentBehaviorIndex;
                m_CurrentState = BehaviorState::Running;
                return m_CurrentState;
            case BehaviorState::Running:
                return m_CurrentState;
        }
    }

    m_CurrentBehaviorIndex = 0;
    m_CurrentState = BehaviorState::Success;
    return m_CurrentState;
}
