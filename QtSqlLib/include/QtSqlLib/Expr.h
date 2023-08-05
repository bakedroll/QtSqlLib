#pragma once

#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ID.h>

#include <QVariant>
#include <QMetaType>

#include <optional>

#define EQUAL(A, B) equal(QtSqlLib::ID(A), QVariant(B))
#define EQUAL_COL(A, B) equal(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::Expr::ColumnId(QtSqlLib::ID(B))))

#define UNEQUAL(A, B) unequal(QtSqlLib::ID(A), QVariant(B))
#define UNEQUAL_COL(A, B) unequal(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::Expr::ColumnId(QtSqlLib::ID(B))))

#define LESSEQUAL(A, B) lessEqual(QtSqlLib::ID(A), QVariant(B))
#define LESSEQUAL_COL(A, B) lessEqual(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::Expr::ColumnId(QtSqlLib::ID(B))))

#define LESS(A, B) less(QtSqlLib::ID(A), QVariant(B))
#define LESS_COL(A, B) less(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::Expr::ColumnId(QtSqlLib::ID(B))))

#define GREATEREQUAL(A, B) greaterEqual(QtSqlLib::ID(A), QVariant(B))
#define GREATEREQUAL_COL(A, B) greaterEqual(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::Expr::ColumnId(QtSqlLib::ID(B))))

#define GREATER(A, B) greater(QtSqlLib::ID(A), QVariant(B))
#define GREATER_COL(A, B) greater(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::Expr::ColumnId(QtSqlLib::ID(B))))

#define ISNULL(A, B) greater(QtSqlLib::ID(A))

#define OR or()
#define AND and()

#define _(A) braces(QtSqlLib::Expr().A)

namespace QtSqlLib
{

class Expr
{
public:
  using OptionalIID = std::optional<std::reference_wrapper<const API::IID>>;

  class ColumnId
  {
  public:
    ColumnId();
    ColumnId(const API::TableColumnId& tableColumnId);
    ColumnId(const API::IID& columnId);
    ColumnId(const QString& tableAlias, const API::TableColumnId& tableColumnId);
    virtual ~ColumnId();

    const API::TableColumnId& get() const;
    bool isTableIdValid() const;
    QString getTableAlias() const;

  private:
    ColumnId(const QString& tableAlias, const API::TableColumnId& tableColumnId, bool bIsTableIdValid);

    QString m_tableAlias;
    API::TableColumnId m_tableColumnId;
    bool m_bIsTableIdValid;

  };

  Expr();
  Expr(Expr&& other) noexcept;

  Expr(const Expr& other) = delete;
  Expr& operator=(const Expr& other) = delete;

  virtual ~Expr();

  Expr& equal(const ColumnId& columnId, const QVariant& value);
  Expr& unequal(const ColumnId& columnId, const QVariant& value);
  Expr& lessEqual(const ColumnId& columnId, const QVariant& value);
  Expr& less(const ColumnId& columnId, const QVariant& value);
  Expr& greaterEqual(const ColumnId& columnId, const QVariant& value);
  Expr& greater(const ColumnId& columnId, const QVariant& value);

  Expr& isNull(const ColumnId& columnId);

  Expr& or();
  Expr& and();

  Expr& braces(Expr& nestedExpr);

  QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId = std::nullopt) const;

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
    virtual QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId = std::nullopt) const = 0;

  };

  class Comparison : public ITermElement
  {
  public:
    Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs);
    ~Comparison() override;

    QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId) const override;

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

    QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId) const override;

  private:
    std::unique_ptr<Expr> m_nestedExpr;

  };

  class Logic : public ITermElement
  {
  public:
    explicit Logic(LogicalOperator op);
    ~Logic() override;

    QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId) const override;

  private:
    LogicalOperator m_operator;

  };

  enum class NextTermExpectation
  {
    ComparisonOrNestedExpr,
    LogicalOperator
  };

  Expr& addComparison(ComparisonOperator op, const ColumnId& colIdLhs, const QVariant& value);
  Expr& addComparison(std::unique_ptr<Comparison>&& comparison);

  Expr& addLogic(std::unique_ptr<Logic>&& logic);

  NextTermExpectation m_nextExpectation;
  std::vector<std::unique_ptr<ITermElement>> m_termElements;

};

}

Q_DECLARE_METATYPE(QtSqlLib::Expr::ColumnId);
