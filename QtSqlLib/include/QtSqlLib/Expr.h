#pragma once

#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/EComparisonOperator.h>

#include <QVariant>
#include <QMetaType>

#include <optional>

namespace QtSqlLib::API
{
class IQueryIdentifiers;
class ISchema;
}

namespace QtSqlLib
{

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

  template <typename TLeft, typename TRight>
  Expr& equal(TLeft&& lhs, TRight&& rhs, bool noCase = false)
  {
    addComparison(EComparisonOperator::Equal, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)), noCase);
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& unequal(TLeft&& lhs, TRight&& rhs, bool noCase = false)
  {
    addComparison(EComparisonOperator::Unequal, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)), noCase);
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& lessEqual(TLeft&& lhs, TRight&& rhs, bool noCase = false)
  {
    addComparison(EComparisonOperator::LessEqual, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)), noCase);
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& less(TLeft&& lhs, TRight&& rhs, bool noCase = false)
  {
    addComparison(EComparisonOperator::Less, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)), noCase);
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& greaterEqual(TLeft&& lhs, TRight&& rhs, bool noCase = false)
  {
    addComparison(EComparisonOperator::GreaterEqual, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)), noCase);
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& greater(TLeft&& lhs, TRight&& rhs, bool noCase = false)
  {
    addComparison(EComparisonOperator::Greater, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)), noCase);
    return *this;
  }

  template <typename T>
  Expr& isNull(T&& value)
  {
    addComparison(EComparisonOperator::IsNull, makeVariant(std::forward<T>(value)), QVariant(), false);
    return *this;
  }

  Expr& opOr();
  Expr& opAnd();

  Expr& braces(Expr& nestedExpr);

  QString toQueryString(
    API::ISchema& schema,
    const API::IQueryIdentifiers& queryIdentifiers,
    std::vector<QVariant>& boundValuesOut) const;

private:
  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(EComparisonOperator op, const QVariant& lhs, const QVariant& rhs, bool noCase);
  Expr& addLogic(std::unique_ptr<ITermElement>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

  QVariant makeVariant(ColumnHelper::ColumnData&& data);
  QVariant makeVariant(QVariant&& value);

  QVariant makeVariant(const ColumnHelper::ColumnData& data);
  QVariant makeVariant(const QVariant& value);

  template<typename T, typename = std::enable_if_t<std::is_enum_v<T> || std::is_fundamental_v<T>>>
  QVariant makeVariant(const T& value)
  {
    return QVariant::fromValue<ColumnHelper::ColumnData>(ColumnHelper::ColumnData(value));
  }

};

}
