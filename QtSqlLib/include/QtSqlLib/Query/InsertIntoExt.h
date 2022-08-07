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
  InsertIntoExt(API::ISchema::Id tableId);
  ~InsertIntoExt() override;

  InsertIntoExt& value(API::ISchema::Id columnId, const QVariant& value);
  InsertIntoExt& linkToOneTuple(API::ISchema::Id relationshipId, const API::ISchema::TupleValues& tupleKeyValues);
  InsertIntoExt& linkToManyTuples(API::ISchema::Id relationshipId, const std::vector<API::ISchema::TupleValues>& tupleKeyValuesList);

  InsertIntoExt& returnIds();

  void prepare(API::ISchema& schema) override;

private:
  class InsertIntoReferences : public InsertInto
  {
  public:
    InsertIntoReferences(API::ISchema::Id tableId);
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
    QueryInsertedIds(API::ISchema::Id tableId);
    ~QueryInsertedIds() override;

    SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, QueryResults& previousQueryResults) override;
    QueryResults getQueryResults(API::ISchema& schema, QSqlQuery& query) const override;

  private:
    API::ISchema::Id m_tableId;

  };

  void throwIdLinkedTupleAlreadyExisting(API::ISchema::Id relationshipId) const;

  std::unique_ptr<InsertIntoReferences>& getOrCreateInsertQuery();
  bool isSeparateLinkTuplesQueryNeeded(const API::ISchema::Relationship& relationship) const;

  std::unique_ptr<InsertIntoReferences> m_insertQuery;

  enum class LinkType
  {
    ToOne,
    ToMany
  };

  struct LinkedTuples
  {
    LinkType linkType;
    std::vector<API::ISchema::TupleValues> linkedPrimaryKeys;
  };

  API::ISchema::Id m_tableId;
  bool m_bIsReturningInsertedIds;
  std::map<API::ISchema::Id, LinkedTuples> m_linkedTuplesMap;

  void addUpdateForeignKeyColumnsToInsertIntoQuery(API::ISchema& schema, API::ISchema::Id relationshipId,
                                                   const API::ISchema::Relationship& relationship,
                                                   const API::ISchema::Table& childTable,
                                                   const LinkedTuples& linkedTuples) const;

  void addLinkTuplesQueriesForRelationshipIds(const std::set<API::ISchema::Id>& relationshipIds);

};

}
