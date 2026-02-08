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

class Comparison;
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

  bool isEmpty() const;

  template <typename TLeft, typename TRight>
  Expr& equal(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Equal, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& unequal(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Unequal, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& lessEqual(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::LessEqual, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& less(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Less, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& greaterEqual(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::GreaterEqual, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& greater(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Greater, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& opIs(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Is, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& opNot(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Not, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& opLike(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::Like, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  template <typename TLeft, typename TRight>
  Expr& opIn(TLeft&& lhs, TRight&& rhs)
  {
    addComparison(EComparisonOperator::In, makeVariant(std::forward<TLeft>(lhs)), makeVariant(std::forward<TRight>(rhs)));
    return *this;
  }

  Expr& noCase();
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
    LogicalOperatorOrCollate
  };

  Expr& addComparison(EComparisonOperator op, const QVariant& lhs, const QVariant& rhs);
  Expr& addLogic(std::unique_ptr<ITermElement>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;
  Comparison* m_lastComparison;

  QVariant makeVariant(ColumnHelper::ColumnData&& data);
  QVariant makeVariant(ColumnHelper::ColumnAlias&& alias);
  QVariant makeVariant(QVariant&& value);

  QVariant makeVariant(const ColumnHelper::ColumnData& data);
  QVariant makeVariant(const ColumnHelper::ColumnAlias& alias);
  QVariant makeVariant(const QVariant& value);

  template<typename T, typename = std::enable_if_t<std::is_enum_v<T> || std::is_fundamental_v<T>>>
  QVariant makeVariant(const T& value)
  {
    return QVariant::fromValue<ColumnHelper::ColumnData>(ColumnHelper::ColumnData(value));
  }

};

}
