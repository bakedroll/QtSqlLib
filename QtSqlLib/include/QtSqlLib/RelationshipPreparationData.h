#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/PrimaryKey.h>

namespace QtSqlLib::API
{
class ISchema;
}

namespace QtSqlLib
{

class RelationshipPreparationData
{
public:
  enum class RemainingKeysMode
  {
    NoRemainingKeys,
    RemainingPrimaryKeys,
    RemainingForeignKeys
  };

  struct AffectedTuple
  {
    PrimaryKey foreignKeyValues;
    PrimaryKey childKeyValues;
  };

  struct AffectedData
  {
    API::IID::Type tableId = -1;
    bool isLinkTable = false;
    RemainingKeysMode remainingKeysMode = RemainingKeysMode::NoRemainingKeys;
    const API::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap;
    std::vector<AffectedTuple> affectedTuples;
  };

  explicit RelationshipPreparationData(const API::IID& relationshipId);
  virtual ~RelationshipPreparationData();

  void fromOne(const PrimaryKey& tupleKeyValues);
  void fromRemainingKey();

  void toOne(const PrimaryKey& tupleKeyValues);
  void toMany(const std::vector<PrimaryKey>& tupleKeyValuesList);

  AffectedData resolveAffectedTableData(API::ISchema& schema);

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

  API::IID::Type m_relationshipId;
  RelationshipType m_type;
  ExpectedCall m_expectedCall;
  bool m_bRemainingFromKeys;

  PrimaryKey m_fromTupleKeyValues;
  std::vector<PrimaryKey> m_toTupleKeyValuesList;

  AffectedData determineAffectedChildTableData(
    API::ISchema& schema, const API::Relationship& relationship,
    API::IID::Type fromTableId, API::IID::Type toTableId);
  AffectedData determineAffectedLinkTableData(
    API::ISchema& schema, API::IID::Type fromTableId, API::IID::Type toTableId);

};

}