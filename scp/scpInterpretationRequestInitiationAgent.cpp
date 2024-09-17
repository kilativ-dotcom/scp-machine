#include "scpInterpretationRequestInitiationAgent.hpp"

#include "scpKeynodes.hpp"
#include "scpUtils.hpp"

#include <chrono>

namespace scp
{
ScResult SCPInterpretationRequestInitiationAgent::DoProgram(ScElementaryEvent const & event, ScAction & action)
{
  auto const & startTime = std::chrono::high_resolution_clock::now();
  sc_uint32 const maxCustomerWaitingTime = action.GetMaxCustomerWaitingTime();

  ScAddr const & agentProgram = GetAgentProgram();
  if (!m_context.IsElement(agentProgram))
  {
    SC_AGENT_LOG_ERROR("Agent program is not valid");
    action.FinishUnsuccessfully();
  }

  ScAddr const & scpParams = m_context.GenerateNode(ScType::NodeConst);
  ScAddr const & firstArgumentArc =
      m_context.GenerateConnector(ScType::EdgeAccessConstPosPerm, scpParams, agentProgram);
  m_context.GenerateConnector(ScType::EdgeAccessConstPosPerm, Keynodes::rrel_1, firstArgumentArc);

  ScAddr const & secondArgumentArc =
      m_context.GenerateConnector(ScType::EdgeAccessConstPosPerm, scpParams, event.GetConnector());
  m_context.GenerateConnector(ScType::EdgeAccessConstPosPerm, Keynodes::rrel_2, secondArgumentArc);

  ScAction scpAction = m_context.GenerateAction(Keynodes::action_scp_interpretation_request);
  scpAction.SetArguments(agentProgram, scpParams);

  ScAddr const & authorArc =
      m_context.GenerateConnector(ScType::EdgeDCommonConst, scpAction, Keynodes::abstract_scp_machine);
  m_context.GenerateConnector(ScType::EdgeAccessConstPosPerm, Keynodes::nrel_authors, authorArc);

  if (maxCustomerWaitingTime == 0)
    scpAction.InitiateAndWait();
  else
  {
    auto const & timeFromStart = scp::Utils::GetTimeFromStart(startTime);
    if (timeFromStart < maxCustomerWaitingTime)
      scpAction.InitiateAndWait(maxCustomerWaitingTime - timeFromStart);
    else
    {
      SC_AGENT_LOG_WARNING(
          "Max customer waiting time"
          << maxCustomerWaitingTime
          << " has expired before action of class `action_scp_interpretation_request` was initiated because"
          << timeFromStart << " ms have passed");
      scpAction.InitiateAndWait();
    }
  }
  auto const & resIt = m_context.CreateIterator5(
      action, ScType::EdgeDCommonConst, ScType::NodeConst, ScType::EdgeAccessConstPosPerm, ScKeynodes::nrel_result);
  if (resIt->Next())
  {
    ScAddr const & actionResult = resIt->Get(2);
    action.SetResult(actionResult);
  }
  return action.FinishSuccessfully();
}

ScAddr SCPInterpretationRequestInitiationAgent::GetAgentProgram() const
{
  if (!m_context.IsElement(m_agentImplementationAddr))
  {
    SC_AGENT_LOG_ERROR("Agent implementation node is not valid");
    return ScAddr::Empty;
  }

  ScAddr agentProgram;
  auto const & programsTupleIterator = m_context.CreateIterator5(
      ScType::NodeConst,
      ScType::EdgeDCommonConst,
      m_agentImplementationAddr,
      ScType::EdgeAccessConstPosPerm,
      Keynodes::nrel_sc_agent_program);
  while (programsTupleIterator->Next())
  {
    ScIterator3Ptr programsIterator =
        m_context.CreateIterator3(programsTupleIterator->Get(0), ScType::EdgeAccessConstPosPerm, ScType::NodeConst);
    while (programsIterator->Next())
    {
      if (m_context.CheckConnector(
              Keynodes::agent_scp_program, programsIterator->Get(2), ScType::EdgeAccessConstPosPerm))
      {
        agentProgram = programsIterator->Get(2);
        break;
      }
    }
  }
  if (!agentProgram.IsValid())
  {
    SC_AGENT_LOG_ERROR(
        "Not found program for sc-agent `" << m_context.GetElementSystemIdentifier(GetAbstractAgent()) << "`");
    return ScAddr::Empty;
  }

  // Old SCP program check
  auto const & programKeyElementIterator = m_context.CreateIterator5(
      agentProgram,
      ScType::EdgeAccessConstPosPerm,
      ScType::NodeVar,
      ScType::EdgeAccessConstPosPerm,
      Keynodes::rrel_key_sc_element);
  if (!programKeyElementIterator->Next())
  {
    SC_AGENT_LOG_ERROR(
        "Not found process variable in program for sc-agent `"
        << m_context.GetElementSystemIdentifier(GetAbstractAgent()) << "`");
    return ScAddr::Empty;
  }
  return agentProgram;
}

bool SCPInterpretationRequestInitiationAgent::CheckInitiationCondition(ScElementaryEvent const & event)
{
  ScAddr const & action = event.GetOtherElement();
  auto const & actionAuthorIterator = m_context.CreateIterator5(
      action,
      ScType::EdgeDCommonConst,
      Keynodes::abstract_scp_machine,
      ScType::EdgeAccessConstPosPerm,
      Keynodes::nrel_authors);
  if (actionAuthorIterator->Next())
    return false;
  if (!m_context.CheckConnector(GetActionClass(), action, ScType::EdgeAccessConstPosPerm))
    return false;
  return true;
}
}  // namespace scp
