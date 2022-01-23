#pragma once

#include <QtSqlLib/Schema.h>

namespace QtSqlLib
{

class RelationshipConfigurator
{
public:
  RelationshipConfigurator(Schema::Relationship& relationship);
  virtual ~RelationshipConfigurator();

  RelationshipConfigurator& onDelete(Schema::ForeignKeyAction action);
  RelationshipConfigurator& onUpdate(Schema::ForeignKeyAction action);

private:
  Schema::Relationship& m_relationship;

  bool m_bOnDeleteCalled;
  bool m_bOnUpdateCalled;

};

}