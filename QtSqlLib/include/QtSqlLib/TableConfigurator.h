#pragma once

#include <QString>

#include <map>

namespace QtSqlLib
{

class TableConfigurator
{
public:
  enum class DataType
  {
    Integer,
    Real,
    Varchar,
    Blob
  };

  struct Column
  {
    QString name;
    DataType type = DataType::Integer;
    int varcharLength = 0;

    bool bIsPrimaryKey = false;
    bool bIsAutoIncrement = false;
    bool bIsNotNull = false;
  };

  struct Table
  {
    QString name;

    std::map<unsigned int, Column> columns;
  };

  TableConfigurator(Table& table);
  virtual ~TableConfigurator();

  TableConfigurator& column(unsigned int columnId, const QString& columnName, DataType type, int varcharLength = 64);

  TableConfigurator& primaryKey();
  TableConfigurator& autoIncrement();
  TableConfigurator& notNull();

private:
  Table& m_table;
  int m_lastColumn;

  bool isColumnNameExisting(const QString& name) const;
  void checkColumn() const;

};

}