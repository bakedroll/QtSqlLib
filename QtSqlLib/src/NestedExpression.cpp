#include "NestedExpression.h"

#include "QtSqlLib/Expr.h"

namespace QtSqlLib
{

NestedExpression::NestedExpression(Expr& expr)
  : m_nestedExpr(std::make_unique<Expr>(std::move(expr)))
{
}

NestedExpression::~NestedExpression() = default;

QString NestedExpression::toQueryString(
  API::ISchema& schema,
  std::vector<QVariant>& boundValuesOut,
  const OptionalIID& defaultTableId) const
{
  return QString("(%1)").arg(m_nestedExpr->toQueryString(schema, boundValuesOut, defaultTableId));
}

}
