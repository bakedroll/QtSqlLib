#pragma once

#include <QtSqlLib/API/IDatabase.h>
#include <QtSqlLib/Schema.h>
#include <QtSqlLib/SchemaConfigurator.h>

#include <QSqlDatabase>

namespace QtSqlLib
{

class Database : public API::IDatabase
{
public:
  Database();
  ~Database() override;

  void initialize(const QString& filename) override;
  void close() override;

  API::IQuery::QueryResults execQuery(API::IQueryElement& query) override;

protected:
  virtual void configureSchema(SchemaConfigurator& configurator) = 0;

private:
  QSqlDatabase m_db;

  Schema m_schema;

  bool m_isInitialized;

  void loadDatabaseFile(const QString& filename);
  int  queryDatabaseVersion();
  void createOrMigrateTables(int currentVersion = 1);

  static int getDatabaseVersion();

};

}
