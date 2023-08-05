#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

namespace QtSqlLib::API
{

class IRelationshipConfigurator
{
public:
  IRelationshipConfigurator() = default;
  virtual ~IRelationshipConfigurator() = default;

  virtual IRelationshipConfigurator& onDelete(API::ForeignKeyAction action) = 0;
  virtual IRelationshipConfigurator& onUpdate(API::ForeignKeyAction action) = 0;
  virtual IRelationshipConfigurator& enableForeignKeyIndexing() = 0;

};

}
