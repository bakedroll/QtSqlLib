#include "QtSqlLib/SchemaConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

SchemaConfigurator::SchemaConfigurator(Schema& schema)
  : m_schema(schema)
{
}

SchemaConfigurator::~SchemaConfigurator() = default;

TableConfigurator& SchemaConfigurator::configureTable(Schema::Id tableId, const QString& tableName)
{
  if (m_schema.getTables().count(tableId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Table with id %1 already exists.").arg(tableId));
  }

  if (tableName.isEmpty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Table name with id %1 must not be empty.").arg(tableId));
  }

  if (isTableNameExisting(tableName))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Table with name '%1' already exists.").arg(tableName));
  }

  if (tableName.startsWith("sqlite_"))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Table name must not start with 'sqlite_'.");
  }

  Schema::Table table;
  table.name = tableName;

  m_schema.getTables()[tableId] = table;
  m_tableConfigurators[tableId] = std::make_unique<TableConfigurator>(m_schema.getTables().at(tableId));

  return *m_tableConfigurators.at(tableId);
}

RelationshipConfigurator& SchemaConfigurator::configureRelationship(Schema::Id relationshipId, Schema::Id tableFromId,
                                                                    Schema::Id tableToId, Schema::RelationshipType type)
{
  if (m_schema.getRelationships().count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Relationship with id %1 already exists.").arg(relationshipId));
  }

  const Schema::Relationship relationship{ tableFromId, tableToId, type };

  m_schema.getRelationships()[relationshipId] = relationship;
  m_relationshipConfigurators[relationshipId] = std::make_unique<RelationshipConfigurator>(m_schema.getRelationships().at(relationshipId));

  return *m_relationshipConfigurators.at(relationshipId);
}

bool SchemaConfigurator::isTableNameExisting(const QString& name) const
{
  for (const auto& table : m_schema.getTables())
  {
    if (table.second.name.toLower() == name.toLower())
    {
      return true;
    }
  }

  return false;
}

}
