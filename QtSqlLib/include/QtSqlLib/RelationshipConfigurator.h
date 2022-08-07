#pragma once

#include <QtSqlLib/API/ISchema.h>

namespace QtSqlLib
{

class RelationshipConfigurator
{
public:
  RelationshipConfigurator(API::ISchema::Relationship& relationship);
  virtual ~RelationshipConfigurator();

  RelationshipConfigurator& onDelete(API::ISchema::ForeignKeyAction action);
  RelationshipConfigurator& onUpdate(API::ISchema::ForeignKeyAction action);

private:
  API::ISchema::Relationship& m_relationship;

  bool m_bOnDeleteCalled;
  bool m_bOnUpdateCalled;

};

}