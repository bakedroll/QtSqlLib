#pragma once

#include <QtSqlLib/RelationshipConfigurator.h>
#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/TableConfigurator.h>

#include <map>
#include <memory>

namespace QtSqlLib
{

class SchemaConfigurator
{
public:
  SchemaConfigurator(API::ISchema& schema);
  virtual ~SchemaConfigurator();

  TableConfigurator& configureTable(API::ISchema::Id tableId, const QString& tableName);
  RelationshipConfigurator& configureRelationship(API::ISchema::Id relationshipId, API::ISchema::Id tableFromId,
                                                  API::ISchema::Id tableToId, API::ISchema::RelationshipType type);

private:
  API::ISchema& m_schema;

  std::map<API::ISchema::Id, std::unique_ptr<TableConfigurator>> m_tableConfigurators;
  std::map<API::ISchema::Id, std::unique_ptr<RelationshipConfigurator>> m_relationshipConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
