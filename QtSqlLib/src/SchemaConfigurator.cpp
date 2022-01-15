#include "QtSqlLib/SchemaConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

SchemaConfigurator::SchemaConfigurator(Schema& schema)
  : m_schema(schema)
{
}

SchemaConfigurator::~SchemaConfigurator() = default;

TableConfigurator& SchemaConfigurator::configureTable(unsigned int tableId, const QString& tableName)
{
  if (m_schema.tables.count(tableId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Table with id %1 already exists.").arg(tableId));
  }

  if (isTableNameExisting(tableName))
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Table with name '%1' already exists.").arg(tableName));
  }

  if (tableName.startsWith("sqlite_"))
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      "Table name must not start with 'sqlite_'.");
  }

  TableConfigurator::Table table;
  table.name = tableName;

  m_schema.tables[tableId] = table;
  m_tableConfigurators[tableId] = std::make_unique<TableConfigurator>(m_schema.tables.at(tableId));

  return *m_tableConfigurators.at(tableId);
}

bool SchemaConfigurator::isTableNameExisting(const QString& name) const
{
  for (const auto& table : m_schema.tables)
  {
    if (table.second.name.toLower() == name.toLower())
    {
      return true;
    }
  }

  return false;
}

}
