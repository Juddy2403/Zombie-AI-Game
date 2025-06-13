#pragma once

#include <type_traits>
#include "SteeringHelpers.h"

class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(const AgentInfo &agent) = 0;

	//Seek Functions
	virtual void SetTarget(const TargetData& target) { m_Target = target; }
	virtual void SetRunning(bool isRunning);
	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	void RunStaminaCheck(const AgentInfo& agent);
	TargetData m_Target;
	bool m_IsRunning = false;
};

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;
};

///////////////////////////////////////
//FLEE
//****
class Flee : public Seek
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	//Flee Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;
};

///////////////////////////////////////
//FACE
//****
class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;
};

class FleeWhileFacing : public Flee
{
public:
	FleeWhileFacing() = default;
	virtual ~FleeWhileFacing() = default;

	//Flee Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;
};

///////////////////////////////////////
//PURSUIT
//****
class Pursuit : public Seek
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Pursuit Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;
};

///////////////////////////////////////
//EVADE
//****
class Evade final : public Pursuit
{
public:
	Evade() = default;
	Evade(float evasionRadius);
	virtual ~Evade() = default;

	//Evade Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;

private:
	float m_EvasionRadius = 10.f;
};

///////////////////////////////////////
//WANDER
//****
class Wander final : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behaviour
	SteeringOutput CalculateSteering(const AgentInfo &agent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float angle) { m_MaxAngleChange = angle; }
	void SetSlowRadius(float radius) { m_SlowRadius = radius; }
	void SetTargetRadius(float radius) { m_TargetRadius = radius; }

protected:
	float m_SlowRadius = 15.f;
	float m_TargetRadius = 3.f;

private:
	float m_OffsetDistance = 6.f;
	float m_Radius = 4.f;
	float m_MaxAngleChange = Elite::ToRadians(45);
	float m_WanderAngle = 0.f;
};
