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
  const auto checkTableIdExisting = [this](Id relId, Id tableId)
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

    if ((relationship.second.type == RelationshipType::ManyToOne) ||
      (relationship.second.type == RelationshipType::OneToMany))
    {
      auto parentTableId = relationship.second.tableFromId;
      auto childTableId = relationship.second.tableToId;

      if (relationship.second.type == RelationshipType::ManyToOne)
      {
        std::swap(parentTableId, childTableId);
      }

      const auto& parentTable = m_tables.at(parentTableId);
      auto& childTable = m_tables.at(childTableId);

      const auto parentPrimaryKeyColIds = parentTable.primaryKeys;

      if (parentPrimaryKeyColIds.empty())
      {
        throw DatabaseException(DatabaseException::Type::UnableToLoad,
          QString("Relationship with id %1 expects the table '%2' to have a primary key column").arg(relationship.first).arg(parentTable.name));
      }

      const ForeignKeyReference foreignKeyReference { parentTableId, parentPrimaryKeyColIds,
                                                      relationship.second.onUpdateAction,
                                                      relationship.second.onDeleteAction };

      for (const auto& parentKeyColId : parentPrimaryKeyColIds)
      {
        auto nextAvailableChildTableColid = 0U;
        while (childTable.columns.count(nextAvailableChildTableColid) > 0)
        {
          nextAvailableChildTableColid++;
        }

        const auto& parentKeyCol = parentTable.columns.at(parentKeyColId);

        Column foreignKeyColumn;
        foreignKeyColumn.name = QString("rel_%1_foreign_key_%2").arg(relationship.first).arg(parentKeyCol.name);
        foreignKeyColumn.type = parentKeyCol.type;
        foreignKeyColumn.varcharLength = parentKeyCol.varcharLength;

        childTable.columns[nextAvailableChildTableColid] = foreignKeyColumn;
        childTable.foreignKeyReferences[nextAvailableChildTableColid] = foreignKeyReference;
      }
    }
    else if (relationship.second.type == RelationshipType::ManyToMany)
    {
      
    }
  }
}

}
