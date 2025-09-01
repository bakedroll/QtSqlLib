#pragma once

#include <QtSqlLib/API/IDatabase.h>

#include <QSqlDatabase>

#include <memory>

namespace QtSqlLib::API
{
class ISchema;
class ISchemaConfigurator;
}

namespace QtSqlLib
{

class Database : public API::IDatabase
{
public:
  Database();
  ~Database() override;

  void initialize(
    API::ISchemaConfigurator& schemaConfigurator, const QString& fileName,
    const QString& databaseName = QSqlDatabase::defaultConnection) override;
  void close() override;

  ResultSet execQuery(API::IQueryElement& query) override;
  ResultSetPrinter createResultSetPrinter(ResultSet& resultSet, int maxColumnWidth) const override;

private:
  std::unique_ptr<QSqlDatabase> m_db;
  std::unique_ptr<API::ISchema> m_schema;

  QString m_databaseName;

  void loadDatabaseFile(const QString& filename);
  int  queryDatabaseVersion();
  void createOrMigrateTables(int currentVersion = 1);

  ResultSet execQueryForSchema(API::ISchema& schema, API::IQueryElement& query) const;

  bool isVersionTableExisting() const;

};

}
