#pragma once

#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/PrimaryKey.h>

#include <map>
#include <set>

namespace QtSqlLib::API
{

class ISanityChecker;

class ISchema
{
public:
  ISchema() = default;
  virtual ~ISchema() = default;

  virtual std::map<IID::Type, Table>& getTables() = 0;
  virtual std::map<IID::Type, Relationship>& getRelationships() = 0;
  virtual std::vector<Index>& getIndices() = 0;

  virtual const ISanityChecker& getSanityChecker() const = 0;

  virtual IID::Type getManyToManyLinkTableId(IID::Type relationshipId) const = 0;

  virtual void configureRelationships() = 0;
  virtual void validateAndPrepareIndices() = 0;

  virtual void validatePrimaryKeys(const PrimaryKey& tupleKeyValues) const = 0;
  virtual void validatePrimaryKeysList(const std::vector<PrimaryKey>& tupleKeyValuesList) const = 0;

  virtual std::pair<IID::Type, IID::Type> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    IID::Type relationshipId,
    const PrimaryKey& fromTupleKeyValues,
    const PrimaryKey& toTupleKeyValues) const = 0;

  virtual std::pair<IID::Type, IID::Type> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    IID::Type relationshipId,
    const PrimaryKey& fromTupleKeyValues,
    const std::vector<PrimaryKey>& toTupleKeyValuesList) const = 0;

};

}
