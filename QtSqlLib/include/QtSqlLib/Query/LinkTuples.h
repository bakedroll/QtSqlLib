#pragma once

#include <QtSqlLib/Query/QuerySequence.h>

#include <QtSqlLib/Schema.h>

namespace QtSqlLib::Query
{

class LinkTuples : public QuerySequence
{
public:
  LinkTuples(Schema::Id relationshipId);
  ~LinkTuples() override;

  LinkTuples& fromOne(const Schema::TableColumnValuesMap& rowIds);
  LinkTuples& toOne(const Schema::TableColumnValuesMap& rowIds);
  LinkTuples& toMany(const std::vector<Schema::TableColumnValuesMap>& rowIdsList);

  void prepare(Schema& schema) override;

private:
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
  Schema::TableColumnValuesMap m_fromRowIds;
  std::vector<Schema::TableColumnValuesMap> m_toRowIdsList;

  void prepareOneToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                 Schema::Id fromTableId, Schema::Id toTableId);
  void prepareManyToManyLinkQuery(Schema& schema, const Schema::Relationship& relationship,
                                  Schema::Id fromTableId, Schema::Id toTableId);

};

}
