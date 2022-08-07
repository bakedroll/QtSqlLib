#pragma once

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

  virtual ITableConfigurator& configureTable(ISchema::Id tableId, const QString& tableName) = 0;
  virtual IRelationshipConfigurator& configureRelationship(ISchema::Id relationshipId, API::ISchema::Id tableFromId,
                                                           ISchema::Id tableToId, ISchema::RelationshipType type) = 0;

  virtual std::unique_ptr<ISchema> getSchema() = 0;

};

}
