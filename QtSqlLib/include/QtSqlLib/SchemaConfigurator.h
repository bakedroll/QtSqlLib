#pragma once

#include <QtSqlLib/API/ISchemaConfigurator.h>

#include <QtSqlLib/API/SchemaTypes.h>

#include <map>
#include <memory>

namespace QtSqlLib::API
{
class ISchema;
}

namespace QtSqlLib
{

class SchemaConfigurator : public API::ISchemaConfigurator
{
public:
  SchemaConfigurator();
  virtual ~SchemaConfigurator();

  API::ITableConfigurator& configureTable(const API::IID& tableId, const QString& tableName) override;
  API::IRelationshipConfigurator& configureRelationship(const API::IID& relationshipId, const API::IID& tableFromId,
                                                        const API::IID& tableToId, API::RelationshipType type) override;

  std::unique_ptr<API::ISchema> getSchema() override;

private:
  std::unique_ptr<API::ISchema> m_schema;

  std::map<API::IID::Type, std::unique_ptr<API::ITableConfigurator>> m_tableConfigurators;
  std::map<API::IID::Type, std::unique_ptr<API::IRelationshipConfigurator>> m_relationshipConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
