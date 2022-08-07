#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <set>

namespace QtSqlLib::API
{

class ITableConfigurator
{
public:
  ITableConfigurator() = default;
  virtual ~ITableConfigurator() = default;

  virtual ITableConfigurator& column(API::ISchema::Id columnId, const QString& columnName,
                                     ISchema::DataType type, int varcharLength = 64) = 0;

  virtual ITableConfigurator& primaryKey() = 0;
  virtual ITableConfigurator& autoIncrement() = 0;
  virtual ITableConfigurator& notNull() = 0;

  virtual ITableConfigurator& primaryKeys(const std::set<ISchema::Id>& columnIds) = 0;

};

}
