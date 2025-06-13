#pragma once

#include <functional>
#include "Blackboard.h"

namespace Elite
{
    //-----------------------------------------------------------------
    // BEHAVIOR TREE HELPERS
    //-----------------------------------------------------------------
    enum class BehaviorState
    {
        Failure,
        Success,
        Running
    };

    //-----------------------------------------------------------------
    // BEHAVIOR INTERFACES (BASE)
    //-----------------------------------------------------------------
    class IBehavior
    {
    public:
        IBehavior() = default;

        virtual ~IBehavior() = default;

        virtual BehaviorState Execute(Blackboard *const pBlackBoard) = 0;

    protected:
        BehaviorState m_CurrentState = BehaviorState::Failure;
    };

    //-----------------------------------------------------------------
    // BEHAVIOR TREE (BASE)
    //-----------------------------------------------------------------
    class BehaviorTree final
    {
    public:
        explicit BehaviorTree(Blackboard *const pBlackBoard, IBehavior *const pRootBehavior);;

        ~BehaviorTree();

        void Update();

        Blackboard *GetBlackboard() const
        {
            return m_pBlackBoard;
        }

    private:
        BehaviorState m_CurrentState = BehaviorState::Failure;
        Blackboard *m_pBlackBoard = nullptr;
        IBehavior *m_pRootBehavior = nullptr;
    };

    //-----------------------------------------------------------------
    // BEHAVIOR TREE CONDITIONAL (IBehavior)
    //-----------------------------------------------------------------
    class BehaviorConditional final : public IBehavior
    {
    public:
        explicit BehaviorConditional(std::function<bool(Blackboard *)> fp);

        BehaviorState Execute(Blackboard *const pBlackBoard) override;

    private:
        std::function<bool(Blackboard *)> m_fpConditional = nullptr;
    };

    //-----------------------------------------------------------------
    // BEHAVIOR TREE ACTION (IBehavior)
    //-----------------------------------------------------------------
    class BehaviorAction final : public IBehavior
    {
    public:
        explicit BehaviorAction(std::function<BehaviorState(Blackboard *)> fp);

        BehaviorState Execute(Blackboard *const pBlackBoard) override;

    private:
        std::function<BehaviorState(Blackboard *)> m_fpAction = nullptr;
    };
}
