#pragma once

#include <map>
#include <set>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/SchemaTypes.h>

namespace QtSqlLib::API
{

class ISchema
{
public:
  ISchema() = default;
  virtual ~ISchema() = default;

  virtual std::map<IID::Type, Table>& getTables() = 0;
  virtual std::map<IID::Type, Relationship>& getRelationships() = 0;

  virtual IID::Type getManyToManyLinkTableId(IID::Type relationshipId) const = 0;

  virtual void configureRelationships() = 0;

  virtual void throwIfTableIdNotExisting(IID::Type tableId) const = 0;
  virtual void throwIfRelationshipIsNotExisting(IID::Type relationshipId) const = 0;
  virtual void throwIfColumnIdNotExisting(const Table& table, IID::Type colId) const = 0;

  virtual IID::Type validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const = 0;
  virtual IID::Type validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const = 0;

  virtual std::pair<IID::Type, IID::Type> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const TupleValues& toTupleKeyValues) const = 0;

  virtual std::pair<IID::Type, IID::Type> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const = 0;

};

}
