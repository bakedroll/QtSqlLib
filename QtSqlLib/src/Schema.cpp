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

    auto parentTableId = relationship.second.tableFromId;
    auto childTableId = relationship.second.tableToId;

    if (relationship.second.type == RelationshipType::ManyToOne)
    {
      std::swap(parentTableId, childTableId);
    }

    const auto& parentTable = m_tables.at(parentTableId);
    auto& childTable = m_tables.at(childTableId);

    if ((relationship.second.type == RelationshipType::ManyToOne) ||
      (relationship.second.type == RelationshipType::OneToMany))
    {
      const auto parentPrimaryKeyColIds = parentTable.primaryKeys;

      if (parentPrimaryKeyColIds.empty())
      {
        throw DatabaseException(DatabaseException::Type::UnableToLoad,
          QString("Relationship with id %1 expects the table '%2' to have a primary key column").arg(relationship.first).arg(parentTable.name));
      }

      ForeignKeyReference foreignKeyReference { parentTableId,
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
        foreignKeyReference.mapReferenceParentColIdToChildColId[parentKeyColId] = nextAvailableChildTableColid;
      }

      childTable.foreignKeyReferences.emplace_back(foreignKeyReference);
    }
    else if (relationship.second.type == RelationshipType::ManyToMany)
    {
      Table linkTable;
      linkTable.name = QString("link_%1_to_%2").arg(parentTable.name).arg(childTable.name);

      auto currentColId = 0U;

      const auto addRefTableColumns = [&linkTable, &currentColId, &relationship](Schema::Id refTableId, const Table& refTable)
      {
        ForeignKeyReference foreignKeyReference { refTableId,
          relationship.second.onUpdateAction,
          relationship.second.onDeleteAction };

        for (const auto& refColId : refTable.primaryKeys)
        {
          const auto& refCol = refTable.columns.at(refColId);

          Column col;
          col.name = QString("%1_%2").arg(refTable.name).arg(refCol.name);
          col.type = refCol.type;

          foreignKeyReference.mapReferenceParentColIdToChildColId[refColId] = currentColId;
          linkTable.columns[currentColId] = col;
          linkTable.primaryKeys.insert(currentColId);

          currentColId++;
        }

        linkTable.foreignKeyReferences.emplace_back(foreignKeyReference);
      };

      addRefTableColumns(parentTableId, parentTable);
      addRefTableColumns(childTableId, childTable);

      auto nextAvailableTableId = 0U;
      while (m_tables.count(nextAvailableTableId) > 0)
      {
        nextAvailableTableId++;
      }

      m_tables[nextAvailableTableId] = linkTable;
      m_mapManyToManyRelationshipToLinkTableId[relationship.first] = nextAvailableTableId;
    }
  }
}

}
