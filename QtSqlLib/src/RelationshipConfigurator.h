#pragma once

#include <QtSqlLib/API/IRelationshipConfigurator.h>

namespace QtSqlLib
{

class RelationshipConfigurator : public API::IRelationshipConfigurator
{
public:
  RelationshipConfigurator(API::Relationship& relationship);
  ~RelationshipConfigurator() override;

  IRelationshipConfigurator& onDelete(API::ForeignKeyAction action) override;
  IRelationshipConfigurator& onUpdate(API::ForeignKeyAction action) override;

private:
  API::Relationship& m_relationship;

  bool m_bOnDeleteCalled;
  bool m_bOnUpdateCalled;

};

}