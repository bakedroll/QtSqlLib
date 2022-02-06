#include "QtSqlLib/InsertIntoExt.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/InsertInto.h"

namespace QtSqlLib
{

InsertIntoExt::InsertIntoExt(Schema::Id tableId)
  : QuerySequence()
{
  addQuery(std::make_unique<InsertIntoReferences>(tableId));
}

InsertIntoExt::~InsertIntoExt() = default;

InsertIntoExt& InsertIntoExt::value(Schema::Id columnId, const QVariant& value)
{
  auto& queryInsert = dynamic_cast<InsertIntoReferences&>(getQuery(0));
  queryInsert.value(columnId, value);
  return *this;
}

InsertIntoExt& InsertIntoExt::linkTuple(Schema::Id relationshipId, const QueryDefines::ColumnResultMap& tupleIdsMap)
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
  if (getNumQueries() > 1)
  {
    throw DatabaseException(DatabaseException::Type::InvalidSyntax, 
      "returnId() can only be called once per query.");
  }

  auto& queryInsert = dynamic_cast<InsertIntoReferences&>(getQuery(0));

  addQuery(std::make_unique<QueryInsertedIds>(queryInsert.getTableId()));
  return *this;
}

void InsertIntoExt::prepare(Schema& schema)
{
  auto& queryInsert = dynamic_cast<InsertIntoReferences&>(getQuery(0));
  const auto tableId = queryInsert.getTableId();

  schema.throwIfTableIdNotExisting(tableId);

  const auto& table = schema.getTables().at(tableId);

  std::vector<QVariant> foreignKeyValues;

  const auto& relationships = schema.getRelationships();
  for (const auto& linkedTuple : m_linkedTuple)
  {
    if (relationships.count(linkedTuple.first) == 0)
    {
      throw DatabaseException(DatabaseException::Type::QueryError, 
        QString("No relationship found with with id %1.").arg(linkedTuple.first));
    }

    const auto& relationship = relationships.at(linkedTuple.first);

    if ((relationship.type == Schema::RelationshipType::ManyToMany) ||
      ((relationship.type == Schema::RelationshipType::OneToMany) && (relationship.tableFromId == tableId)) ||
      ((relationship.type == Schema::RelationshipType::ManyToOne) && (relationship.tableToId == tableId)))
    {
      throw DatabaseException(DatabaseException::Type::QueryError, 
        QString("Direct linking to related tuple not allowed for relationship with id %1 to table with id %2.")
        .arg(linkedTuple.first).arg(tableId));
    }

    const auto parentTableId = (relationship.type == Schema::RelationshipType::OneToMany ? relationship.tableFromId : relationship.tableToId);
    const auto& parentTable = schema.getTables().at(parentTableId);

    const auto& foreignKeyReferences = table.mapRelationshioToForeignKeyReferences.at(linkedTuple.first);
    for (const auto& parentKeyCol : parentTable.primaryKeys)
    {
      if (linkedTuple.second.count({ parentTableId, parentKeyCol }) == 0)
      {
        throw DatabaseException(DatabaseException::Type::QueryError,
          QString("Missing primary key of tuple hat should be linked ('%1').").arg(parentTable.columns.at(parentKeyCol).name));
      }

      queryInsert.addColumnId(foreignKeyReferences.mapReferenceParentColIdToChildColId.at(parentKeyCol));
      foreignKeyValues.emplace_back(linkedTuple.second.at({ parentTableId, parentKeyCol }));
    }
  }

  queryInsert.setForeignKeyValues(foreignKeyValues);
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
  : IQuery()
  , m_tableId(tableId)
{
}

InsertIntoExt::QueryInsertedIds::~QueryInsertedIds() = default;

QueryDefines::SqlQuery InsertIntoExt::QueryInsertedIds::getSqlQuery(Schema& schema)
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

QueryDefines::QueryResults InsertIntoExt::QueryInsertedIds::getQueryResults(Schema& schema, QSqlQuery& query) const
{
  const auto& table = schema.getTables().at(m_tableId);

  if (!query.next())
  {
    throw DatabaseException(DatabaseException::Type::QueryError, 
      QString("Could not query last inserted id from table '%1'.").arg(table.name));
  }

  QueryDefines::ColumnResultMap resultsMap;

  auto currentValue = 1;
  for (const auto& primaryKey : table.primaryKeys)
  {
    resultsMap[{ m_tableId, primaryKey }] = query.value(currentValue);
    currentValue++;
  }

  return { resultsMap };
}

}
