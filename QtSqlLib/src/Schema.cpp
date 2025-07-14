#include "QtSqlLib/Schema.h"

#include "QtSqlLib/API/ISanityChecker.h"
#include "QtSqlLib/DatabaseException.h"

#include <set>

namespace QtSqlLib
{

Schema::Schema() = default;

Schema::~Schema() = default;

std::map<API::IID::Type, API::Table>& Schema::getTables()
{
  return m_tables;
}

std::map<API::IID::Type, API::Relationship>& Schema::getRelationships()
{
  return m_relationships;
}

std::vector<API::Index>& Schema::getIndices()
{
  return m_indices;
}

const API::ISanityChecker& Schema::getSanityChecker() const
{
  return *m_sanityChecker;
}

API::IID::Type Schema::getManyToManyLinkTableId(API::IID::Type relationshipId) const
{
  m_sanityChecker->throwIfRelationshipIsNotExisting(relationshipId);
  return m_mapManyToManyRelationshipToLinkTableId.at(relationshipId);
}

void Schema::configureRelationships()
{
  const auto checkTableIdExisting = [this](API::IID::Type relId, API::IID::Type tableId)
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

    if (relationship.second.type == API::RelationshipType::ManyToOne)
    {
      std::swap(parentTableId, childTableId);
    }

    const auto& parentTable = m_tables.at(parentTableId);
    auto& childTable = m_tables.at(childTableId);

    if ((relationship.second.type == API::RelationshipType::ManyToOne) ||
      (relationship.second.type == API::RelationshipType::OneToMany))
    {
      const auto parentPrimaryKeyColIds = parentTable.primaryKeys;

      if (parentPrimaryKeyColIds.empty())
      {
        throw DatabaseException(DatabaseException::Type::InvalidSyntax,
          QString("Relationship with id %1 expects the table '%2' to have a primary key column").arg(relationship.first).arg(parentTable.name));
      }

      API::ForeignKeyReference foreignKeyReference {
        parentTableId,
        relationship.second.onUpdateAction,
        relationship.second.onDeleteAction,
        {} };

      ColumnHelper::SelectColumnList indexedColumns;
      for (const auto& keyCol : parentPrimaryKeyColIds)
      {
        API::IID::Type nextAvailableChildTableColid = 0;
        while (childTable.columns.count(nextAvailableChildTableColid) > 0)
        {
          nextAvailableChildTableColid++;
        }

        const auto& parentKeyCol = parentTable.columns.at(keyCol.columnId);

        API::Column foreignKeyColumn;
        foreignKeyColumn.name = QString("rel_%1_foreign_key_%2").arg(relationship.first).arg(parentKeyCol.name);
        foreignKeyColumn.type = parentKeyCol.type;
        foreignKeyColumn.varcharLength = parentKeyCol.varcharLength;

        childTable.columns[nextAvailableChildTableColid] = foreignKeyColumn;
        foreignKeyReference.primaryForeignKeyColIdMap[keyCol.columnId] = nextAvailableChildTableColid;

        if (relationship.second.bForeignKeyIndexingEnabled)
        {
          indexedColumns.emplace_back(ColumnHelper::SelectColumn { nextAvailableChildTableColid });
        }
      }

      if (relationship.second.bForeignKeyIndexingEnabled)
      {
        API::Index index;
        index.tableId = childTableId;
        index.columns = indexedColumns;
        index.columns.shrink_to_fit();
        m_indices.emplace_back(index);
      }

      childTable.relationshipToForeignKeyReferencesMap[{ relationship.first, parentTableId }].emplace_back(foreignKeyReference);
    }
    else if (relationship.second.type == API::RelationshipType::ManyToMany)
    {
      API::Table linkTable;
      linkTable.name = QString("rel_%1_link_%2_to_%3")
        .arg(relationship.first).arg(parentTable.name).arg(childTable.name);

      auto currentColId = 0U;
      auto nextAvailableTableId = 0U;
      while (m_tables.count(nextAvailableTableId) > 0)
      {
        nextAvailableTableId++;
      }

      const auto addRefTableColumns = [this, &linkTable, &currentColId, &relationship, nextAvailableTableId]
        (API::IID::Type refTableId, const API::Table& refTable)
      {
        API::ForeignKeyReference foreignKeyReference {
          refTableId,
          relationship.second.onUpdateAction,
          relationship.second.onDeleteAction,
          {} };

        ColumnHelper::SelectColumnList indexedColumns;
        for (const auto& refKeyCol : refTable.primaryKeys)
        {
          const auto& refCol = refTable.columns.at(refKeyCol.columnId);

          API::Column col;
          col.name = QString("%1_%2_%3").arg(refTable.name).arg(refCol.name).arg(currentColId);
          col.type = refCol.type;

          foreignKeyReference.primaryForeignKeyColIdMap[refKeyCol.columnId] = currentColId;
          linkTable.columns[currentColId] = col;
          linkTable.primaryKeys.emplace_back(currentColId);

          if (relationship.second.bForeignKeyIndexingEnabled)
          {
            indexedColumns.emplace_back(ColumnHelper::SelectColumn { currentColId });
          }

          currentColId++;
        }

        linkTable.relationshipToForeignKeyReferencesMap[{ relationship.first, refTableId }].emplace_back(foreignKeyReference);

        if (relationship.second.bForeignKeyIndexingEnabled)
        {
          API::Index index;
          index.tableId = nextAvailableTableId;
          index.columns = indexedColumns;
          index.columns.shrink_to_fit();
          m_indices.emplace_back(index);
        }
      };

      addRefTableColumns(parentTableId, parentTable);
      addRefTableColumns(childTableId, childTable);

      m_tables[nextAvailableTableId] = linkTable;
      m_mapManyToManyRelationshipToLinkTableId[relationship.first] = nextAvailableTableId;
    }
  }
}

void Schema::validateAndPrepareIndices()
{
  std::map<API::IID::Type, int> tableIndexCounter;

  for (auto& index : m_indices)
  {
    if (m_tables.count(index.tableId) == 0)
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("Index cannot be created for non-existing table id '%1'").arg(index.tableId));
    }

    const auto& table = m_tables.at(index.tableId);
    for (const auto& col : index.columns)
    {
      if (table.columns.count(col.columnId) == 0)
      {
        throw DatabaseException(DatabaseException::Type::InvalidId,
          QString("Index cannot be created. Table with id '%1' has no column id '%2'").arg(index.tableId).arg(col.columnId));
      }
    }

    if (tableIndexCounter.count(index.tableId) == 0)
    {
      tableIndexCounter[index.tableId] = 1;
    }
    index.name = QString("index_%1_%2").arg(table.name).arg(tableIndexCounter[index.tableId]++);
  }
}

void Schema::validatePrimaryKeys(const PrimaryKey& tupleKeyValues) const
{
  if (tupleKeyValues.isNull())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Expected at least one column.");
  }

  std::set<API::IID::Type> colIds;
  for (const auto& value : tupleKeyValues.values())
  {
    colIds.insert(value.columnId);
  }

  const auto tableId = tupleKeyValues.tableId();
  m_sanityChecker->throwIfTableIdNotExisting(tableId);
  const auto& table = m_tables.at(tableId);

  for (const auto& primaryKey : table.primaryKeys)
  {
    if (colIds.count(primaryKey.columnId) == 0)
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
}

void Schema::validatePrimaryKeysList(const std::vector<PrimaryKey>& tupleKeyValuesList) const
{
  if (tupleKeyValuesList.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Primary key column values must not be empty.");
  }

  auto firstTableIdSet = false;
  API::IID::Type lastTableId = 0;
  for (const auto& tupleKeyValues : tupleKeyValuesList)
  {
    validatePrimaryKeys(tupleKeyValues);
    const auto currentTableId = tupleKeyValues.tableId();

    if (firstTableIdSet && lastTableId != currentTableId)
    {
      throw DatabaseException(DatabaseException::Type::InvalidSyntax,
        "Primary keys should belong to a single table.");
    }

    firstTableIdSet = true;
    lastTableId = currentTableId;
  }

  m_sanityChecker->throwIfTableIdNotExisting(lastTableId);
}

std::pair<API::IID::Type, API::IID::Type> Schema::verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
  API::IID::Type relationshipId,
  const PrimaryKey& fromTupleKeyValues,
  const PrimaryKey& toTupleKeyValues) const
{
  return verifyRelationshipPrimaryKeysAndGetTableIds(false, relationshipId, fromTupleKeyValues, { toTupleKeyValues });
}

std::pair<API::IID::Type, API::IID::Type> Schema::verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
  API::IID::Type relationshipId,
  const PrimaryKey& fromTupleKeyValues,
  const std::vector<PrimaryKey>& toTupleKeyValuesList) const
{
  return verifyRelationshipPrimaryKeysAndGetTableIds(true, relationshipId, fromTupleKeyValues, toTupleKeyValuesList);
}

void Schema::setSanityChecker(std::unique_ptr<API::ISanityChecker> sanityChecker)
{
  m_sanityChecker = std::move(sanityChecker);
}

std::pair<API::IID::Type, API::IID::Type> Schema::verifyRelationshipPrimaryKeysAndGetTableIds(
  bool bIsOneToMany,
  API::IID::Type relationshipId,
  const PrimaryKey& fromTupleKeyValues,
  const std::vector<PrimaryKey>& toTupleKeyValuesList) const
{
  m_sanityChecker->throwIfRelationshipIsNotExisting(relationshipId);
  const auto& relationship = m_relationships.at(relationshipId);
  const auto bIgnoreFromKeys = fromTupleKeyValues.isNull();

  validatePrimaryKeysList(toTupleKeyValuesList);

  const auto tableToId = toTupleKeyValuesList.at(0).tableId();
  const auto expectedTableFromId = (tableToId == relationship.tableToId
    ? relationship.tableFromId
    : relationship.tableToId);

  
  auto tableFromId = 0;
  if (bIgnoreFromKeys)
  {
    tableFromId = expectedTableFromId;
  }
  else
  {
    validatePrimaryKeys(fromTupleKeyValues);
    tableFromId = fromTupleKeyValues.tableId();
  }

  const auto isOneToMany = (relationship.type == API::RelationshipType::OneToMany);
  const auto isManyToOne = (relationship.type == API::RelationshipType::ManyToOne);

  if (bIsOneToMany)
  {
    if ((relationship.type != API::RelationshipType::ManyToMany) &&
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
  const API::Relationship& relationship,
  API::IID::Type tableFromId,
  API::IID::Type tableToId,
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
