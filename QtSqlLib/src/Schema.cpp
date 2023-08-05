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
        relationship.second.onDeleteAction };

      for (const auto& parentKeyColId : parentPrimaryKeyColIds)
      {
        auto nextAvailableChildTableColid = 0U;
        while (childTable.columns.count(nextAvailableChildTableColid) > 0)
        {
          nextAvailableChildTableColid++;
        }

        const auto& parentKeyCol = parentTable.columns.at(parentKeyColId);

        API::Column foreignKeyColumn;
        foreignKeyColumn.name = QString("rel_%1_foreign_key_%2").arg(relationship.first).arg(parentKeyCol.name);
        foreignKeyColumn.type = parentKeyCol.type;
        foreignKeyColumn.varcharLength = parentKeyCol.varcharLength;

        childTable.columns[nextAvailableChildTableColid] = foreignKeyColumn;
        foreignKeyReference.primaryForeignKeyColIdMap[{ parentTableId, parentKeyColId }] = nextAvailableChildTableColid;
      }

      childTable.relationshipToForeignKeyReferencesMap[{ relationship.first, parentTableId }].emplace_back(foreignKeyReference);
    }
    else if (relationship.second.type == API::RelationshipType::ManyToMany)
    {
      API::Table linkTable;
      linkTable.name = QString("rel_%1_link_%2_to_%3")
        .arg(relationship.first).arg(parentTable.name).arg(childTable.name);

      auto currentColId = 0U;

      const auto addRefTableColumns = [&linkTable, &currentColId, &relationship](API::IID::Type refTableId, const API::Table& refTable)
      {
        API::ForeignKeyReference foreignKeyReference { refTableId,
          relationship.second.onUpdateAction,
          relationship.second.onDeleteAction };

        for (const auto& refColId : refTable.primaryKeys)
        {
          const auto& refCol = refTable.columns.at(refColId);

          API::Column col;
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

API::IID::Type Schema::validatePrimaryKeysAndGetTableId(const API::TupleValues& tupleKeyValues) const
{
  if (tupleKeyValues.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Expected at least one column.");
  }

  auto firstTableIdSet = false;
  API::IID::Type tableId = 0;
  std::set<API::IID::Type> colIds;

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

  m_sanityChecker->throwIfTableIdNotExisting(tableId);
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

API::IID::Type Schema::validatePrimaryKeysListAndGetTableId(const std::vector<API::TupleValues>& tupleKeyValuesList) const
{
  if (tupleKeyValuesList.empty())
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      "Primary key column values must not be empty.");
  }

  auto firstTableIdSet = false;
  API::IID::Type tableId = 0;
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

  m_sanityChecker->throwIfTableIdNotExisting(tableId);

  return tableId;
}

std::pair<API::IID::Type, API::IID::Type> Schema::verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
  API::IID::Type relationshipId,
  const API::TupleValues& fromTupleKeyValues,
  const API::TupleValues& toTupleKeyValues) const
{
  return verifyRelationshipPrimaryKeysAndGetTableIds(false, relationshipId, fromTupleKeyValues, { toTupleKeyValues });
}

std::pair<API::IID::Type, API::IID::Type> Schema::verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
  API::IID::Type relationshipId,
  const API::TupleValues& fromTupleKeyValues,
  const std::vector<API::TupleValues>& toTupleKeyValuesList) const
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
  const API::TupleValues& fromTupleKeyValues,
  const std::vector<API::TupleValues>& toTupleKeyValuesList) const
{
  m_sanityChecker->throwIfRelationshipIsNotExisting(relationshipId);
  const auto& relationship = m_relationships.at(relationshipId);
  const auto bIgnoreFromKeys = fromTupleKeyValues.empty();

  const auto tableToId = validatePrimaryKeysListAndGetTableId(toTupleKeyValuesList);
  const auto expectedTableFromId = (tableToId == relationship.tableToId
    ? relationship.tableFromId
    : relationship.tableToId);

  const auto tableFromId = (!bIgnoreFromKeys
    ? validatePrimaryKeysAndGetTableId(fromTupleKeyValues)
    : expectedTableFromId);

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
