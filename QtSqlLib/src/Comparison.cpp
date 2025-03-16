#include "Comparison.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/API/ISchema.h"
#include "QtSqlLib/ColumnID.h"

namespace QtSqlLib
{

Comparison::Comparison(ComparisonOperator op, const Operand& lhs, const Operand& rhs)
  : ITermElement()
  , m_operator(op)
  , m_lhs(lhs)
  , m_rhs(rhs)
{
}

Comparison::~Comparison() = default;

QString Comparison::toQString(API::ISchema& schema, const OptionalIID& defaultTableId = std::nullopt) const
{
  const auto getOperandString = [&schema, &defaultTableId](const Operand& operand) -> QString
  {
    switch (operand.type)
    {
    case OperandType::ColumnId:
    {
      const auto columnId = operand.value.value<ColumnID>();
      if (!defaultTableId.has_value() && !columnId.isTableIdValid())
      {
        return "NULL";
      }
      else if (defaultTableId && !columnId.isTableIdValid())
      {
        schema.getSanityChecker().throwIfTableIdNotExisting(defaultTableId->get().get());
      }

      const auto colId = columnId.columnId();
      const auto& table = schema.getTables().at(columnId.isTableIdValid()
        ? columnId.tableId()
        : defaultTableId->get().get());

      schema.getSanityChecker().throwIfColumnIdNotExisting(table, colId);

      const auto tableAlias = columnId.tableAlias();

      return QString("'%1'.'%2'").arg(tableAlias.isEmpty() ? table.name : tableAlias).arg(table.columns.at(colId).name);
    }
    case OperandType::Value:
    {
      if (operand.value.userType() == QMetaType::QString)
      {
        return QString("'%1'").arg(operand.value.toString());
      }

      return operand.value.toString();
    }
    default:
      assert(false);
      break;
    }

    return "";
  };

  QString operatorStr;
  switch (m_operator)
  {
  case ComparisonOperator::Equal:
    operatorStr = "==";
    break;
  case ComparisonOperator::Unequal:
    operatorStr = "!=";
    break;
  case ComparisonOperator::LessEqual:
    operatorStr = "<=";
    break;
  case ComparisonOperator::Less:
    operatorStr = "<";
    break;
  case ComparisonOperator::GreaterEqual:
    operatorStr = ">=";
    break;
  case ComparisonOperator::Greater:
    operatorStr = ">";
    break;
  case ComparisonOperator::IsNull:
    operatorStr = "is";
    break;
  default:
    assert(false);
    break;
  }

  return QString("%1 %2 %3").arg(getOperandString(m_lhs)).arg(operatorStr).arg(getOperandString(m_rhs));
}

}