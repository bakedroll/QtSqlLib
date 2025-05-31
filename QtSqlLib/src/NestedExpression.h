#pragma once

#include "ITermElement.h"

#include <memory>

#include <QString>

namespace QtSqlLib
{

class Expr;

class NestedExpression : public ITermElement
{
public:
  explicit NestedExpression(Expr& expr);
  ~NestedExpression() override;

  QString toQueryString(
    API::ISchema& schema,
    std::vector<QVariant>& boundValuesOut,
    const OptionalIID& defaultTableId) const override;

private:
  std::unique_ptr<Expr> m_nestedExpr;

};

}
