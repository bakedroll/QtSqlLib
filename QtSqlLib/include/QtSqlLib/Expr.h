#pragma once

#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/ComparisonOperator.h>

#include <QVariant>
#include <QMetaType>

#include <optional>

namespace QtSqlLib::API
{
class ISchema;
}

namespace QtSqlLib
{

class ColumnID;
class ITermElement;

class Expr
{
public:
  using OptionalIID = std::optional<std::reference_wrapper<const API::IID>>;

  Expr();
  Expr(Expr&& other) noexcept;

  Expr(const Expr& other) = delete;
  Expr& operator=(const Expr& other) = delete;

  virtual ~Expr();

  Expr& equal(const ColumnID& columnId, const QVariant& value);
  Expr& unequal(const ColumnID& columnId, const QVariant& value);
  Expr& lessEqual(const ColumnID& columnId, const QVariant& value);
  Expr& less(const ColumnID& columnId, const QVariant& value);
  Expr& greaterEqual(const ColumnID& columnId, const QVariant& value);
  Expr& greater(const ColumnID& columnId, const QVariant& value);

  Expr& isNull(const ColumnID& columnId);

  Expr& opOr(void);
  Expr& opAnd(void);

  Expr& braces(Expr& nestedExpr);

  QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId = std::nullopt) const;

private:
  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(ComparisonOperator op, const ColumnID& colIdLhs, const QVariant& value);
  Expr& addComparison(std::unique_ptr<ITermElement>&& comparison);
  Expr& addLogic(std::unique_ptr<ITermElement>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

};

}
