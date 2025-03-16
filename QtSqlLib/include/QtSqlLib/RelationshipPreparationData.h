#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>

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
    API::TupleValues foreignKeyValues;
    API::TupleValues childKeyValues;
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

  void fromOne(const API::TupleValues& tupleKeyValues);
  void fromRemainingKey();

  void toOne(const API::TupleValues& tupleKeyValues);
  void toMany(const std::vector<API::TupleValues>& tupleKeyValuesList);

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

  API::TupleValues m_fromTupleKeyValues;
  std::vector<API::TupleValues> m_toTupleKeyValuesList;

  AffectedData determineAffectedChildTableData(
    API::ISchema& schema, const API::Relationship& relationship,
    API::IID::Type fromTableId, API::IID::Type toTableId);
  AffectedData determineAffectedLinkTableData(
    API::ISchema& schema, API::IID::Type fromTableId, API::IID::Type toTableId);

};

}