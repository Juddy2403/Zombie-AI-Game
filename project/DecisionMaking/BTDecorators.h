#pragma once
#include <utility>
#include "BehaviorTree.h"


namespace Elite
{
    //-----------------------------------------------------------------
	//DECORATORS
	//-----------------------------------------------------------------
	class BehaviorConditionDecorator final : public IBehavior
	{
	public:
		explicit BehaviorConditionDecorator(IBehavior* const childBehavior, std::function<bool(Blackboard*)> fp) :
		m_pChildBehavior(childBehavior), m_fpConditional(std::move(fp)) {}
		~BehaviorConditionDecorator() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
		IBehavior* m_pChildBehavior = nullptr;
	};

	class BehaviorBlackboardCondition final : public IBehavior
	{
	public:
		explicit BehaviorBlackboardCondition(IBehavior* const childBehavior,const std::string& blackboardKey, const bool invert = false) :
		m_pChildBehavior(childBehavior), m_BlackboardKey(blackboardKey), m_Invert(invert) {}
		~BehaviorBlackboardCondition() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		const std::string m_BlackboardKey{};
		bool m_Invert = false;
		IBehavior* m_pChildBehavior = nullptr;
	};

	class BehaviorInverter final : public IBehavior
	{
	public:
		explicit BehaviorInverter(IBehavior* const childBehavior) : m_pChildBehavior(childBehavior) {}
		~BehaviorInverter() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
	};

	class BehaviorConditionalInverter final : public IBehavior
	{
	public:
		explicit BehaviorConditionalInverter(std::function<bool(Blackboard*)> fp) : m_fpConditional(std::move(fp)) {}

		BehaviorState Execute(Blackboard* const pBlackBoard) override;

	private:
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};

	class BehaviorForceSuccess final : public IBehavior
	{
	public:
		explicit BehaviorForceSuccess(IBehavior* const childBehavior) : m_pChildBehavior(childBehavior) {}
		~BehaviorForceSuccess() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
	};

	class BehaviorForceFailure final : public IBehavior
	{
	public:
		explicit BehaviorForceFailure(IBehavior* const childBehavior) : m_pChildBehavior(childBehavior) {}
		~BehaviorForceFailure() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
	};

	class BehaviorRepeat : public IBehavior
	{
	public:
		explicit BehaviorRepeat(IBehavior* const childBehavior, int numCycles, bool tryToRunInOneFrame = false) : m_pChildBehavior(childBehavior), m_NumCycles(numCycles),
		m_TryToRunInOneFrame(tryToRunInOneFrame){}
		~BehaviorRepeat() override { SAFE_DELETE(m_pChildBehavior); }

		virtual BehaviorState Execute(Blackboard* const pBlackBoard) override;
	protected:
		void SetNumCycles(int numCycles) { m_NumCycles = numCycles; }
		IBehavior* m_pChildBehavior = nullptr;
		int m_NumCycles;
		int m_CurrentNumCycles = 0;
		bool m_TryToRunInOneFrame = false;
	};

	class BehaviorRepeatBlackboardValue final : public BehaviorRepeat
	{
	public:
		explicit BehaviorRepeatBlackboardValue(IBehavior * const childBehavior, const std::string &key,
		bool tryToRunInOneFrame = false): BehaviorRepeat(childBehavior, 0, tryToRunInOneFrame)
							  , m_BlackboardKey(key) {}

		~BehaviorRepeatBlackboardValue() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		std::string m_BlackboardKey{};
	};

	class BehaviorRetryUntilSuccessful final : public IBehavior
	{
	public:
		explicit BehaviorRetryUntilSuccessful(IBehavior* const childBehavior, int numCycles) : m_pChildBehavior(childBehavior), m_NumAttempts(numCycles) {}
		~BehaviorRetryUntilSuccessful() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
		const int m_NumAttempts;
		int m_CurrentNumAttempts = 0;
	};

	class BehaviorKeepRunningUntilFailure final : public IBehavior
	{
	public:
		explicit BehaviorKeepRunningUntilFailure(IBehavior* const childBehavior) : m_pChildBehavior(childBehavior) {}
		~BehaviorKeepRunningUntilFailure() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
	};

	class BehaviorRepeatUntil final : public IBehavior
	{
	public:
		explicit BehaviorRepeatUntil(IBehavior* const childBehavior, std::function<bool(Blackboard*)> fp) : m_pChildBehavior(childBehavior), m_fpConditional(std::move(fp)) {}
		~BehaviorRepeatUntil() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};
	class BehaviorAbortIf final : public IBehavior
	{
	public:
		explicit BehaviorAbortIf(IBehavior* const childBehavior, std::function<bool(Blackboard*)> fp) : m_pChildBehavior(childBehavior), m_fpConditional(std::move(fp)) {}
		~BehaviorAbortIf() override { SAFE_DELETE(m_pChildBehavior); }

		BehaviorState Execute(Blackboard* const pBlackBoard) override;
	private:
		IBehavior* m_pChildBehavior = nullptr;
		std::function<bool(Blackboard*)> m_fpConditional = nullptr;
	};
}
