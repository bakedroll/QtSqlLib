#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/IRelationshipConfigurator.h>
#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/API/ITableConfigurator.h>

#include <memory>

namespace QtSqlLib::API
{

class ISchemaConfigurator
{
public:
  ISchemaConfigurator() = default;
  virtual ~ISchemaConfigurator() = default;

  virtual ITableConfigurator& configureTable(const IID& tableId, const QString& tableName) = 0;
  virtual IRelationshipConfigurator& configureRelationship(const IID& relationshipId, const IID& tableFromId,
                                                           const IID& tableToId, API::RelationshipType type) = 0;

  virtual std::unique_ptr<ISchema> getSchema() = 0;

};

}
