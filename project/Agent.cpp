#include "stdafx.h"
#include "IExamInterface.h"

#include "Agent.h"
#include "MapSearchSystem.h"
#include "DecisionMaking/BehaviorActions.h"
#include "DecisionMaking/BehaviorCondition.h"
#include "DecisionMaking/BehaviorTree.h"
#include "DecisionMaking/BTComposites.h"
#include "DecisionMaking/BTDecorators.h"
#include "Steering/CombinedSteeringBehaviors.h"
#include "Steering/SteeringBehaviors.h"
#include "Steering/SteeringHelpers.h"

Agent::Agent(IExamInterface *const pInterface): m_pInterface(pInterface)
{
    m_pEvadeBehavior = new Evade();
    m_pSeekSteeringBehavior = new Seek();
    m_pBlendedSeekAndEvadeSteeringBehavior = new BlendedSteering({
        {m_pSeekSteeringBehavior, 0.4f}, {m_pEvadeBehavior, 0.6f}
    });
    m_pWanderSteeringBehavior = new Wander();
    m_pFleeWhileFacingSteeringBehavior = new FleeWhileFacing();
    m_pBlendedSeekAndWanderSteeringBehavior = new BlendedSteering({
        {m_pSeekSteeringBehavior, 0.8f}, {m_pWanderSteeringBehavior, 0.2f}
    });
    m_pFaceBehavior = new Face();
    m_pPrioritySteeringBehavior = new PrioritySteering({
        m_pBlendedSeekAndWanderSteeringBehavior, m_pSeekSteeringBehavior, m_pFleeWhileFacingSteeringBehavior,
        m_pFaceBehavior, m_pWanderSteeringBehavior, m_pBlendedSeekAndEvadeSteeringBehavior
    });
    m_pMapSearch = new MapSearchSystem(pInterface->Agent_GetInfo());

    CreateBehaviorTree();
}

Agent::~Agent()
{
    SAFE_DELETE(m_pWanderSteeringBehavior);
    SAFE_DELETE(m_pSeekSteeringBehavior);
    SAFE_DELETE(m_pFleeWhileFacingSteeringBehavior);
    SAFE_DELETE(m_pBlendedSeekAndWanderSteeringBehavior);
    SAFE_DELETE(m_pPrioritySteeringBehavior);
    SAFE_DELETE(m_pBehaviorTree);
    SAFE_DELETE(m_pFaceBehavior);
}

void Agent::UpdateDebug(float dt)
{
    if (m_pInterface->Input_IsMouseButtonUp(Elite::InputMouseButton::eLeft))
    {
        //Update_Debug target based on input
        Elite::MouseData mouseData = m_pInterface->Input_GetMouseData(Elite::InputType::eMouseButton,
                                                                      Elite::InputMouseButton::eLeft);
        const Elite::Vector2 pos = Elite::Vector2(static_cast<float>(mouseData.X), static_cast<float>(mouseData.Y));
        m_MouseTarget = m_pInterface->Debug_ConvertScreenToWorld(pos);
    }
    //auto nextTargetPos = m_pInterface->NavMesh_GetClosestPathPoint(m_MouseTarget);
    //m_pWeightedSeekSteeringBehavior->SetTarget(nextTargetPos);
    // std::cout<< m_pInterface->Agent_GetInfo().Position <<"\n";
}

void Agent::Update(float dt)
{
    // check if the target has been reached
    m_pMapSearch->Update(dt);
    m_pBlackboard->GetData("currentTarget", m_CurrTarget);
    if (m_CurrTarget.has_value() && m_CurrTarget.value().DistanceSquared(m_pInterface->Agent_GetInfo().Position) <=
        16.f)
    {
        m_pMapSearch->ReachedTarget(m_CurrTarget.value());
        Elite::Vector2 targetPos;
        m_pMapSearch->GetCurrentTarget(m_pInterface->Agent_GetInfo().Position, targetPos);
        m_CurrTarget = targetPos;
    }
    m_pBlackboard->ChangeData("wasBitten", m_pInterface->Agent_GetInfo().WasBitten);
    SetChaseData(dt);
    m_pBehaviorTree->Update();
}

void Agent::RenderDebug(float dt) const
{
    m_pInterface->Draw_SolidCircle(m_MouseTarget, .7f, {0, 0}, {1, 0, 0});
    if (m_CurrTarget.has_value()) m_pInterface->Draw_SolidCircle(m_CurrTarget.value(), 2.f, {0, 0}, {1, 1, 1});

    Elite::Vector2 lastEnemyPos;
    m_pBlackboard->GetData("lastEnemyPos", lastEnemyPos);
    m_pInterface->Draw_SolidCircle(lastEnemyPos, 1.f, {0, 0}, {52.f / 255.f, 213.f / 255.f, 235.f / 255.f});
    m_pMapSearch->RenderDebug(m_pInterface);
}

SteeringOutput Agent::GetSteeringOutput(float dt)
{
    auto agentInfo = m_pInterface->Agent_GetInfo();
    auto steering = m_pPrioritySteeringBehavior->CalculateSteering(agentInfo);
    HandleRadarMode(dt, steering);
    return steering;
}

Elite::Blackboard *Agent::CreateBlackboard() const
{
    Elite::Blackboard *const pBlackboard = new Elite::Blackboard();
    pBlackboard->AddData("interface", m_pInterface);
    pBlackboard->AddData("prioritySteering", m_pPrioritySteeringBehavior);
    pBlackboard->AddData("isBeingChased", false);
    std::vector<EnemyInfo> enemiesInFov{};
    pBlackboard->AddData("enemiesInFovInfo", enemiesInFov);
    pBlackboard->AddData("lastEnemyPos", Elite::Vector2(0, 0));
    pBlackboard->AddData("shotLastFrame", false);
    pBlackboard->AddData("radarMode", false);
    pBlackboard->AddData("wasBitten", false);
    pBlackboard->AddData("mapSearch", m_pMapSearch);
    pBlackboard->AddData("currentTarget", m_CurrTarget);
    std::optional<eItemType> itemTarget;
    pBlackboard->AddData("targetItemType", itemTarget);
    std::map<eItemType, bool> itemNeedList{};
    itemNeedList[eItemType::PISTOL] = false;
    itemNeedList[eItemType::SHOTGUN] = false;
    itemNeedList[eItemType::FOOD] = false;
    itemNeedList[eItemType::MEDKIT] = false;
    pBlackboard->AddData("itemNeedList", itemNeedList);
    pBlackboard->AddData("itemSeekList", std::vector<ItemInfo>{});
    pBlackboard->AddData("itemsInFOV", 0);
    pBlackboard->AddData("currentItemInFOV", 0);
    return pBlackboard;
}

void Agent::CreateBehaviorTree()
{
    m_pBlackboard = CreateBlackboard();
    m_pBehaviorTree = new Elite::BehaviorTree(
        m_pBlackboard,
        new Elite::BehaviorSelector({
            // Use item
            new Elite::BehaviorForceFailure(new Elite::BehaviorAction(BT_Actions::UseItemIfNeeded)),
            // Purge zone action
            new Elite::BehaviorConditionDecorator(new Elite::BehaviorAction(BT_Actions::FleePurgeZone),
                                                  BT_Conditions::IsInPurgeZone),
            // Was bitten this frame action
            new Elite::BehaviorForceFailure(
            new Elite::BehaviorBlackboardCondition(new Elite::BehaviorSequence({
                                                       new Elite::BehaviorAction(BT_Actions::SetIsBeingChased),
                                                       new Elite::BehaviorAction(BT_Actions::SetEnemyBehindPos),
                                                       new Elite::BehaviorAction(BT_Actions::SetRunModeTrue)
                                                   }),
                                                   "wasBitten")),
            // Enemy actions
            new Elite::BehaviorBlackboardCondition(new Elite::BehaviorSelector({
                                                       new Elite::BehaviorConditionDecorator(
                                                           new Elite::BehaviorForceSuccess(new Elite::BehaviorSequence({
                                                               new Elite::BehaviorAction(BT_Actions::FaceAndFleeEnemy),
                                                               new Elite::BehaviorAction(
                                                                   BT_Actions::ShotLastFrameActions),
                                                               new Elite::BehaviorConditional(
                                                                   BT_Conditions::IsFacingEnemy),
                                                               new Elite::BehaviorAction(BT_Actions::Shoot),
                                                               new Elite::BehaviorConditional(
                                                                   BT_Conditions::RanOutOfBullets),
                                                               new Elite::BehaviorAction(BT_Actions::DiscardWeapon),
                                                           })), BT_Conditions::CanGoForKill),
                new Elite::BehaviorForceFailure(new Elite::BehaviorAction(BT_Actions::GoIntoRadarMode)),
                                                       new Elite::BehaviorConditionDecorator(
                                                           new Elite::BehaviorAction(BT_Actions::FleeEnemy),
                                                           BT_Conditions::IsEnemyInHouse),
                                                       new Elite::BehaviorConditionDecorator(
                                                           new Elite::BehaviorAction(BT_Actions::EvadeInHouseInFOV),
                                                           BT_Conditions::HasHouseInFOV),
                                                       new Elite::BehaviorConditionDecorator(
                                                           new Elite::BehaviorAction(
                                                               BT_Actions::EvadeToClosestRememberedHouse),
                                                           BT_Conditions::RemembersAnyHouse),
                                                       new Elite::BehaviorAction(BT_Actions::FleeEnemy)
                                                   }), "isBeingChased"),
            // Item in FOV actions
             new Elite::BehaviorConditionDecorator(new Elite::BehaviorSelector({
                                                      // Has garbage in FOV and one empty slot
                                                       new Elite::BehaviorConditionDecorator(
                                                           new Elite::BehaviorSequence({
                                                               new Elite::BehaviorAction(
                                                                   BT_Actions::SeekGarbageInGrabRange),
                                                               new Elite::BehaviorForceSuccess( new Elite::BehaviorConditionDecorator(
                                                                   new Elite::BehaviorAction(BT_Actions::RemoveGarbage),
                                                                   BT_Conditions::IsGarbageInGrabRange))
                                                           }),
                                                           BT_Conditions::HasGarbageInFOVAndOneEmptySlot),
                                                       new Elite::BehaviorSequence({
                                                           new Elite::BehaviorAction(BT_Actions::CheckoutItems),
                                                           new Elite::BehaviorAction(
                                                               BT_Actions::SeekFirstItemInSeekList),
                                                           new Elite::BehaviorForceSuccess(
                                                               new Elite::BehaviorConditionDecorator(
                                                                   new Elite::BehaviorAction(BT_Actions::GrabItem),
                                                                   BT_Conditions::IsItemInGrabRange))
                                                       })

                                                   }), BT_Conditions::HasItemInFOV),
             // On seek list not empty
             new Elite::BehaviorConditionDecorator(new Elite::BehaviorSequence({
                                                       new Elite::BehaviorAction(
                                                           BT_Actions::SeekFirstItemInSeekList),
                                                       new Elite::BehaviorForceSuccess(
                                                           new Elite::BehaviorConditionDecorator(
                                                               new Elite::BehaviorAction(BT_Actions::GrabItem),
                                                               BT_Conditions::IsItemInGrabRange))
                                                   }), BT_Conditions::IsSeekListNotEmpty),
             // Item search actions
             new Elite::BehaviorSequence({
                 new Elite::BehaviorAction(BT_Actions::CheckItemNeeds),
                 new Elite::BehaviorAction(BT_Actions::SetPossibleItemTarget),
                 new Elite::BehaviorConditionDecorator(new Elite::BehaviorAction(BT_Actions::SeekTargetItem),
                                                       BT_Conditions::IsTargetItemSet)
             }),
            // House actions
             new Elite::BehaviorConditionDecorator(new Elite::BehaviorAction(BT_Actions::CheckoutHouse),
                                                   BT_Conditions::HasUncheckedHouseInFOV),
             // Next target actions
             new Elite::BehaviorSequence({
                 new Elite::BehaviorAction(BT_Actions::SetNextTarget),
                 new Elite::BehaviorAction(BT_Actions::GoToNextTarget),
                 new Elite::BehaviorForceSuccess(new Elite::BehaviorConditionDecorator(
                     new Elite::BehaviorAction(BT_Actions::GoIntoRadarMode), BT_Conditions::IsInHouse
                 ))
             }),
            // If nothing else to be done, chill in house
            new Elite::BehaviorAction(BT_Actions::Wander)
            // DEBUG Steering
           // new Elite::BehaviorAction(BT_Actions::SetDebugSteering)
        })
    );
}

void Agent::SetChaseData(float dt)
{
    bool isBeingChased;
    m_pBlackboard->GetData("isBeingChased", isBeingChased);
    //std::cout<<isBeingChased<<"\n";
    if (m_pInterface->FOV_GetStats().NumEnemies > 0)
    {
        m_CurrChaseTime = 0.f;
        m_pBlackboard->ChangeData("isBeingChased", true);
        m_pBlackboard->ChangeData("lastEnemyPos", m_pInterface->GetEnemiesInFOV()[0].Location);
        return;
    }
    if (isBeingChased)
    {
        m_CurrChaseTime += dt;
        if (m_CurrChaseTime > m_MaxChaseTime)
        {
            m_CurrChaseTime = 0.f;
            m_pBlackboard->ChangeData("isBeingChased", false);
        }
    }
}

void Agent::HandleRadarMode(float dt, SteeringOutput &steeringOutput) const
{
    bool radarMode;
    m_pBlackboard->GetData("radarMode", radarMode);

    if (radarMode)
    {
        steeringOutput.AutoOrient = false;
        steeringOutput.AngularVelocity = m_pInterface->Agent_GetInfo().MaxAngularSpeed;
    }
    //reset at the end of the frame
    m_pBlackboard->ChangeData("radarMode", false);
}
