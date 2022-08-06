#include "QtSqlLib/Schema.h"

#include "QtSqlLib/DatabaseException.h"

#include <set>

namespace QtSqlLib
{

bool Schema::TableColumnId::operator<(const TableColumnId& rhs) const
{
  if (tableId == rhs.tableId)
  {
    return columnId < rhs.columnId;
  }
  return tableId < rhs.tableId;
}

bool Schema::TableColumnId::operator!=(const TableColumnId& rhs) const
{
  return (tableId != rhs.tableId) || (columnId != rhs.columnId);
}

bool Schema::RelationshipTableId::operator<(const RelationshipTableId& rhs) const
{
  if (relationshipId == rhs.relationshipId)
  {
    return tableId < rhs.tableId;
  }
  return relationshipId < rhs.relationshipId;
}

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
  throwIfRelationshipIsNotExisting(relationshipId);
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
        foreignKeyReference.primaryForeignKeyColIdMap[{ parentTableId, parentKeyColId }] = nextAvailableChildTableColid;
      }

      childTable.relationshipToForeignKeyReferencesMap[{ relationship.first, parentTableId }].emplace_back(foreignKeyReference);
    }
    else if (relationship.second.type == RelationshipType::ManyToMany)
    {
      Table linkTable;
      linkTable.name = QString("rel_%1_link_%2_to_%3")
        .arg(relationship.first).arg(parentTable.name).arg(childTable.name);

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
          col.name = QString("%1_%2_%3").arg(refTable.name).arg(refCol.name).arg(currentColId);
          col.type = refCol.type;

          foreignKeyReference.primaryForeignKeyColIdMap[{ refTableId, refColId }] = currentColId;
          linkTable.columns[currentColId] = col;
          linkTable.primaryKeys.insert(currentColId);

          currentColId++;
        }

        linkTable.relationshipToForeignKeyReferencesMap[{ relationship.first, refTableId }].emplace_back(foreignKeyReference);
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

void Schema::throwIfRelationshipIsNotExisting(Id relationshipId) const
{
  if (m_relationships.count(relationshipId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown relationship id: %1.").arg(relationshipId));
  }
}

void Schema::throwIfColumnIdNotExisting(const Table& table, Id colId)
{
  if (table.columns.count(colId) == 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidId,
      QString("Unknown column id %1 in table '%2'.").arg(colId).arg(table.name));
  }
}

Schema::Id Schema::validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const
{
  if (tupleKeyValues.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Expected at least one column.");
  }

  auto firstTableIdSet = false;
  Id tableId = 0;
  std::set<Id> colIds;

  for (const auto& value : tupleKeyValues)
  {
    if (!firstTableIdSet)
    {
      tableId = value.first.tableId;
      firstTableIdSet = true;
    }
    else if (tableId != value.first.tableId)
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Inconsistent table ids detected. Columns should reference only a single table.");
    }

    colIds.insert(value.first.columnId);
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

Schema::Id Schema::validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const
{
  if (tupleKeyValuesList.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Primary key column values must not be empty.");
  }

  auto firstTableIdSet = false;
  Schema::Id tableId = 0;
  for (const auto& tupleKeyValues : tupleKeyValuesList)
  {
    if (!firstTableIdSet)
    {
      tableId = validatePrimaryKeysAndGetTableId(tupleKeyValues);
      firstTableIdSet = true;
      continue;
    }

    if (tableId != validatePrimaryKeysAndGetTableId(tupleKeyValues))
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Primary keys should belong to a single table.");
    }
  }

  throwIfTableIdNotExisting(tableId);

  return tableId;
}

std::pair<Schema::Id, Schema::Id> Schema::verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
  Schema::Id relationshipId,
  const TupleValues& fromTupleKeyValues,
  const TupleValues& toTupleKeyValues) const
{
  return verifyRelationshipPrimaryKeysAndGetTableIds(false, relationshipId, fromTupleKeyValues, { toTupleKeyValues });
}

std::pair<Schema::Id, Schema::Id> Schema::verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
  Schema::Id relationshipId,
  const TupleValues& fromTupleKeyValues,
  const std::vector<TupleValues>& toTupleKeyValuesList) const
{
  return verifyRelationshipPrimaryKeysAndGetTableIds(true, relationshipId, fromTupleKeyValues, toTupleKeyValuesList);
}

std::pair<Schema::Id, Schema::Id> Schema::verifyRelationshipPrimaryKeysAndGetTableIds(
  bool bIsOneToMany,
  Schema::Id relationshipId,
  const TupleValues& fromTupleKeyValues,
  const std::vector<TupleValues>& toTupleKeyValuesList) const
{
  throwIfRelationshipIsNotExisting(relationshipId);
  const auto& relationship = m_relationships.at(relationshipId);
  const auto bIgnoreFromKeys = fromTupleKeyValues.empty();

  const auto tableToId = validatePrimaryKeysListAndGetTableId(toTupleKeyValuesList);
  const auto expectedTableFromId = (tableToId == relationship.tableToId
    ? relationship.tableFromId
    : relationship.tableToId);

  const auto tableFromId = (!bIgnoreFromKeys
    ? validatePrimaryKeysAndGetTableId(fromTupleKeyValues)
    : expectedTableFromId);

  const auto isOneToMany = (relationship.type == Schema::RelationshipType::OneToMany);
  const auto isManyToOne = (relationship.type == Schema::RelationshipType::ManyToOne);

  if (bIsOneToMany)
  {
    if ((relationship.type != Schema::RelationshipType::ManyToMany) &&
      ((isManyToOne && (relationship.tableToId == tableToId)) ||
        (isOneToMany && (relationship.tableToId != tableToId))))
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Not possible to assign multiple tuple keys for this relationship type.");
    }
  }

  if (!isTableIdsMatching(relationship, tableFromId, tableToId, bIgnoreFromKeys))
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Given entity keys not are not matching relationship table ids.");
  }

  return { tableFromId, tableToId };
}

bool Schema::isTableIdsMatching(
  const Schema::Relationship& relationship,
  Schema::Id tableFromId,
  Schema::Id tableToId,
  bool bIgnoreFromKeys)
{
  if (bIgnoreFromKeys)
  {
    return (tableToId == relationship.tableToId) || (tableToId == relationship.tableFromId);
  }

  return (((tableToId == relationship.tableToId) && (tableFromId == relationship.tableFromId)) ||
    ((tableToId == relationship.tableFromId) && (tableFromId == relationship.tableToId)));
}

}
