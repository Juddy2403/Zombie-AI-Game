#pragma once
#include "BehaviorTree.h"

namespace Elite
{
    //--- COMPOSITE BASE ---
    class BehaviorComposite : public Elite::IBehavior
    {
    public:
        explicit BehaviorComposite(const std::vector<IBehavior*>& childBehaviors)
        { m_ChildBehaviors = childBehaviors;	}

        ~BehaviorComposite() override
        {
            for (Elite::IBehavior* pb : m_ChildBehaviors)
                SAFE_DELETE(pb);
            m_ChildBehaviors.clear();
        }

        BehaviorState Execute(Blackboard* const pBlackBoard) override = 0;

    protected:
        std::vector<IBehavior*> m_ChildBehaviors = {};
        int m_PrevRunningIdx = 0;
    };

    //--- SELECTOR ---
    class BehaviorSelector final: public BehaviorComposite
    {
    public:
        explicit BehaviorSelector(const std::vector<IBehavior*>& childBehaviors) :
            BehaviorComposite(childBehaviors) {}

        ~BehaviorSelector() override = default;

        BehaviorState Execute(Blackboard* const pBlackBoard) override;
    };

    //--- SEQUENCE ---
    class BehaviorSequence : public BehaviorComposite
    {
    public:
        explicit BehaviorSequence(const std::vector<IBehavior*>& childBehaviors) :
            BehaviorComposite(childBehaviors) {}

        ~BehaviorSequence() override = default;

        BehaviorState Execute(Blackboard* const pBlackBoard) override;
    };

    //--- PARTIAL SEQUENCE ---
    class BehaviorPartialSequence final : public BehaviorSequence
    {
    public:
        explicit BehaviorPartialSequence(const std::vector<IBehavior*>& childBehaviors)
            : BehaviorSequence(childBehaviors) {}

        ~BehaviorPartialSequence() override = default;

        BehaviorState Execute(Blackboard* const pBlackBoard) override;

    private:
        unsigned int m_CurrentBehaviorIndex = 0;
    };
}
