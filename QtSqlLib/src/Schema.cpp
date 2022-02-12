#include "QtSqlLib/Schema.h"

#include "QtSqlLib/DatabaseException.h"

#include <set>

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

Schema::Id Schema::getManyToManyLinkTableId(Id relationshipId) const
{
  throwIfRelationshipIdNotExisting(relationshipId);
  return m_mapManyToManyRelationshipToLinkTableId.at(relationshipId);
}

void Schema::configureRelationships()
{
  const auto checkTableIdExisting = [this](Id relId, Id tableId)
  {
    if (m_tables.count(tableId) == 0)
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
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
        throw DatabaseException(DatabaseException::Type::InvalidSyntax,
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
        foreignKeyReference.mapReferenceParentColIdToChildColId[{ parentTableId, parentKeyColId }] = nextAvailableChildTableColid;
      }

      childTable.mapRelationshioToForeignKeyReferences[relationship.first].emplace_back(foreignKeyReference);
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

          foreignKeyReference.mapReferenceParentColIdToChildColId[{ refTableId, refColId }] = currentColId;
          linkTable.columns[currentColId] = col;
          linkTable.primaryKeys.insert(currentColId);

          currentColId++;
        }

        linkTable.mapRelationshioToForeignKeyReferences[relationship.first].emplace_back(foreignKeyReference);
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

void Schema::throwIfTableIdNotExisting(Id tableId) const
{
  if (m_tables.count(tableId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown table id: %1.").arg(tableId));
  }
}

void Schema::throwIfRelationshipIdNotExisting(Id relationshipId) const
{
  if (m_relationships.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown relationship id: %1.").arg(relationshipId));
  }
}

void Schema::throwIfColumnIdNotExisting(const Table& table, Id colId) const
{
  if (table.columns.count(colId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown column id %1 in table '%2'.").arg(colId).arg(table.name));
  }
}

Schema::Id Schema::validatePrimaryKeysAndGetTableId(const TableColumnValuesMap& columnValues) const
{
  if (columnValues.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Expected at least one column.");
  }

  auto firstTableIdSet = false;
  Id tableId = 0;
  std::set<Id> colIds;

  for (const auto& colValue : columnValues)
  {
    if (!firstTableIdSet)
    {
      tableId = colValue.first.first;
      firstTableIdSet = true;
    }
    else if (tableId != colValue.first.first)
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Inconsistent table ids detected. Columns should reference only a single table.");
    }

    colIds.insert(colValue.first.second);
  }

  throwIfTableIdNotExisting(tableId);
  const auto& table = m_tables.at(tableId);

  for (const auto& primaryKey : table.primaryKeys)
  {
    if (colIds.count(primaryKey) == 0)
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Column id expected to be a primary key.");
    }
  }

  if (table.primaryKeys.size() != colIds.size())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Wrong number of primary keys given.");
  }

  return tableId;
}

}
