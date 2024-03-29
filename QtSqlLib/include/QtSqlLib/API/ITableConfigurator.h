#pragma once

#include <QtSqlLib/API/SchemaTypes.h>

namespace QtSqlLib::API
{

class ITableConfigurator
{
public:
  ITableConfigurator() = default;
  virtual ~ITableConfigurator() = default;

  virtual ITableConfigurator& column(const IID& columnId, const QString& columnName,
                                     API::DataType type, int varcharLength = 64) = 0;

  virtual ITableConfigurator& primaryKey() = 0;
  virtual ITableConfigurator& autoIncrement() = 0;
  virtual ITableConfigurator& notNull() = 0;
  virtual ITableConfigurator& unique() = 0;

  virtual ITableConfigurator& primaryKeys(const std::vector<API::IID::Type>& columnIds) = 0;
  virtual ITableConfigurator& uniqueCols(const std::vector<API::IID::Type>& columnIds) = 0;

};

}
