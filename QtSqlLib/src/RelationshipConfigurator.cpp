#include "QtSqlLib/RelationshipConfigurator.h"

#include "QtSqlLib/DatabaseException.h"

#include <QString>

namespace QtSqlLib
{

RelationshipConfigurator::RelationshipConfigurator(Schema::Relationship& relationship)
  : m_relationship(relationship)
  , m_bOnDeleteCalled(false)
  , m_bOnUpdateCalled(false)
{
}

RelationshipConfigurator::~RelationshipConfigurator() = default;

RelationshipConfigurator& RelationshipConfigurator::onDelete(Schema::ForeignKeyAction action)
{
  if (m_bOnDeleteCalled)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("onDelete() should only be called once for a relationshio."));
  }

  m_relationship.onDeleteAction = action;

  m_bOnDeleteCalled = true;
  return *this;
}

RelationshipConfigurator& RelationshipConfigurator::onUpdate(Schema::ForeignKeyAction action)
{
  if (m_bOnUpdateCalled)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("onUpdate() should only be called once for a relationshio."));
  }

  m_relationship.onUpdateAction = action;

  m_bOnUpdateCalled = true;
  return *this;
}
}
