#pragma once

#include <QtSqlLib/API/IRelationshipConfigurator.h>

namespace QtSqlLib
{

class RelationshipConfigurator : public API::IRelationshipConfigurator
{
public:
  RelationshipConfigurator(API::ISchema::Relationship& relationship);
  ~RelationshipConfigurator() override;

  IRelationshipConfigurator& onDelete(API::ISchema::ForeignKeyAction action) override;
  IRelationshipConfigurator& onUpdate(API::ISchema::ForeignKeyAction action) override;

private:
  API::ISchema::Relationship& m_relationship;

  bool m_bOnDeleteCalled;
  bool m_bOnUpdateCalled;

};

}