#include "scpUtils.hpp"
#include "ScpOperatorSysGen.hpp"

namespace scp
{
using namespace utils;
SCPOperatorSysGen::SCPOperatorSysGen(const std::unique_ptr<ScMemoryContext> &ctx, ScAddr addr): SCPOperatorElStr5(ctx, addr)
{
}


std::string SCPOperatorSysGen::GetTypeName()
{
  return "sys_gen";
}

sc_result SCPOperatorSysGen::Parse()
{
  return SCPOperatorElStr5::Parse();
}

sc_result SCPOperatorSysGen::Execute()
{
//  if (SC_RESULT_OK != ResetValues())
//    return SC_RESULT_ERROR;

  if (operands[0]->IsAssign())
  {
#ifdef SCP_DEBUG
    Utils::logSCPError(ms_context, "Operand 1 must have FIXED modifier", addr);
#endif
    FinishExecutionWithError();
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }
  if (operands[2]->IsAssign())
  {
#ifdef SCP_DEBUG
    Utils::logSCPError(ms_context, "Operand 3 must have FIXED modifier", addr);
#endif
    FinishExecutionWithError();
    return SC_RESULT_ERROR_INVALID_PARAMS;
  }
  if (operands[0]->IsFixed())
  {
    if (!operands[0]->GetValue().IsValid())
    {
#ifdef SCP_DEBUG
      Utils::logSCPError(ms_context, "Operand 1 has modifier FIXED, but has no value", addr);
#endif
      FinishExecutionWithError();
      return SC_RESULT_ERROR_INVALID_PARAMS;
    }
  }
  if (operands[2]->IsFixed())
  {
    if (!operands[2]->GetValue().IsValid())
    {
#ifdef SCP_DEBUG
      Utils::logSCPError(ms_context, "Operand 3 has modifier FIXED, but has no value", addr);
#endif
      FinishExecutionWithError();
      return SC_RESULT_ERROR_INVALID_PARAMS;
    }
  }
  std::cout<<"checked operand types"<<std::endl;
  ScAddr templateForGeneration = operands[0]->GetValue();
  ScTemplate scTemplateForGeneration;
  ScTemplateParams params;
  ScAddr nodeBeforeParams = operands[2]->GetValue();
  auto outerIter = ms_context->Iterator3(nodeBeforeParams, ScType::EdgeAccessConstPosPerm, ScType::Node);
  std::cout<<"before while"<<std::endl;
  while (outerIter->Next())
  {
    ScAddr pair = outerIter->Get(2);
    std::cout<<"keyIter->new"<<std::endl;
    auto keyIter = ms_context->Iterator5(pair, ScType::EdgeAccessConstPosPerm, ScType::Unknown, ScType::EdgeAccessConstPosPerm, Keynodes::rrel_1);
    std::cout<<"valueIter->new"<<std::endl;
    auto valueIter = ms_context->Iterator5(pair, ScType::EdgeAccessConstPosPerm, ScType::Unknown, ScType::EdgeAccessConstPosPerm, Keynodes::rrel_2);
    ScAddr key;
    ScAddr value;

    std::cout<<"keyIter->Next"<<std::endl;
    if (keyIter->Next())
      key = keyIter->Get(2);
    std::cout<<"valueIter->Next"<<std::endl;
    if (valueIter->Next())
      value = valueIter->Get(2);
    if (key.IsValid() && value.IsValid())
    {
      SCPOperand keyOperand = SCPOperand(ms_context, keyIter->Get(1));
      SCPOperand valueOperand = SCPOperand(ms_context, valueIter->Get(1));

      std::cout<<"add param"<<std::endl;
      params.Add(keyOperand.GetValue(), valueOperand.GetValue());
      std::cout<<"     OK"<<std::endl;
    }
    else
    {
      Utils::logSCPError(ms_context, "cannot find key or value for paramNode", pair);
    }
  }

  std::cout<<"build template"<<std::endl;
  ms_context->HelperBuildTemplate(scTemplateForGeneration, templateForGeneration);
  std::cout<<"     --- OK"<<std::endl;

  std::cout<<"generate template"<<std::endl;
  ScTemplateGenResult genResult;
  ms_context->HelperGenTemplate(scTemplateForGeneration, genResult, params);
  std::cout<<"     --- OK"<<std::endl;

  ScAddr generatedElements;
  if (operands[3]->IsFixed())
    generatedElements = operands[3]->GetValue();
  if (!generatedElements.IsValid())
  {
    generatedElements = ms_context->CreateNode(ScType::NodeConstStruct);
    operands[3]->ResetValue();
    operands[3]->SetValue(generatedElements);
  }
  std::cout<<"generated hash is " << generatedElements.Hash()<<std::endl;
  for (size_t i = 0; i < genResult.Size(); ++i)
  {
    ScAddr createdAddr;
    genResult.Get(i, createdAddr);
    ms_context->CreateEdge(ScType::EdgeAccessConstPosPerm, generatedElements, createdAddr);
  }
  if (operands[3] != nullptr)
  {
    std::cout<<"changing operand4 value"<<std::endl;
    operands[3]->ResetValue();
    operands[3]->SetValue(ms_context->CreateNode(ScType::NodeConst));
  }
  else
  {
    std::cout<<"operand4 was not passed"<<std::endl;
  }
  operands[1]->ResetValue();
  operands[1]->SetValue(ms_context->CreateNode(ScType::NodeConst));
  FinishExecutionSuccessfully();
  return SC_RESULT_OK;
}

}  // namespace scp













