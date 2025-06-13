#pragma once
#include "Exam_HelperStructs.h"
#include "EliteMath/EVector2.h"

//SteeringParams (alias TargetData)
struct SteeringParams final //Also used as Target for SteeringBehaviors
{
	Elite::Vector2 Position;
	float Orientation;

	Elite::Vector2 LinearVelocity;
	float AngularVelocity;

	SteeringParams(const Elite::Vector2& position = Elite::ZeroVector2, float orientation = 0.f,
		const Elite::Vector2& linearVel = Elite::ZeroVector2, float angularVel = 0.f) :
		Position(position),
		Orientation(orientation),
		LinearVelocity(linearVel),
		AngularVelocity(angularVel) {}

#pragma region Operator Overloads
	SteeringParams(const SteeringParams& other)
	{
		Position = other.Position;
		Orientation = other.Orientation;
		LinearVelocity = other.LinearVelocity;
		AngularVelocity = other.AngularVelocity;
	}

	SteeringParams& operator=(const SteeringParams& other)
	{
		Position = other.Position;
		Orientation = other.Orientation;
		LinearVelocity = other.LinearVelocity;
		AngularVelocity = other.AngularVelocity;

		return *this;
	}

	bool operator==(const SteeringParams& other) const
	{
		return Position == other.Position && Orientation == other.Orientation && LinearVelocity == other.LinearVelocity && AngularVelocity == other.AngularVelocity;
	}

	bool operator!=(const SteeringParams& other) const
	{
		return Position != other.Position || Orientation != other.Orientation || LinearVelocity != other.LinearVelocity || AngularVelocity != other.AngularVelocity;
	}
#pragma endregion

};
using TargetData = SteeringParams; //Alias for SteeringBehavior usage (Bit clearer in its context ;) )

//SteeringOutput
struct SteeringOutput final : SteeringPlugin_Output
{
	SteeringOutput(const Elite::Vector2& linVel = { 0.f,0.f }, float angVel = 0.f)
	{
		LinearVelocity = linVel;
		AngularVelocity = angVel;
	}

	SteeringOutput& operator=(const SteeringOutput& other)
	{
		LinearVelocity = other.LinearVelocity;
		AngularVelocity = other.AngularVelocity;
		AutoOrient = other.AutoOrient;
		RunMode = other.RunMode;
		return *this;
	}

	SteeringOutput& operator+(const SteeringOutput& other)
	{
		LinearVelocity += other.LinearVelocity;
		AngularVelocity += other.AngularVelocity;

		return *this;
	}

	SteeringOutput& operator*=(const SteeringOutput& other)
	{
		LinearVelocity = LinearVelocity * other.LinearVelocity;
		AngularVelocity = AngularVelocity * other.AngularVelocity;

		return *this;
	}

	SteeringOutput& operator*=(float f)
	{
		LinearVelocity = f * LinearVelocity;
		AngularVelocity = f * AngularVelocity;

		return *this;
	}

	SteeringOutput& operator/=(float f)
	{
		LinearVelocity = LinearVelocity / f;
		AngularVelocity = AngularVelocity / f;

		return *this;
	}
};
