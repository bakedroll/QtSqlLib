#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <QVariant>
#include <QMetaType>

#include <optional>

namespace QtSqlLib
{

class Expr
{
public:
  class ColumnId
  {
  public:
    ColumnId();
    ColumnId(const API::ISchema::TableColumnId& tableColumnId);
    ColumnId(API::ISchema::Id columnId);
    ColumnId(const QString& tableAlias, const API::ISchema::TableColumnId& tableColumnId);
    virtual ~ColumnId();

    const API::ISchema::TableColumnId& get() const;
    bool isTableIdValid() const;
    QString getTableAlias() const;

  private:
    ColumnId(const QString& tableAlias, const API::ISchema::TableColumnId& tableColumnId, bool bIsTableIdValid);

    QString m_tableAlias;
    API::ISchema::TableColumnId m_tableColumnId;
    bool m_bIsTableIdValid;

  };

  Expr();
  Expr(Expr&& other) noexcept;

  Expr(const Expr& other) = delete;
  Expr& operator=(const Expr& other) = delete;

  virtual ~Expr();

  Expr& equal(const ColumnId& columnId, const QVariant& value);
  Expr& equal(const ColumnId& colIdLhs, const ColumnId& colIdRhs);

  Expr& unequal(const ColumnId& columnId, const QVariant& value);
  Expr& unequal(const ColumnId& colIdLhs, const ColumnId& colIdRhs);

  Expr& lessEqual(const ColumnId& columnId, const QVariant& value);
  Expr& lessEqual(const ColumnId& colIdLhs, const ColumnId& colIdRhs);

  Expr& less(const ColumnId& columnId, const QVariant& value);
  Expr& less(const ColumnId& colIdLhs, const ColumnId& colIdRhs);

  Expr& greaterEqual(const ColumnId& columnId, const QVariant& value);
  Expr& greaterEqual(const ColumnId& colIdLhs, const ColumnId& colIdRhs);

  Expr& greater(const ColumnId& columnId, const QVariant& value);
  Expr& greater(const ColumnId& colIdLhs, const ColumnId& colIdRhs);

  Expr& isNull(const ColumnId& columnId);

  Expr& or();
  Expr& and();

  Expr& braces(Expr& nestedExpr);

  QString toQString(API::ISchema& schema, const std::optional<API::ISchema::Id>& defaultTableId = std::nullopt) const;

private:
  enum class ComparisonOperator
  {
    Equal,
    Unequal,
    LessEqual,
    Less,
    GreaterEqual,
    Greater,
    IsNull
  };

  enum class OperandType
  {
    ColumnId,
    Value
  };

  enum class LogicalOperator
  {
    And,
    Or
  };

  struct Operand
  {
    OperandType type;
    QVariant value;
  };

  class ITermElement
  {
  public:
    ITermElement() = default;
    virtual ~ITermElement() = default;
    virtual QString toQString(API::ISchema& schema, const std::optional<API::ISchema::Id>& defaultTableId = std::nullopt) const = 0;

  };

  class Comparison : public ITermElement
  {
  public:
    Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs);
    ~Comparison() override;

    QString toQString(API::ISchema& schema, const std::optional<API::ISchema::Id>& defaultTableId) const override;

  private:
    ComparisonOperator m_operator;
    Operand m_lhs;
    Operand m_rhs;

  };

  class NestedExpression : public ITermElement
  {
  public:
    explicit NestedExpression(Expr& expr);
    ~NestedExpression() override;

    QString toQString(API::ISchema& schema, const std::optional<API::ISchema::Id>& defaultTableId) const override;

  private:
    std::unique_ptr<Expr> m_nestedExpr;

  };

  class Logic : public ITermElement
  {
  public:
    explicit Logic(LogicalOperator op);
    ~Logic() override;

    QString toQString(API::ISchema& schema, const std::optional<API::ISchema::Id>& defaultTableId) const override;

  private:
    LogicalOperator m_operator;

  };

  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(ComparisonOperator op, const ColumnId& colIdLhs, const ColumnId& colIdRhs);
  Expr& addComparison(ComparisonOperator op, const ColumnId& colIdLhs, const QVariant& value);

  Expr& addComparison(std::unique_ptr<Comparison>&& comparison);

  Expr& addLogic(std::unique_ptr<Logic>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

};

}

Q_DECLARE_METATYPE(QtSqlLib::Expr::ColumnId);
