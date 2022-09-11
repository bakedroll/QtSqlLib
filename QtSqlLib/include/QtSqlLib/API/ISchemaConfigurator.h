#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/IRelationshipConfigurator.h>
#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/API/ITableConfigurator.h>

#include <memory>

#define CONFIGURE_TABLE(X, Y) configureTable(QtSqlLib::ID(X), Y)
#define CONFIGURE_RELATIONSHIP(X, Y, Z, T) configureRelationship((QtSqlLib::ID(X), (QtSqlLib::ID(Y), (QtSqlLib::ID(Z), T)

namespace QtSqlLib::API
{

class ISchemaConfigurator
{
public:
  ISchemaConfigurator() = default;
  virtual ~ISchemaConfigurator() = default;

  virtual ITableConfigurator& configureTable(const IID& tableId, const QString& tableName) = 0;
  virtual IRelationshipConfigurator& configureRelationship(const IID& relationshipId, const IID& tableFromId,
                                                           const IID& tableToId, ISchema::RelationshipType type) = 0;

  virtual std::unique_ptr<ISchema> getSchema() = 0;

};

}
