#include "QtSqlLib/Database.h"

#include "QtSqlLib/DatabaseException.h"
#include "QtSqlLib/QueryPrepareVisitor.h"
#include "QtSqlLib/QueryExecuteVisitor.h"
#include "QtSqlLib/Query/FromTable.h"
#include "QtSqlLib/Query/InsertInto.h"
#include "QtSqlLib/Query/Query.h"
#include "QtSqlLib/Query/QuerySequence.h"

#include <utilsLib/Utils.h>

#include <QSqlQuery>
#include <QVariant>

#include <set>

namespace QtSqlLib
{

static const Schema::Id s_sqliteMasterTableId = 0;
static const Schema::Id s_sqliteMasterTypeColId = 0;
static const Schema::Id s_sqliteMasterNameColId = 1;

static const Schema::Id s_versionColId = 0;
static const Schema::Id s_versionTableid = std::numeric_limits<Schema::Id>::max();
static const QString s_versionTableName = "database_version";

static QString getDataTypeName(Schema::DataType type, int varcharLength)
{
  switch (type)
  {
  case Schema::DataType::Integer:
    return "INTEGER";
  case Schema::DataType::Real:
    return "REAL";
  case Schema::DataType::Varchar:
    return QString("VARCHAR(%1)").arg(varcharLength);
  case Schema::DataType::Blob:
    return "BLOB";
  default:
    throw DatabaseException(DatabaseException::Type::UnableToLoad, "Unknown data type.");
  }
}

static QString getActionString(Schema::ForeignKeyAction action)
{
  switch (action)
  {
  case Schema::ForeignKeyAction::NoAction:
    return "NO ACTION";
  case Schema::ForeignKeyAction::Restrict:
    return "RESTRICT";
  case Schema::ForeignKeyAction::SetNull:
    return "SET NULL";
  case Schema::ForeignKeyAction::SetDefault:
    return "SET DEFAULT";
  case Schema::ForeignKeyAction::Cascade:
    return "CASCADE";
  default:
    break;
  }

  return "";
}

static API::IQuery::QueryResults execQueryForSchema(Schema& schema, API::IQueryElement& query)
{
  QueryPrepareVisitor prepateVisitor(schema);
  query.accept(prepateVisitor);

  QueryExecuteVisitor executeVisitor(schema);

  QSqlDatabase::database().transaction();

  try
  {
    query.accept(executeVisitor);
  }
  catch (DatabaseException& e)
  {
    QSqlDatabase::database().rollback();
    throw e;
  }

  QSqlDatabase::database().commit();

  return executeVisitor.getLastQueryResults();
}

class CreateTable : public Query::Query
{
public:
  CreateTable(const Schema::Table& table)
    : Query()
    , m_table(table)
  {}

  ~CreateTable() override = default;

  API::IQuery::SqlQuery getSqlQuery(Schema& schema, QueryResults& previousQueryResults) override
  {
    const auto cutTailingComma = [](QString& str)
    {
      str = str.left(str.length() - 2);
    };

    const auto bIsSinglePrimaryKey = (m_table.primaryKeys.size() == 1);

    QString columns;
    for (const auto& column : m_table.columns)
    {
      columns += QString("'%1' %2")
        .arg(column.second.name)
        .arg(getDataTypeName(column.second.type, column.second.varcharLength));

      if (bIsSinglePrimaryKey && (m_table.primaryKeys.count(column.first) > 0))
      {
        columns += " PRIMARY KEY";
      }
      if (column.second.bIsAutoIncrement)
      {
        columns += " AUTOINCREMENT";
      }
      if (column.second.bIsNotNull)
      {
        columns += " NOT NULL";
      }
      columns.append(", ");
    }

    if (m_table.primaryKeys.size() > 1)
    {
      QString primaryKeyNames;
      for (const auto& primaryKeyId : m_table.primaryKeys)
      {
        primaryKeyNames += QString("'%1', ").arg(m_table.columns.at(primaryKeyId).name);
      }
      cutTailingComma(primaryKeyNames);
      columns += QString("PRIMARY KEY(%1), ").arg(primaryKeyNames);
    }

    if (!m_table.relationshipToForeignKeyReferencesMap.empty())
    {
      for (const auto& foreignKeyReferences : m_table.relationshipToForeignKeyReferencesMap)
      {
        QString foreignKeyColNames;
        QString parentKeyColNames;

        const auto& parentTable = schema.getTables().at(foreignKeyReferences.second.referenceTableId);

        for (const auto& refKeyColumn : foreignKeyReferences.second.primaryForeignKeyColIdMap)
        {
          foreignKeyColNames += QString("%1, ").arg(m_table.columns.at(refKeyColumn.second).name);
          parentKeyColNames += QString("%1, ").arg(parentTable.columns.at(refKeyColumn.first.columnId).name);
        }
        cutTailingComma(foreignKeyColNames);
        cutTailingComma(parentKeyColNames);

        QString onDeleteStr;
        if (foreignKeyReferences.second.onDeleteAction != Schema::ForeignKeyAction::NoAction)
        {
          onDeleteStr = QString(" ON DELETE %1").arg(getActionString(foreignKeyReferences.second.onDeleteAction));
        }

        QString onUpdateStr;
        if (foreignKeyReferences.second.onUpdateAction != Schema::ForeignKeyAction::NoAction)
        {
          onUpdateStr = QString(" ON UPDATE %1").arg(getActionString(foreignKeyReferences.second.onUpdateAction));
        }

        columns += QString("FOREIGN KEY (%1) REFERENCES '%2'(%3)%4%5, ")
          .arg(foreignKeyColNames).arg(parentTable.name).arg(parentKeyColNames)
          .arg(onDeleteStr).arg(onUpdateStr);
      }
    }

    if (columns.right(2) == ", ")
    {
      cutTailingComma(columns);
    }
    columns = columns.simplified();

    const auto queryString = QString("CREATE TABLE '%1' (%2);").arg(m_table.name).arg(columns);
    UTILS_LOG_DEBUG(queryString.toStdString().c_str());

    QSqlQuery query;
    query.prepare(queryString);

    return { query };
  }

private:
  const Schema::Table& m_table;

};

Database::Database()
  : m_isInitialized(false)
{
}

Database::~Database()
{
  if (m_db.isOpen())
  {
    m_db.close();
  }
}

void Database::initialize(const QString& filename)
{
  if (m_isInitialized)
  {
    UTILS_LOG_WARN("Database is already initialized.");
    return;
  }

  SchemaConfigurator configurator(m_schema);
  configurator.configureTable(s_versionTableid, s_versionTableName)
    .column(s_versionColId, "version", Schema::DataType::Integer).primaryKey().notNull();

  configureSchema(configurator);
  m_schema.configureRelationships();

  loadDatabaseFile(filename);

  m_isInitialized = true;
}

void Database::close()
{
  m_db.close();
}

API::IQuery::QueryResults Database::execQuery(API::IQueryElement& query)
{
  return execQueryForSchema(m_schema, query);
}

void Database::loadDatabaseFile(const QString& filename)
{
  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(filename);

  if (!m_db.open())
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

    const auto targetVersion = getDatabaseVersion();
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
  const auto results = execQuery(Query::FromTable(s_versionTableid).select(s_versionColId));
  if (results.values.empty())
  {
    return -1;
  }

  return results.values[0].at({ s_versionTableid, s_versionColId }).toInt();
}

void Database::createOrMigrateTables(int currentVersion)
{
  const auto targetVersion = getDatabaseVersion();
  for (auto version = currentVersion+1; version <= targetVersion; version++)
  {
    if (version == 1)
    {
      Query::QuerySequence sequence;
      for (const auto& table : m_schema.getTables())
      {
        sequence.addQuery(std::make_unique<CreateTable>(table.second));
      }

      auto query = std::make_unique<Query::InsertInto>(s_versionTableid);
      query->value(s_versionColId, targetVersion);

      sequence.addQuery(std::move(query));

      execQuery(sequence);
    }
  }
}

int Database::getDatabaseVersion()
{
  return 1;
}

bool Database::isVersionTableExisting() const
{
  Schema sqliteMasterSchema;

  auto& table = sqliteMasterSchema.getTables()[s_sqliteMasterTableId];
  table.name = "sqlite_master";
  table.columns[s_sqliteMasterTypeColId].name = "type";
  table.columns[s_sqliteMasterNameColId].name = "name";

  const auto results = execQueryForSchema(sqliteMasterSchema,
    Query::FromTable(s_sqliteMasterTableId)
      .select(s_sqliteMasterNameColId)
      .where(Expr()
        .equal(s_sqliteMasterTypeColId, "table")
        .and()
        .equal(s_sqliteMasterNameColId, s_versionTableName)));

  return !results.values.empty();
}

}
