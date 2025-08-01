#pragma once

#include <QString>
#include <QVariant>

#include <optional>
#include <vector>

namespace QtSqlLib::API
{
class IID;
class IQueryIdentifiers;
class ISchema;
}

namespace QtSqlLib
{

class ITermElement
{
public:
  virtual ~ITermElement() = default;

  virtual QString toQueryString(
    API::ISchema& schema,
    const API::IQueryIdentifiers& queryIdentifiers,
    std::vector<QVariant>& boundValuesOut) const = 0;

};

}
