#pragma once

#include <QtSqlLib/API/ISchema.h>

namespace QtSqlLib::API
{

class IRelationshipConfigurator
{
public:
  IRelationshipConfigurator() = default;
  virtual ~IRelationshipConfigurator() = default;

  virtual IRelationshipConfigurator& onDelete(ISchema::ForeignKeyAction action) = 0;
  virtual IRelationshipConfigurator& onUpdate(ISchema::ForeignKeyAction action) = 0;

};

}
