#pragma once

#include <QtSqlLib/RelationshipConfigurator.h>
#include <QtSqlLib/Schema.h>
#include <QtSqlLib/TableConfigurator.h>

#include <QString>

#include <map>
#include <memory>

namespace QtSqlLib
{

class SchemaConfigurator
{
public:
  SchemaConfigurator(Schema& schema);
  virtual ~SchemaConfigurator();

  TableConfigurator& configureTable(Schema::Id tableId, const QString& tableName);
  RelationshipConfigurator& configureRelationship(Schema::Id relationshipId, Schema::Id tableFromId,
                                                  Schema::Id tableToId, Schema::RelationshipType type);

private:
  Schema& m_schema;

  std::map<Schema::Id, std::unique_ptr<TableConfigurator>> m_tableConfigurators;
  std::map<Schema::Id, std::unique_ptr<RelationshipConfigurator>> m_relationshipConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
