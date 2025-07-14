#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/ISchema.h>
#include <QtSqlLib/ColumnHelper.h>

#include <QString>

#include <optional>

namespace QtSqlLib::API
{

class IQueryIdentifiers
{
public:
  virtual ~IQueryIdentifiers() = default;

  virtual void addTableIdentifier(
    const std::optional<IID::Type>& relationshipId,
    IID::Type tableId,
    const std::optional<QString>& tableAlias = std::nullopt) = 0;

  virtual QString resolveColumnIdentifier(ISchema& schema, const ColumnHelper::ColumnData& columnData) const = 0;
  virtual QString resolveTableIdentifier(ISchema& schema, const std::optional<IID::Type>& relationshipId = std::nullopt) const = 0;
  virtual IID::Type tableId(const std::optional<IID::Type>& relationshipId = std::nullopt) const = 0;

};

}
