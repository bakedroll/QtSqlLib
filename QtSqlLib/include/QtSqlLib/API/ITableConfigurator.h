#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/ISchema.h>

#define COLUMN(X, Y, Z) column(QtSqlLib::ID(X), Y, Z)
#define COLUMN_VARCHAR(X, Y, Z) column(QtSqlLib::ID(X), Y, QtSqlLib::API::DataType::Varchar, Z)

#define PRIMARY_KEY primaryKey()
#define AUTO_INCREMENT autoIncrement()
#define NOT_NULL notNull()

#define PRIMARY_KEYS(X) primaryKeys(X)

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

  virtual ITableConfigurator& primaryKeys(const std::vector<API::IID::Type>& columnIds) = 0;

};

}
