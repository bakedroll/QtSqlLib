#pragma once

#include <QtSqlLib/IDatabase.h>
#include <QtSqlLib/SchemaConfigurator.h>

#include <QSqlDatabase>

namespace QtSqlLib
{

class Database : public IDatabase
{
public:
  Database();
  ~Database() override;

  void initialize(const QString& filename) override;
  void close() override;

  void execQuery(const IQuery& query) const override;

protected:
  virtual void configureSchema(SchemaConfigurator& configurator) = 0;

private:
  QSqlDatabase m_db;

  SchemaConfigurator::Schema m_schema;

  bool m_isInitialized;

  void loadDatabaseFile(const QString& filename);
  int  queryDatabaseVersion() const;
  void createOrMigrateTables(int currentVersion = 1) const;

  static int getDatabaseVersion();

};

}