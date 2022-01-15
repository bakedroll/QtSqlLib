#pragma once

#include <QtSqlLib/TableConfigurator.h>

#include <QString>

#include <map>
#include <memory>

namespace QtSqlLib
{

class SchemaConfigurator
{
public:
  struct Schema
  {
    std::map<unsigned int, TableConfigurator::Table> tables;
  };

  SchemaConfigurator(Schema& schema);
  virtual ~SchemaConfigurator();

  TableConfigurator& configureTable(unsigned int tableId, const QString& tableName);

  static QString getVersionTableName();

private:
  Schema& m_schema;

  std::map<unsigned int, std::unique_ptr<TableConfigurator>> m_tableConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
