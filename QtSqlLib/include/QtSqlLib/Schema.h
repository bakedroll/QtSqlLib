#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <map>

namespace QtSqlLib
{

class Schema : public API::ISchema
{
public:
  explicit Schema();
  ~Schema() override;

  std::map<API::IID::Type, API::Table>& getTables() override;
  std::map<API::IID::Type, API::Relationship>& getRelationships() override;
  std::vector<API::Index>& getIndices() override;

  API::ColumnMetaInfo getColumnMetaInfo(API::IID::Type tableId, API::IID::Type columnId) const override;

  const API::ISanityChecker& getSanityChecker() const override;

  API::IID::Type getManyToManyLinkTableId(API::IID::Type relationshipId) const override;

  void configureRelationships() override;
  void validateAndPrepareIndices() override;

  API::IID::Type validatePrimaryKeysAndGetTableId(const API::TupleValues& tupleKeyValues) const override;
  API::IID::Type validatePrimaryKeysListAndGetTableId(const std::vector<API::TupleValues>& tupleKeyValuesList) const override;

  std::pair<API::IID::Type, API::IID::Type> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    API::IID::Type relationshipId,
    const API::TupleValues& fromTupleKeyValues,
    const API::TupleValues& toTupleKeyValues) const override;

  std::pair<API::IID::Type, API::IID::Type> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    API::IID::Type relationshipId,
    const API::TupleValues& fromTupleKeyValues,
    const std::vector<API::TupleValues>& toTupleKeyValuesList) const override;

  void setSanityChecker(std::unique_ptr<API::ISanityChecker> sanityChecker);

private:
  std::unique_ptr<API::ISanityChecker> m_sanityChecker;

  std::map<API::IID::Type, API::Table> m_tables;
  std::map<API::IID::Type, API::Relationship> m_relationships;
  std::vector<API::Index> m_indices;

  std::map<API::IID::Type, API::IID::Type> m_mapManyToManyRelationshipToLinkTableId;

  std::pair<API::IID::Type, API::IID::Type> verifyRelationshipPrimaryKeysAndGetTableIds(
    bool bIsOneToMany,
    API::IID::Type relationshipId,
    const API::TupleValues& fromTupleKeyValues,
    const std::vector<API::TupleValues>& toTupleKeyValuesList) const;

  static bool isTableIdsMatching(const API::Relationship& relationship, API::IID::Type tableFromId,
                                 API::IID::Type tableToId, bool bIgnoreFromKeys);

};

}
