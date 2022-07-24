#pragma once

#include <QtSqlLib/Query/QuerySequence.h>
#include "QtSqlLib/Query/InsertInto.h"

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class InsertIntoExt : public QuerySequence
{
public:
  InsertIntoExt(Schema::Id tableId);
  ~InsertIntoExt() override;

  InsertIntoExt& value(Schema::Id columnId, const QVariant& value);
  InsertIntoExt& linkToOneTuple(Schema::Id relationshipId, const Schema::TupleValues& tupleKeyValues);
  InsertIntoExt& linkToManyTuples(Schema::Id relationshipId, const std::vector<Schema::TupleValues>& tupleKeyValuesList);

  InsertIntoExt& returnIds();

  void prepare(Schema& schema) override;

private:
  class InsertIntoReferences : public InsertInto
  {
  public:
    InsertIntoReferences(Schema::Id tableId);
    ~InsertIntoReferences() override;

    void addForeignKeyValue(const QVariant& value);

  protected:
    void bindQueryValues(QSqlQuery& query) const override;

  private:
    std::vector<QVariant> m_foreignKeyValues;

  };

  class QueryInsertedIds : public Query
  {
  public:
    QueryInsertedIds(Schema::Id tableId);
    ~QueryInsertedIds() override;

    SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;
    QueryResults getQueryResults(Schema& schema, QSqlQuery& query) const override;

  private:
    Schema::Id m_tableId;

  };

  void throwIdLinkedTupleAlreadyExisting(Schema::Id relationshipId) const;

  std::unique_ptr<InsertIntoReferences>& getOrCreateInsertQuery();
  bool isSeparateLinkTuplesQueryNeeded(const Schema::Relationship& relationship) const;

  std::unique_ptr<InsertIntoReferences> m_insertQuery;

  enum class LinkType
  {
    ToOne,
    ToMany
  };

  struct LinkedTuples
  {
    LinkType linkType;
    std::vector<Schema::TupleValues> linkedPrimaryKeys;
  };

  Schema::Id m_tableId;
  bool m_bIsReturningInsertedIds;
  std::map<Schema::Id, LinkedTuples> m_linkedTuplesMap;

  void addUpdateForeignKeyColumnsToInsertIntoQuery(Schema& schema, Schema::Id relationshipId,
                                                   const Schema::Relationship& relationship,
                                                   const Schema::Table& childTable,
                                                   const LinkedTuples& linkedTuples) const;

  void addLinkTuplesQueriesForRelationshipIds(const std::set<Schema::Id>& relationshipIds);

};

}
