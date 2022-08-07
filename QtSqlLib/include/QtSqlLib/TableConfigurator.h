#pragma once

#include <QtSqlLib/API/ISchema.h>

#include <optional>

namespace QtSqlLib
{

class TableConfigurator
{
public:
  TableConfigurator(API::ISchema::Table& table);
  virtual ~TableConfigurator();

  TableConfigurator& column(API::ISchema::Id columnId, const QString& columnName, API::ISchema::DataType type, int varcharLength = 64);

  TableConfigurator& primaryKey();
  TableConfigurator& autoIncrement();
  TableConfigurator& notNull();

  TableConfigurator& primaryKeys(API::ISchema::Id columnId);

  template <typename... T>
  TableConfigurator& primaryKeys(API::ISchema::Id columnId, T... args)
  {
    m_bIsConfiguringPrimaryKeys = true;

    primaryKeys(columnId);
    primaryKeys(args...);

    m_bIsConfiguringPrimaryKeys = false;
    m_bIsPrimaryKeysConfigured = true;
    return *this;
  }

private:
  API::ISchema::Table& m_table;
  std::optional<API::ISchema::Id> m_lastColumnId;

  bool m_bIsConfiguringPrimaryKeys;
  bool m_bIsPrimaryKeysConfigured;

  bool isColumnNameExisting(const QString& name) const;
  void checkColumn() const;

};

}