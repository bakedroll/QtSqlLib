#pragma once

#include <QtSqlLib/API/IDatabase.h>
#include <QtSqlLib/Schema.h>
#include <QtSqlLib/SchemaConfigurator.h>

#include <QSqlDatabase>

#include <memory>

namespace QtSqlLib
{

class Database : public API::IDatabase
{
public:
  Database();
  ~Database() override;

  void initialize(const QString& fileName, const QString& databaseName = QSqlDatabase::defaultConnection) override;
  void close() override;

  API::IQuery::QueryResults execQuery(API::IQueryElement& query) override;

protected:
  virtual void configureSchema(SchemaConfigurator& configurator) = 0;

private:
  std::unique_ptr<QSqlDatabase> m_db;

  Schema m_schema;

  bool m_isInitialized;
  QString m_databaseName;

  void loadDatabaseFile(const QString& filename);
  int  queryDatabaseVersion();
  void createOrMigrateTables(int currentVersion = 1);

  API::IQuery::QueryResults execQueryForSchema(API::ISchema& schema, API::IQueryElement& query) const;

  bool isVersionTableExisting() const;

};

}
