#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

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

  virtual ColumnMetaInfo getColumnMetaInfo(API::IID::Type tableId, API::IID::Type columnId) const = 0;

  virtual const ISanityChecker& getSanityChecker() const = 0;

  virtual IID::Type getManyToManyLinkTableId(IID::Type relationshipId) const = 0;

  virtual void configureRelationships() = 0;
  virtual void validateAndPrepareIndices() = 0;

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
