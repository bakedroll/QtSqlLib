#pragma once

#include <QtSqlLib/API/SchemaTypes.h>
#include <QtSqlLib/ColumnHelper.h>

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

  virtual ITableConfigurator& primaryKeys(const ColumnHelper::SelectColumnList& columns) = 0;
  virtual ITableConfigurator& uniqueCols(const ColumnHelper::SelectColumnList& columns) = 0;

};

}
