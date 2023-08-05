#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

#include <memory>

namespace QtSqlLib::API
{

class IID;
class IIndexConfigurator;
class IRelationshipConfigurator;
class ISchema;
class ITableConfigurator;

class ISchemaConfigurator
{
public:
  ISchemaConfigurator() = default;
  virtual ~ISchemaConfigurator() = default;

  virtual ITableConfigurator& configureTable(const IID& tableId, const QString& tableName) = 0;
  virtual IRelationshipConfigurator& configureRelationship(const IID& relationshipId, const IID& tableFromId,
                                                           const IID& tableToId, API::RelationshipType type) = 0;
  virtual IIndexConfigurator& configureIndex(const IID& tableId) = 0;

  virtual std::unique_ptr<ISchema> getSchema() = 0;

};

}
