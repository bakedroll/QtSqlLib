#include "Logic.h"

namespace QtSqlLib
{

Logic::Logic(LogicalOperator op)
  : m_operator(op)
{
}

Logic::~Logic() = default;

QString Logic::toQueryString(
  API::ISchema& /*schema*/,
  std::vector<QVariant>& /*boundValuesOut*/,
  const OptionalIID& /*defaultTableId*/) const
{
  switch (m_operator)
  {
  case LogicalOperator::And:
    return "AND";
  case LogicalOperator::Or:
    return "OR";
  default:
    break;
  }

  assert(false);
  return "";
}

}