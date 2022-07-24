#pragma once

#include <QtSqlLib/Query/QuerySequence.h>
#include "QtSqlLib/Query/UpdateTable.h"

#include <QtSqlLib/Schema.h>

namespace QtSqlLib::Query
{

class LinkTuples : public QuerySequence
{
public:
  LinkTuples(Schema::Id relationshipId);
  ~LinkTuples() override;

  LinkTuples& fromOne(const Schema::TupleValues& tupleKeyValues);
  LinkTuples& fromRemainingKey();

  LinkTuples& toOne(const Schema::TupleValues& tupleKeyValues);
  LinkTuples& toMany(const std::vector<Schema::TupleValues>& tupleKeyValuesList);

  void prepare(Schema& schema) override;

private:
  class UpdateTableForeignKeys : public UpdateTable
  {
  public:
    enum class Mode
    {
      Default,
      AffectedChildKeyValuesRemaining,
      ForeignKeyValuesRemaining
    };

    UpdateTableForeignKeys(
      Schema::Id tableId,
      const Schema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap);
    ~UpdateTableForeignKeys() override;

    void setMode(Mode mode);
    void setForeignKeyValues(const Schema::TupleValues& parentKeyValues);
    void makeAndAddWhereExpr(const Schema::TupleValues& affectedChildKeyValues);

    SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;

  private:
    Mode m_mode;
    const Schema::PrimaryForeignKeyColumnIdMap& m_primaryForeignKeyColIdMap;

  };

  enum class ExpectedCall
  {
    From,
    To,
    Complete
  };

  enum class RelationshipType
  {
    ToOne,
    ToMany
  };

  Schema::Id m_relationshipId;
  ExpectedCall m_expectedCall;

  RelationshipType m_type;

  bool m_bRemainingFromKeys;
  Schema::TupleValues m_fromTupleKeyValues;
  std::vector<Schema::TupleValues> m_toTupleKeyValuesList;

  void prepareToOneLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                 Schema::Id fromTableId, Schema::Id toTableId);
  void prepareToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                  Schema::Id fromTableId, Schema::Id toTableId);

};

}
