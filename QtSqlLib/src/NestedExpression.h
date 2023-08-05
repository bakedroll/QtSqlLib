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

  QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId) const override;

private:
  std::unique_ptr<Expr> m_nestedExpr;

};

}
