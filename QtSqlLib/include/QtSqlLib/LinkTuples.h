#pragma once

#include <QtSqlLib/QuerySequence.h>

#include <QtSqlLib/Schema.h>

namespace QtSqlLib
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

  struct LinkedTableIds
  {
    Schema::Id tableFromId;
    Schema::Id tableToId;
  };

  Schema::Id m_relationshipId;
  ExpectedCall m_expectedCall;

  RelationshipType m_type;
  Schema::TableColumnValuesMap m_fromRowIds;
  std::vector<Schema::TableColumnValuesMap> m_toRowIdsList;

  LinkedTableIds validateAndGetLinkedTableIds(Schema& schema);
  static bool isTableIdsMatching(const Schema::Relationship& relationship, const LinkedTableIds& linkedTableIds);

};

}
