#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/ISchema.h>

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
    API::ISchema::TupleValues foreignKeyValues;
    API::ISchema::TupleValues childKeyValues;
  };

  struct AffectedData
  {
    API::IID::Type tableId = -1;
    bool isLinkTable = false;
    RemainingKeysMode remainingKeysMode = RemainingKeysMode::NoRemainingKeys;
    const API::ISchema::PrimaryForeignKeyColumnIdMap& primaryForeignKeyColIdMap;
    std::vector<AffectedTuple> affectedTuples;
  };

  explicit RelationshipPreparationData(const API::IID& relationshipId);
  virtual ~RelationshipPreparationData();

  void fromOne(const API::ISchema::TupleValues& tupleKeyValues);
  void fromRemainingKey();

  void toOne(const API::ISchema::TupleValues& tupleKeyValues);
  void toMany(const std::vector<API::ISchema::TupleValues>& tupleKeyValuesList);

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

  API::ISchema::TupleValues m_fromTupleKeyValues;
  std::vector<API::ISchema::TupleValues> m_toTupleKeyValuesList;

  AffectedData determineAffectedChildTableData(
    API::ISchema& schema, const API::ISchema::Relationship& relationship,
    API::IID::Type fromTableId, API::IID::Type toTableId);
  AffectedData determineAffectedLinkTableData(
    API::ISchema& schema, const API::ISchema::Relationship& relationship,
    API::IID::Type fromTableId, API::IID::Type toTableId);

};

}