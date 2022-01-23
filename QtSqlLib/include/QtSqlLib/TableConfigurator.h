#pragma once

#include <QtSqlLib/Schema.h>

#include <QString>

#include <map>

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

private:
  Schema::Table& m_table;
  int m_lastColumn;

  bool isColumnNameExisting(const QString& name) const;
  void checkColumn() const;

};

}