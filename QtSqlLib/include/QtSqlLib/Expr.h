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

  Expr& equal(const ColumnID& columnId, const QVariant& value, bool noCase = false);
  Expr& unequal(const ColumnID& columnId, const QVariant& value, bool noCase = false);
  Expr& lessEqual(const ColumnID& columnId, const QVariant& value, bool noCase = false);
  Expr& less(const ColumnID& columnId, const QVariant& value, bool noCase = false);
  Expr& greaterEqual(const ColumnID& columnId, const QVariant& value, bool noCase = false);
  Expr& greater(const ColumnID& columnId, const QVariant& value, bool noCase = false);

  Expr& isNull(const ColumnID& columnId);

  Expr& opOr(void);
  Expr& opAnd(void);

  Expr& braces(Expr& nestedExpr);

  QString toQueryString(
    API::ISchema& schema,
    std::vector<QVariant>& boundValuesOut,
    const OptionalIID& defaultTableId = std::nullopt) const;

private:
  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(ComparisonOperator op, const ColumnID& colIdLhs, const QVariant& value, bool noCase);
  Expr& addComparison(std::unique_ptr<ITermElement>&& comparison);
  Expr& addLogic(std::unique_ptr<ITermElement>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

};

}
