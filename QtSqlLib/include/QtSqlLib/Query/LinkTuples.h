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

  LinkTuples& fromOne(const Schema::TableColumnValuesMap& rowIds);
  LinkTuples& fromRemainingKey();

  LinkTuples& toOne(const Schema::TableColumnValuesMap& rowIds);
  LinkTuples& toMany(const std::vector<Schema::TableColumnValuesMap>& rowIdsList);

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
      const Schema::PrimaryForeignKeyColIdMap& primaryForeignKeyColIdMap);
    ~UpdateTableForeignKeys() override;

    void setMode(Mode mode);
    void setForeignKeyValues(const Schema::TableColumnValuesMap& parentKeyValues);
    void makeAndAddWhereExpr(const Schema::TableColumnValuesMap& affectedChildKeyValues);

    SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override;

  private:
    Mode m_mode;
    const Schema::PrimaryForeignKeyColIdMap& m_primaryForeignKeyColIdMap;

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
  Schema::TableColumnValuesMap m_fromRowIds;
  std::vector<Schema::TableColumnValuesMap> m_toRowIdsList;

  void prepareToOneLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                 Schema::Id fromTableId, Schema::Id toTableId);
  void prepareToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                  Schema::Id fromTableId, Schema::Id toTableId);

};

}
