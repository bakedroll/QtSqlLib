#pragma once

#include <QtSqlLib/API/ISchemaConfigurator.h>

#include <QtSqlLib/API/SchemaTypes.h>

#include <memory>
#include <vector>

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
  ~SchemaConfigurator() override;

  API::ITableConfigurator& configureTable(const API::IID& tableId, const QString& tableName) override;
  API::IRelationshipConfigurator& configureRelationship(const API::IID& relationshipId, const API::IID& tableFromId,
                                                        const API::IID& tableToId, API::RelationshipType type) override;
  API::IIndexConfigurator& configureIndex(const API::IID& tableId) override;

  std::unique_ptr<API::ISchema> getSchema() override;

private:
  std::unique_ptr<API::ISchema> m_schema;

  std::vector<std::unique_ptr<API::ITableConfigurator>> m_tableConfigurators;
  std::vector<std::unique_ptr<API::IRelationshipConfigurator>> m_relationshipConfigurators;
  std::vector<std::unique_ptr<API::IIndexConfigurator>> m_indexConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
