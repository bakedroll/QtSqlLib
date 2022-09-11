#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <QVariant>

#include <map>

namespace QtSqlLib
{

class Schema : public API::ISchema
{
public:
  Schema();
  ~Schema() override;

  std::map<API::IID::Type, Table>& getTables() override;
  std::map<API::IID::Type, Relationship>& getRelationships() override;

  API::IID::Type getManyToManyLinkTableId(API::IID::Type relationshipId) const override;

  void configureRelationships() override;

  void throwIfTableIdNotExisting(API::IID::Type tableId) const override;
  void throwIfRelationshipIsNotExisting(API::IID::Type relationshipId) const override;
  void throwIfColumnIdNotExisting(const Table& table, API::IID::Type colId) const override;

  API::IID::Type validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const override;
  API::IID::Type validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const override;

  std::pair<API::IID::Type, API::IID::Type> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    API::IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const TupleValues& toTupleKeyValues) const override;

  std::pair<API::IID::Type, API::IID::Type> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    API::IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const override;

private:
  std::map<API::IID::Type, Table> m_tables;
  std::map<API::IID::Type, Relationship> m_relationships;
  std::map<API::IID::Type, API::IID::Type> m_mapManyToManyRelationshipToLinkTableId;

  std::pair<API::IID::Type, API::IID::Type> verifyRelationshipPrimaryKeysAndGetTableIds(
    bool bIsOneToMany,
    API::IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const;

  static bool isTableIdsMatching(const Relationship& relationship, API::IID::Type tableFromId,
                                 API::IID::Type tableToId, bool bIgnoreFromKeys);

};

}
