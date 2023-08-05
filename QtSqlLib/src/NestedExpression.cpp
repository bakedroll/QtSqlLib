#include "NestedExpression.h"

#include "QtSqlLib/Expr.h"

namespace QtSqlLib
{

NestedExpression::NestedExpression(Expr& expr)
  : m_nestedExpr(std::make_unique<Expr>(std::move(expr)))
{
}

NestedExpression::~NestedExpression() = default;

QString NestedExpression::toQString(API::ISchema& schema, const OptionalIID& defaultTableId) const
{
  return QString("(%1)").arg(m_nestedExpr->toQString(schema, defaultTableId));
}

}
