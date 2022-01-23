#include "QtSqlLib/Schema.h"

#include "QtSqlLib/DatabaseException.h"

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

void Schema::configureRelationships()
{
  const auto checkTableIdExisting = [this](Schema::Id relId, Schema::Id tableId)
  {
    if (m_tables.count(tableId) == 0)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        QString("Relationship with id %1 references an unknown table with id %2.").arg(relId).arg(tableId));
    }
  };

  for (auto& relationship : m_relationships)
  {
    checkTableIdExisting(relationship.first, relationship.second.tableFromId);
    checkTableIdExisting(relationship.first, relationship.second.tableToId);

    if ((relationship.second.type == Schema::RelationshipType::ManyToOne) ||
      (relationship.second.type == Schema::RelationshipType::OneToMany))
    {
      auto parentTableId = relationship.second.tableFromId;
      auto childTableId = relationship.second.tableToId;

      if (relationship.second.type == Schema::RelationshipType::ManyToOne)
      {
        std::swap(parentTableId, childTableId);
      }

      const auto& parentTable = m_tables.at(parentTableId);
      auto& childTable = m_tables.at(childTableId);

      Schema::Column parentIdColumn;
      for (const auto& col : parentTable.columns)
      {
        if (col.second.bIsPrimaryKey)
        {
          parentIdColumn = col.second;
          break;
        }
      }

      if (parentIdColumn.name.isEmpty())
      {
        throw DatabaseException(DatabaseException::Type::UnableToLoad,
          QString("Relationship with id %1 expects the table '%2' to have a primary key column").arg(relationship.first).arg(parentTable.name));
      }

      auto nextAvailableChildTableColid = 0U;
      while (childTable.columns.count(nextAvailableChildTableColid) > 0)
      {
        nextAvailableChildTableColid++;
      }

      Schema::Column foreignKeyColumn;
      foreignKeyColumn.name = QString("rel_%1_foreign_key").arg(relationship.first);
      foreignKeyColumn.type = parentIdColumn.type;
      foreignKeyColumn.varcharLength = parentIdColumn.varcharLength;

      childTable.columns[nextAvailableChildTableColid] = foreignKeyColumn;

      relationship.second.foreignKeyColId = nextAvailableChildTableColid;
    }
    else if (relationship.second.type == Schema::RelationshipType::ManyToMany)
    {
      
    }
  }
}

}
