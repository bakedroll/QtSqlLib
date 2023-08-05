#pragma once

#include <QtSqlLib/Query/QuerySequence.h>
#include "QtSqlLib/Query/InsertInto.h"

#include <QtSqlLib/API/IID.h>

#include <QVariant>

#include <vector>

namespace QtSqlLib::Query
{

class InsertIntoReferences;

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
