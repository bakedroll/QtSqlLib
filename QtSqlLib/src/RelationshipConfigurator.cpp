#include "RelationshipConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

#include <QString>

namespace QtSqlLib
{

RelationshipConfigurator::RelationshipConfigurator(API::Relationship& relationship)
  : m_relationship(relationship)
  , m_bOnDeleteCalled(false)
  , m_bOnUpdateCalled(false)
{
}

RelationshipConfigurator::~RelationshipConfigurator() = default;

API::IRelationshipConfigurator& RelationshipConfigurator::onDelete(API::ForeignKeyAction action)
{
  if (m_bOnDeleteCalled)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("onDelete() should only be called once for a relationshio."));
  }

  m_relationship.onDeleteAction = action;

  m_bOnDeleteCalled = true;
  return *this;
}

API::IRelationshipConfigurator& RelationshipConfigurator::onUpdate(API::ForeignKeyAction action)
{
  if (m_bOnUpdateCalled)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("onUpdate() should only be called once for a relationshio."));
  }

  m_relationship.onUpdateAction = action;

  m_bOnUpdateCalled = true;
  return *this;
}

}
