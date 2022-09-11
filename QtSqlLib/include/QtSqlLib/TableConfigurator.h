#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/ITableConfigurator.h>

#include <optional>

namespace QtSqlLib
{

class TableConfigurator : public API::ITableConfigurator
{
public:
  TableConfigurator(API::ISchema::Table& table);
  virtual ~TableConfigurator();

  ITableConfigurator& column(const API::IID& columnId, const QString& columnName,
                             API::ISchema::DataType type, int varcharLength) override;

  ITableConfigurator& primaryKey() override;
  ITableConfigurator& autoIncrement() override;
  ITableConfigurator& notNull() override;

  ITableConfigurator& primaryKeys(const std::vector<API::IID::Type>& columnIds) override;

private:
  API::ISchema::Table& m_table;
  std::optional<API::IID::Type> m_lastColumnId;

  bool m_bIsPrimaryKeysConfigured;

  bool isColumnNameExisting(const QString& name) const;
  void checkColumn() const;

};

}