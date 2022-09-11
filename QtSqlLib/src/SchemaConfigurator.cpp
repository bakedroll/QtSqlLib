#include "QtSqlLib/SchemaConfigurator.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/RelationshipConfigurator.h"
#include "QtSqlLib/Schema.h"
#include "QtSqlLib/TableConfigurator.h"

namespace QtSqlLib
{

SchemaConfigurator::SchemaConfigurator() :
  ISchemaConfigurator(),
  m_schema(std::make_unique<Schema>())
{
}

SchemaConfigurator::~SchemaConfigurator() = default;

API::ITableConfigurator& SchemaConfigurator::configureTable(const API::IID& tableId, const QString& tableName)
{
  const auto tid = tableId.get();
  if (m_schema->getTables().count(tid) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Table with id %1 already exists.").arg(tid));
  }

  if (tableName.isEmpty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Table name with id %1 must not be empty.").arg(tid));
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

  API::ISchema::Table table;
  table.name = tableName;

  m_schema->getTables()[tid] = table;
  m_tableConfigurators[tid] = std::make_unique<TableConfigurator>(m_schema->getTables().at(tid));

  return *m_tableConfigurators.at(tid);
}

API::IRelationshipConfigurator& SchemaConfigurator::configureRelationship(const API::IID& relationshipId, const API::IID& tableFromId,
                                                                          const API::IID& tableToId, API::ISchema::RelationshipType type)
{
  const auto relId = relationshipId.get();
  if (m_schema->getRelationships().count(relId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("Relationship with id %1 already exists.").arg(relId));
  }

  if ((tableFromId.get() == tableToId.get()) && type == API::ISchema::RelationshipType::ManyToOne)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "The relationship type ManyToOne is not allowed for a relationship of a table that references itself due to possible ambiguity. " \
      "Please use OneToMany instead.");
  }

  const API::ISchema::Relationship relationship{ tableFromId.get(), tableToId.get(), type };

  m_schema->getRelationships()[relId] = relationship;
  m_relationshipConfigurators[relId] =
    std::make_unique<RelationshipConfigurator>(m_schema->getRelationships().at(relId));

  return *m_relationshipConfigurators.at(relId);
}

std::unique_ptr<API::ISchema> SchemaConfigurator::getSchema()
{
  return std::move(m_schema);
}

bool SchemaConfigurator::isTableNameExisting(const QString& name) const
{
  for (const auto& table : m_schema->getTables())
  {
    if (table.second.name.toLower() == name.toLower())
    {
      return true;
    }
  }

  return false;
}

}
