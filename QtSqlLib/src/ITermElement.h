#pragma once

#include <QString>
#include <QVariant>

#include <optional>
#include <vector>

namespace QtSqlLib::API
{
class IID;
class ISchema;
}

namespace QtSqlLib
{

class ITermElement
{
public:
  using OptionalIID = std::optional<std::reference_wrapper<const API::IID>>;

  ITermElement() = default;
  virtual ~ITermElement() = default;
  virtual QString toQueryString(
    API::ISchema& schema,
    std::vector<QVariant>& boundValuesOut,
    const OptionalIID& defaultTableId = std::nullopt) const = 0;

};

}
