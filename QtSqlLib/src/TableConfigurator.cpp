#include "QtSqlLib/TableConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

TableConfigurator::TableConfigurator(API::ISchema::Table& table)
  : m_table(table)
  , m_bIsPrimaryKeysConfigured(false)
{
}

TableConfigurator::~TableConfigurator() = default;

API::ITableConfigurator& TableConfigurator::column(API::ISchema::Id columnId, const QString& columnName,
                                                   API::ISchema::DataType type, int varcharLength)
{
  if (m_table.columns.count(columnId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Column with id %1 already exists for table '%2'.").arg(columnId).arg(m_table.name));
  }

  if (columnName.isEmpty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Column name must not be empty for table '%1'.").arg(m_table.name));
  }

  if (isColumnNameExisting(columnName))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Column with name '%1' already exists for table '%2'.").arg(columnName).arg(m_table.name));
  }

  if ((type == API::ISchema::DataType::Varchar) && (varcharLength <= 0))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Varchar length must greater that 0 for column '%1' of table '%2'.").arg(columnName).arg(m_table.name));
  }

  API::ISchema::Column column;
  column.name = columnName;
  column.type = type;
  column.varcharLength = varcharLength;
  column.bIsAutoIncrement = false;
  column.bIsNotNull = false;

  m_table.columns[columnId] = column;
  m_lastColumnId = columnId;

  return *this;
}

API::ITableConfigurator& TableConfigurator::primaryKey()
{
  checkColumn();
  const auto colId = m_lastColumnId.value();
  auto& col = m_table.columns.at(colId);

  if (m_table.primaryKeys.count(colId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("primaryKey() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  if (!m_table.primaryKeys.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Only one column of a table can be the primary, see table '%1'.").arg(m_table.name));
  }

  m_table.primaryKeys.insert(colId);

  return *this;
}

API::ITableConfigurator& TableConfigurator::autoIncrement()
{
  checkColumn();
  auto& col = m_table.columns.at(m_lastColumnId.value());

  if (col.bIsAutoIncrement)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("autoIncrement() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  col.bIsAutoIncrement = true;
  return *this;
}

API::ITableConfigurator& TableConfigurator::notNull()
{
  checkColumn();
  auto& col = m_table.columns.at(m_lastColumnId.value());

  if (col.bIsNotNull)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("notNull() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  col.bIsNotNull = true;
  return *this;
}

API::ITableConfigurator& TableConfigurator::primaryKeys(const std::set<API::ISchema::Id>& columnIds)
{
  if (m_bIsPrimaryKeysConfigured)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Primary key configuration can only be called once for table '%1'").arg(m_table.name));
  }

  if (columnIds.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("The primary key column set must not be empty for table table '%1'").arg(m_table.name));
  }

  m_table.primaryKeys = columnIds;
  m_bIsPrimaryKeysConfigured = true;
  return *this;
}

bool TableConfigurator::isColumnNameExisting(const QString& name) const
{
  for (const auto& column : m_table.columns)
  {
    if (column.second.name.toLower() == name.toLower())
    {
      return true;
    }
  }

  return false;
}

void TableConfigurator::checkColumn() const
{
  if (!m_lastColumnId.has_value())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "No column defined");
  }
}

}
