#include "QtSqlLib/Database.h"

#include "QtSqlLib/API/ISchemaConfigurator.h"
#include "QtSqlLib/API/ITableConfigurator.h"
#include "QtSqlLib/ColumnID.h"
#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/Expr.h"
#include "QtSqlLib/ID.h"
#include "QtSqlLib/Query/FromTable.h"
#include "QtSqlLib/Query/InsertInto.h"
#include "QtSqlLib/Query/QuerySequence.h"
#include "QtSqlLib/QueryExecuteVisitor.h"
#include "QtSqlLib/QueryPrepareVisitor.h"
#include "QtSqlLib/Schema.h"

#include "CreateIndex.h"
#include "CreateTable.h"
#include "SanityChecker.h"

#include <QVariant>

#include <set>

namespace QtSqlLib
{

static const int s_defaultSchemaTargetVersion = 1;

static const API::IID::Type s_sqliteMasterTableId = 0;
static const API::IID::Type s_sqliteMasterTypeColId = 0;
static const API::IID::Type s_sqliteMasterNameColId = 1;

static const API::IID::Type s_versionColId = 0;
static const API::IID::Type s_versionTableid = std::numeric_limits<API::IID::Type>::max();
static const QString s_versionTableName = "database_version";

static void verifyPrimaryKeys(const API::Table& table)
{
  for (const auto& key : table.primaryKeys)
  {
    auto found = false;
    for (const auto& column : table.columns)
    {
      if (column.first == key)
      {
        found = true;
        break;
      }
    }

    if (!found)
    {
      throw DatabaseException(DatabaseException::Type::InvalidId,
        QString("Invalid primary key specified for table '%1'").arg(table.name));
    }
  }
}

Database::Database() = default;

Database::~Database()
{
  Database::close();
}

void Database::initialize(API::ISchemaConfigurator& schemaConfigurator, const QString& fileName,
                          const QString& databaseName)
{
  if (m_db)
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Database is already initialized.");
  }

  m_databaseName = databaseName;

  schemaConfigurator.configureTable(ID(s_versionTableid), s_versionTableName)
    .column(ID(s_versionColId), "version", API::DataType::Integer).primaryKey().notNull();

  m_schema = schemaConfigurator.getSchema();
  m_schema->configureRelationships();
  m_schema->validateAndPrepareIndices();

  loadDatabaseFile(fileName);
}

void Database::close()
{
  if (m_db && m_db->isOpen())
  {
    m_db->close();
    m_db.reset();

    QSqlDatabase::removeDatabase(m_databaseName);
  }
}

ResultSet Database::execQuery(API::IQueryElement& query)
{
  return execQueryForSchema(*m_schema, query);
}

void Database::loadDatabaseFile(const QString& filename)
{
  m_db = std::make_unique<QSqlDatabase>(QSqlDatabase::addDatabase("QSQLITE", m_databaseName));
  m_db->setDatabaseName(filename);

  if (!m_db->open())
  {
    throw DatabaseException(DatabaseException::Type::UnableToLoad,
      QString("Could not load database file: %1.").arg(filename));
  }

  if (!isVersionTableExisting())
  {
    createOrMigrateTables(0);
  }
  else
  {
    const auto currentVersion = queryDatabaseVersion();
    if (currentVersion <= 0)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        "Could not query version.");
    }

    const auto targetVersion = s_defaultSchemaTargetVersion;
    if (currentVersion < targetVersion)
    {
      createOrMigrateTables(currentVersion);
    }
    else if (currentVersion > targetVersion)
    {
      throw DatabaseException(DatabaseException::Type::UnableToLoad,
        "DB version newer than expected.");
    }
  }
}

int Database::queryDatabaseVersion()
{
  const auto results = execQuery(Query::FromTable(ID(s_versionTableid)).select({ s_versionColId }));
  if (!results.hasNext())
  {
    return -1;
  }

  return results.next().at({ s_versionTableid, s_versionColId }).toInt();
}

void Database::createOrMigrateTables(int currentVersion)
{
  const auto targetVersion = s_defaultSchemaTargetVersion;
  for (auto version = currentVersion+1; version <= targetVersion; version++)
  {
    if (version == 1)
    {
      Query::QuerySequence sequence;
      for (const auto& table : m_schema->getTables())
      {
        verifyPrimaryKeys(table.second);
        sequence.addQuery(std::make_unique<Query::CreateTable>(table.second));
      }

      for (const auto& index : m_schema->getIndices())
      {
        sequence.addQuery(std::make_unique<Query::CreateIndex>(index));
      }

      auto query = std::make_unique<Query::InsertInto>(ID(s_versionTableid));
      query->value(ID(s_versionColId), targetVersion);

      sequence.addQuery(std::move(query));

      execQuery(sequence);
    }
  }
}

ResultSet Database::execQueryForSchema(API::ISchema& schema, API::IQueryElement& query) const
{
  if (!m_db || !m_schema)
  {
    throw DatabaseException(DatabaseException::Type::UnexpectedError, "Database is not yet initialized.");
  }

  QueryPrepareVisitor prepateVisitor(schema);
  query.accept(prepateVisitor);

  QueryExecuteVisitor executeVisitor(*m_db, schema);

  QSqlDatabase::database().transaction();

  try
  {
    query.accept(executeVisitor);
  }
  catch (DatabaseException&)
  {
    QSqlDatabase::database().rollback();
    throw;
  }

  QSqlDatabase::database().commit();

  return executeVisitor.getLastQueryResults();
}

bool Database::isVersionTableExisting() const
{
  Schema sqliteMasterSchema;
  sqliteMasterSchema.setSanityChecker(
    std::make_unique<SanityChecker>(sqliteMasterSchema.getTables(), sqliteMasterSchema.getRelationships()));

  auto& table = sqliteMasterSchema.getTables()[s_sqliteMasterTableId];
  table.name = "sqlite_master";
  table.columns[s_sqliteMasterTypeColId].name = "type";
  table.columns[s_sqliteMasterNameColId].name = "name";

  const auto results = execQueryForSchema(sqliteMasterSchema,
    Query::FromTable(ID(s_sqliteMasterTableId))
    .select({ s_sqliteMasterNameColId })
    .where(Expr()
      .equal(ID(s_sqliteMasterTypeColId), "table")
      .and()
      .equal(ID(s_sqliteMasterNameColId), s_versionTableName)));

  return results.hasNext();
}

}
