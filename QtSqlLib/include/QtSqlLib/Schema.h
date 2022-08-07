#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <QVariant>

#include <map>
#include <set>

namespace QtSqlLib
{

class Schema : public API::ISchema
{
public:
  Schema();
  ~Schema() override;

  std::map<Id, Table>& getTables() override;
  std::map<Id, Relationship>& getRelationships() override;

  Id getManyToManyLinkTableId(Id relationshipId) const override;

  void configureRelationships() override;

  void throwIfTableIdNotExisting(Id tableId) const override;
  void throwIfRelationshipIsNotExisting(Id relationshipId) const override;
  void throwIfColumnIdNotExisting(const Table& table, Id colId) const override;

  Id validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const override;
  Id validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const override;

  std::pair<Id, Id> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const TupleValues& toTupleKeyValues) const override;

  std::pair<Id, Id> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const override;

private:
  std::map<Id, Table> m_tables;
  std::map<Id, Relationship> m_relationships;
  std::map<Id, Id> m_mapManyToManyRelationshipToLinkTableId;

  std::pair<Id, Id> verifyRelationshipPrimaryKeysAndGetTableIds(
    bool bIsOneToMany,
    Id relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const;

  static bool isTableIdsMatching(const Relationship& relationship, Id tableFromId,
                                 Id tableToId, bool bIgnoreFromKeys);

};

}
