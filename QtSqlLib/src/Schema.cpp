#include "QtSqlLib/Schema.h"

namespace QtSqlLib
{

Schema::Schema() = default;

Schema::~Schema() = default;

std::map<Schema::Id, Schema::Table>& Schema::getTables()
{
  return m_tables;
}

std::map<Schema::Id, Schema::Relationship>& Schema::getRelationships()
{
  return m_relationships;
}

}
