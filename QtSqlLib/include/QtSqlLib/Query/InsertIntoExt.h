#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include "QtSqlLib/Query/InsertInto.h"

#include <QVariant>

#include <vector>

#define INSERT_INTO_EXT(X) QtSqlLib::Query::InsertIntoExt(QtSqlLib::ID(X))

#define VALUE(X, Y) value(QtSqlLib::ID(X), Y)
#define LINK_TO_ONE_TUPLE(X, Y) linkToOneTuple(QtSqlLib::ID(X), Y)
#define LINK_TO_MANY_TUPLES(X, ...) linkToManyTuples(QtSqlLib::ID(X), __VA_ARGS__)
#define RETURN_IDS returnIds()

namespace QtSqlLib::Query
{

class InsertIntoExt : public QuerySequence
{
public:
  InsertIntoExt(const API::IID& tableId);
  ~InsertIntoExt() override;

  InsertIntoExt& value(const API::IID& columnId, const QVariant& value);
  InsertIntoExt& linkToOneTuple(const API::IID& relationshipId, const API::TupleValues& tupleKeyValues);
  InsertIntoExt& linkToManyTuples(const API::IID& relationshipId, const std::vector<API::TupleValues>& tupleKeyValuesList);

  InsertIntoExt& returnIds();

  void prepare(API::ISchema& schema) override;

private:
  class InsertIntoReferences : public InsertInto
  {
  public:
    InsertIntoReferences(API::IID::Type tableId);
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
    QueryInsertedIds(API::IID::Type tableId);
    ~QueryInsertedIds() override;

    SqlQuery getSqlQuery(const QSqlDatabase& db, API::ISchema& schema, const ResultSet& previousQueryResults) override;
    ResultSet getQueryResults(API::ISchema& schema, QSqlQuery& query) const override;

  private:
    API::IID::Type m_tableId;

  };

  void throwIdLinkedTupleAlreadyExisting(API::IID::Type relationshipId) const;

  std::unique_ptr<InsertIntoReferences>& getOrCreateInsertQuery();
  bool isSeparateLinkTuplesQueryNeeded(const API::Relationship& relationship) const;

  std::unique_ptr<InsertIntoReferences> m_insertQuery;

  enum class LinkType
  {
    ToOne,
    ToMany
  };

  struct LinkedTuples
  {
    LinkType linkType;
    std::vector<API::TupleValues> linkedPrimaryKeys;
  };

  API::IID::Type m_tableId;
  bool m_bIsReturningInsertedIds;
  std::map<API::IID::Type, LinkedTuples> m_linkedTuplesMap;

  void addUpdateForeignKeyColumnsToInsertIntoQuery(API::ISchema& schema, API::IID::Type relationshipId,
                                                   const API::Relationship& relationship,
                                                   const API::Table& childTable,
                                                   const LinkedTuples& linkedTuples) const;

  void addLinkTuplesQueriesForRelationshipIds(const std::set<API::IID::Type>& relationshipIds);

};

}
