#include "QtSqlLib/TableConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

  TableConfigurator::TableConfigurator(Table& table)
  : m_table(table)
  , m_lastColumn(-1)
{
}

TableConfigurator::~TableConfigurator() = default;

TableConfigurator& TableConfigurator::column(unsigned int columnId, const QString& columnName, DataType type, int varcharLength)
{
  if (m_table.columns.count(columnId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Column with id %1 already exists for table.").arg(columnId).arg(m_table.name));
  }

  if (isColumnNameExisting(columnName))
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Column with name '%1' already exists for table.").arg(columnName).arg(m_table.name));
  }

  if ((type == DataType::Varchar) && (varcharLength <= 0))
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Varchar length must greater that 0 for column '%1' of table '%2'.").arg(columnName).arg(m_table.name));
  }

  Column column;
  column.name = columnName;
  column.type = type;
  column.varcharLength = varcharLength;
  column.isPrimaryKey = false;
  column.isAutoIncrement = false;
  column.isNotNull = false;

  m_table.columns[columnId] = column;
  m_lastColumn = columnId;

  return *this;
}

TableConfigurator& TableConfigurator::primaryKey()
{
  checkColumn();
  auto& col = m_table.columns.at(m_lastColumn);

  if (col.isPrimaryKey)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("primaryKey() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  col.isPrimaryKey = true;
  return *this;
}

TableConfigurator& TableConfigurator::autoIncrement()
{
  checkColumn();
  auto& col = m_table.columns.at(m_lastColumn);

  if (col.isAutoIncrement)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("autoIncrement() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  col.isAutoIncrement = true;
  return *this;
}

TableConfigurator& TableConfigurator::notNull()
{
  checkColumn();
  auto& col = m_table.columns.at(m_lastColumn);

  if (col.isNotNull)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("notNull() should only be called once for column '%1' of table '%2'.").arg(col.name).arg(m_table.name));
  }

  col.isNotNull = true;
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
  if (m_lastColumn < 0)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      "No column defined");
  }
}

}
