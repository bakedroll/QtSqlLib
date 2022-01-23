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

  if (tableName.isEmpty())
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Table name with id %1 must not be empty.").arg(tableId));
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

RelationshipConfigurator& SchemaConfigurator::configureRelationship(unsigned int relationshipId, unsigned int tableFromId,
                                                                    unsigned int tableToId, RelationshipConfigurator::RelationshipType type)
{
  if (m_schema.relationships.count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Relationship with id %1 already exists.").arg(relationshipId));
  }

  const RelationshipConfigurator::Relationship relationship{ tableFromId, tableToId, type };

  m_schema.relationships[relationshipId] = relationship;
  m_relationshipConfigurators[relationshipId] = std::make_unique<RelationshipConfigurator>(m_schema.relationships.at(relationshipId));

  return *m_relationshipConfigurators.at(relationshipId);
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
