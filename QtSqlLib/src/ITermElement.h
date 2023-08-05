#pragma once

#include <QString>

#include <optional>

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
  virtual QString toQString(API::ISchema& schema, const OptionalIID& defaultTableId = std::nullopt) const = 0;

};

}
