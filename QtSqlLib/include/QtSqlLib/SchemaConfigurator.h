#pragma once

#include <QtSqlLib/API/ISchemaConfigurator.h>

#include <map>
#include <memory>

namespace QtSqlLib
{

class SchemaConfigurator : public API::ISchemaConfigurator
{
public:
  SchemaConfigurator(API::ISchema& schema);
  virtual ~SchemaConfigurator();

  API::ITableConfigurator& configureTable(API::ISchema::Id tableId, const QString& tableName) override;
  API::IRelationshipConfigurator& configureRelationship(API::ISchema::Id relationshipId, API::ISchema::Id tableFromId,
                                                        API::ISchema::Id tableToId, API::ISchema::RelationshipType type) override;

private:
  API::ISchema& m_schema;

  std::map<API::ISchema::Id, std::unique_ptr<API::ITableConfigurator>> m_tableConfigurators;
  std::map<API::ISchema::Id, std::unique_ptr<API::IRelationshipConfigurator>> m_relationshipConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
