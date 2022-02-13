#include "QtSqlLib/Query/InsertIntoExt.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Query/InsertInto.h"

namespace QtSqlLib::Query
{

InsertIntoExt::InsertIntoExt(Schema::Id tableId)
  : QuerySequence()
  , m_tableId(tableId)
  , m_bIsReturningInsertedIds(false)
{
}

InsertIntoExt::~InsertIntoExt() = default;

InsertIntoExt& InsertIntoExt::value(Schema::Id columnId, const QVariant& value)
{
  getOrCreateInsertQuery()->value(columnId, value);
  return *this;
}

InsertIntoExt& InsertIntoExt::linkTuple(Schema::Id relationshipId, const Schema::TableColumnValuesMap& tupleIdsMap)
{
  if (m_linkedTuple.count(relationshipId) > 0)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax,
      QString("More than one linked tuple of same relationship with id %1 specified.").arg(relationshipId));
  }
  
  m_linkedTuple[relationshipId] = tupleIdsMap;
  return *this;
}

InsertIntoExt& InsertIntoExt::returnIds()
{
  if (m_bIsReturningInsertedIds)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, 
      "returnId() can only be called once per query.");
  }

  m_bIsReturningInsertedIds = true;
  return *this;
}

void InsertIntoExt::prepare(Schema& schema)
{
  getOrCreateInsertQuery();

  schema.throwIfTableIdNotExisting(m_tableId);

  const auto& table = schema.getTables().at(m_tableId);

  std::vector<QVariant> foreignKeyValues;

  const auto& relationships = schema.getRelationships();
  for (const auto& linkedTuple : m_linkedTuple)
  {
    schema.throwIfRelationshipIdNotExisting(linkedTuple.first);
    const auto& relationship = relationships.at(linkedTuple.first);

    if ((relationship.type == Schema::RelationshipType::ManyToMany) ||
      ((relationship.type == Schema::RelationshipType::OneToMany) && (relationship.tableFromId == m_tableId)) ||
      ((relationship.type == Schema::RelationshipType::ManyToOne) && (relationship.tableToId == m_tableId)))
    {
      throw DatabaseException(DatabaseException::Type::QueryError, 
        QString("Direct linking to related tuple not allowed for relationship with id %1 to table with id %2.")
        .arg(linkedTuple.first).arg(m_tableId));
    }

    const auto parentTableId = (relationship.type == Schema::RelationshipType::OneToMany ? relationship.tableFromId : relationship.tableToId);
    const auto& parentTable = schema.getTables().at(parentTableId);

    const auto& foreignKeyReferences = table.mapRelationshioToForeignKeyReferences.at(linkedTuple.first).at(0);
    for (const auto& parentKeyCol : parentTable.primaryKeys)
    {
      if (linkedTuple.second.count({ parentTableId, parentKeyCol }) == 0)
      {
        throw DatabaseException(DatabaseException::Type::QueryError,
          QString("Missing primary key of tuple hat should be linked ('%1').").arg(parentTable.columns.at(parentKeyCol).name));
      }

      m_insertQuery->addColumnId(foreignKeyReferences.mapReferenceParentColIdToChildColId.at({ parentTableId, parentKeyCol }));
      foreignKeyValues.emplace_back(linkedTuple.second.at({ parentTableId, parentKeyCol }));
    }
  }

  m_insertQuery->setForeignKeyValues(foreignKeyValues);

  addQuery(std::move(m_insertQuery));

  if (m_bIsReturningInsertedIds)
  {
    addQuery(std::make_unique<QueryInsertedIds>(m_tableId));
  }
}

InsertIntoExt::InsertIntoReferences::InsertIntoReferences(Schema::Id tableId)
  : InsertInto(tableId)
{
}

InsertIntoExt::InsertIntoReferences::~InsertIntoReferences() = default;

void InsertIntoExt::InsertIntoReferences::setForeignKeyValues(const std::vector<QVariant>& values)
{
  m_foreignKeyValues = values;
}

void InsertIntoExt::InsertIntoReferences::bindQueryValues(QSqlQuery& query) const
{
  InsertInto::bindQueryValues(query);

  for (const auto& value : m_foreignKeyValues)
  {
    query.addBindValue(value);
  }
}

InsertIntoExt::QueryInsertedIds::QueryInsertedIds(Schema::Id tableId)
  : Query()
  , m_tableId(tableId)
{
}

InsertIntoExt::QueryInsertedIds::~QueryInsertedIds() = default;

API::IQuery::SqlQuery InsertIntoExt::QueryInsertedIds::getSqlQuery(Schema& schema)
{
  schema.throwIfTableIdNotExisting(m_tableId);
  const auto& table = schema.getTables().at(m_tableId);

  QString keyColumns;
  for (const auto& primaryKey : table.primaryKeys)
  {
    keyColumns += QString("'%1'.'%2', ").arg(table.name).arg(table.columns.at(primaryKey).name);
  }
  keyColumns = keyColumns.left(keyColumns.length() - 2);

  return { QSqlQuery(QString("SELECT rowid, %1 FROM '%2' WHERE rowid = last_insert_rowid();")
    .arg(keyColumns).arg(table.name)) };
}

API::IQuery::QueryResults InsertIntoExt::QueryInsertedIds::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  const auto& table = schema.getTables().at(m_tableId);

  if (!query.next())
  {
    throw DatabaseException(DatabaseException::Type::QueryError, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  Schema::TableColumnValuesMap resultsMap;

  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    resultsMap[{ m_tableId, primaryKey }] = query.value(currentValue);
    currentValue++;
  }

  return { resultsMap };
}

std::unique_ptr<InsertIntoExt::InsertIntoReferences>& InsertIntoExt::getOrCreateInsertQuery()
{
  if (!m_insertQuery)
  {
    m_insertQuery = std::make_unique<InsertIntoReferences>(m_tableId);
  }
  return m_insertQuery;
}

}
