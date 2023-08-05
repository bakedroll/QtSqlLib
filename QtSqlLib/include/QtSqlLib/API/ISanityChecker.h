#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

namespace QtSqlLib::API
{

class ISanityChecker
{
public:
  virtual ~ISanityChecker() = default;

  virtual void throwIfTableIdNotExisting(IID::Type tableId) const = 0;
  virtual void throwIfRelationshipIsNotExisting(IID::Type relationshipId) const = 0;
  virtual void throwIfColumnIdNotExisting(const Table& table, IID::Type colId) const = 0;

};

}
