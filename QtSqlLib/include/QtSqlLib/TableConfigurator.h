#pragma once

#include <QtSqlLib/Schema.h>

#include <QString>

#include <map>
#include <optional>

namespace QtSqlLib
{

class TableConfigurator
{
public:
  TableConfigurator(Schema::Table& table);
  virtual ~TableConfigurator();

  TableConfigurator& column(Schema::Id columnId, const QString& columnName, Schema::DataType type, int varcharLength = 64);

  TableConfigurator& primaryKey();
  TableConfigurator& autoIncrement();
  TableConfigurator& notNull();

  TableConfigurator& primaryKeys(Schema::Id columnId);

  template <typename... T>
  TableConfigurator& primaryKeys(Schema::Id columnId, T... args)
  {
    m_bIsConfiguringPrimaryKeys = true;

    primaryKeys(columnId);
    primaryKeys(args...);

    m_bIsConfiguringPrimaryKeys = false;
    m_bIsPrimaryKeysConfigured = true;
    return *this;
  }

private:
  Schema::Table& m_table;
  std::optional<Schema::Id> m_lastColumnId;

  bool m_bIsConfiguringPrimaryKeys;
  bool m_bIsPrimaryKeysConfigured;

  bool isColumnNameExisting(const QString& name) const;
  void checkColumn() const;

};

}