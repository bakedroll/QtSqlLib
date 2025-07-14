#pragma once

#include <QtSqlLib/API/ITableConfigurator.h>

#include <QtSqlLib/API/SchemaTypes.h>

#include <optional>

namespace QtSqlLib
{

class TableConfigurator : public API::ITableConfigurator
{
public:
  TableConfigurator(API::Table& table);
  ~TableConfigurator() override;

  ITableConfigurator& column(const API::IID& columnId, const QString& columnName,
                             API::DataType type, int varcharLength) override;

  ITableConfigurator& primaryKey() override;
  ITableConfigurator& autoIncrement() override;
  ITableConfigurator& notNull() override;
  ITableConfigurator& unique() override;

  ITableConfigurator& primaryKeys(const ColumnHelper::SelectColumnList& columns) override;
  ITableConfigurator& uniqueCols(const ColumnHelper::SelectColumnList& columns) override;

private:
  API::Table& m_table;
  std::optional<API::IID::Type> m_lastColumnId;

  bool m_bIsPrimaryKeysConfigured;

  bool isColumnNameExisting(const QString& name) const;
  void checkColumn() const;

};

}