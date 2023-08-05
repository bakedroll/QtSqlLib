#include "QtSqlLib/SanityChecker.h"

#include "QtSqlLib/DatabaseException.h"

namespace QtSqlLib
{

SanityChecker::SanityChecker(
    std::map<API::IID::Type, API::Table>& tables,
    std::map<API::IID::Type, API::Relationship>& relationships) :
  m_tablesRef(tables),
  m_relationshipsRef(relationships)
{
}

SanityChecker::~SanityChecker() = default;

void SanityChecker::throwIfTableIdNotExisting(API::IID::Type tableId) const
{
  if (m_tablesRef.count(tableId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown table id: %1.").arg(tableId));
  }
}

void SanityChecker::throwIfRelationshipIsNotExisting(API::IID::Type relationshipId) const
{
  if (m_relationshipsRef.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown relationship id: %1.").arg(relationshipId));
  }
}

void SanityChecker::throwIfColumnIdNotExisting(const API::Table& table, API::IID::Type colId) const
{
  if (table.columns.count(colId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown column id %1 in table '%2'.").arg(colId).arg(table.name));
  }
}

}
