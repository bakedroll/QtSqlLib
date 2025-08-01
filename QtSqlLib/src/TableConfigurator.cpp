#include "TableConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

static bool contains(const ColumnHelper::SelectColumnList& columns, API::IID::Type value)
{
  return std::find_if(columns.cbegin(), columns.cend(),
    [&value](const ColumnHelper::SelectColumn& col) { return col.columnId == value; }) != columns.cend();
}

TableConfigurator::TableConfigurator(API::Table& table)
  : m_table(table)
  , m_bIsPrimaryKeysConfigured(false)
{
}

TableConfigurator::~TableConfigurator() = default;

API::ITableConfigurator& TableConfigurator::column(
  const API::IID& columnId, const QString& columnName,
  API::DataType type, int varcharLength)
{
  const auto cid = columnId.get();
  if ((cid & API::IID::sc_columnIdReservedBits) != 0x0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Column with id %1 for table '%2' exceeds the maximum allowed range of 27 bits.").arg(cid).arg(m_table.name));
  }

  if (m_table.columns.count(cid) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Column with id %1 already exists for table '%2'.").arg(cid).arg(m_table.name));
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

  if ((type == API::DataType::Varchar) && (varcharLength <= 0))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Varchar length must greater that 0 for column '%1' of table '%2'.").arg(columnName).arg(m_table.name));
  }

  API::Column column;
  column.name = columnName;
  column.type = type;
  column.varcharLength = varcharLength;
  column.bIsAutoIncrement = false;
  column.bIsNotNull = false;

  m_table.columns[cid] = column;
  m_lastColumnId = cid;

  return *this;
}

API::ITableConfigurator& TableConfigurator::primaryKey()
{
  checkColumn();
  const auto colId = m_lastColumnId.value();
  auto& col = m_table.columns.at(colId);

  if (contains(m_table.primaryKeys, colId))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("primaryKey() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  if (!m_table.primaryKeys.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Only one column of a table can be the primary, see table '%1'.").arg(m_table.name));
  }

  m_table.primaryKeys.emplace_back(colId);

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

API::ITableConfigurator& TableConfigurator::unique()
{
  checkColumn();
  auto& col = m_table.columns.at(m_lastColumnId.value());

  if (col.bIsUnique)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("unique() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  col.bIsUnique = true;
  return *this;
}

API::ITableConfigurator& TableConfigurator::primaryKeys(const ColumnHelper::SelectColumnList& columns)
{
  if (m_bIsPrimaryKeysConfigured)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Primary key configuration can only be called once for table '%1'").arg(m_table.name));
  }

  if (columns.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("The primary key column set must not be empty for table table '%1'").arg(m_table.name));
  }

  m_table.primaryKeys = columns;
  m_bIsPrimaryKeysConfigured = true;
  return *this;
}

API::ITableConfigurator& TableConfigurator::uniqueCols(const ColumnHelper::SelectColumnList& columns)
{
  if (columns.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("The unique column set must not be empty for table table '%1'").arg(m_table.name));
  }

  m_table.uniqueColIds = columns;
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
