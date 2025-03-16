#pragma once

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/IIndexConfigurator.h>
#include <QtSqlLib/API/IQueryVisitor.h>
#include <QtSqlLib/API/IRelationshipConfigurator.h>
#include <QtSqlLib/API/ITableConfigurator.h>
#include <QtSqlLib/ColumnID.h>
#include <QtSqlLib/ColumnList.h>
#include <QtSqlLib/Database.h>
#include <QtSqlLib/DatabaseException.h>
#include <QtSqlLib/Expr.h>
#include <QtSqlLib/ID.h>
#include <QtSqlLib/Query/BatchInsertInto.h>
#include <QtSqlLib/Query/DeleteFrom.h>
#include <QtSqlLib/Query/FromTable.h>
#include <QtSqlLib/Query/InsertInto.h>
#include <QtSqlLib/Query/InsertIntoExt.h>
#include <QtSqlLib/Query/LinkTuples.h>
#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include <QtSqlLib/Query/UnlinkTuples.h>
#include <QtSqlLib/Query/UpdateTable.h>
#include <QtSqlLib/ResultSet.h>
#include <QtSqlLib/Schema.h>
#include <QtSqlLib/SchemaConfigurator.h>

#include <QtSqlLib/Macros.h>

namespace QtSqlLibTest
{

using BatchInsertInto = QtSqlLib::Query::BatchInsertInto;
using DatabaseException = QtSqlLib::DatabaseException;
using DataType = QtSqlLib::API::DataType;
using DeleteFrom = QtSqlLib::Query::DeleteFrom;
using Expr = QtSqlLib::Expr;
using FromTable = QtSqlLib::Query::FromTable;
using IID = QtSqlLib::API::IID;
using InsertInto = QtSqlLib::Query::InsertInto;
using InsertIntoExt = QtSqlLib::Query::InsertIntoExt;
using IQuery = QtSqlLib::API::IQuery;
using IQuerySequence = QtSqlLib::API::IQuerySequence;
using IQueryVisitor = QtSqlLib::API::IQueryVisitor;
using ISchema = QtSqlLib::API::ISchema;
using LinkTuples = QtSqlLib::Query::LinkTuples;
using Query = QtSqlLib::Query::Query;
using QuerySequence = QtSqlLib::Query::QuerySequence;
using Schema = QtSqlLib::Schema;
using SchemaConfigurator = QtSqlLib::SchemaConfigurator;
using UnlinkTuples = QtSqlLib::Query::UnlinkTuples;
using UpdateTable = QtSqlLib::Query::UpdateTable;

enum class TableIds
{
  Table1,
  Table2,
  Students,
  Professors,
  Lectures
};

enum class Table1Cols
{
  Id,
  Text,
  Number,
  Mandatory
};

enum Table2Cols
{
  Id = 0U,
  Text = 1U,
  Mandatory = 2U
};

enum class StudentsCols
{
  Id = 3U,
  Name = 4U
};

enum class ProfessorsCols
{
  Id = 5U,
  Name = 6U
};

enum class LecturesCols
{
  Id = 7U,
  Topic = 8U
};

enum class Relationships
{
  StudentsConfidant,
  LectureParticipant,
  Lecturer,
  Special1,
  Special2,
  Special3,
  Special4,
  Special5,
  Special6
};

class Funcs
{
public:
  Funcs() = delete;

  static QString getDefaultDatabaseFilename();

  static size_t numResults(QtSqlLib::ResultSet& results);

  static bool isResultTuplesContaining(
    QtSqlLib::ResultSet& results,
    IID::Type tableId, IID::Type columnId, QVariant value);

  static void expectRelations(
    QtSqlLib::ResultSet& results, IID::Type relationshipId,
    IID::Type fromTableId, IID::Type fromColId, IID::Type toTableId, IID::Type toColId,
    const QVariant& fromValue, const QVariantList& toValues);

  template<typename TTableId, typename TColumnId>
  static bool isResultTuplesContaining(
    QtSqlLib::ResultSet& results,
    const TTableId& tableId, const TColumnId& columnId, QVariant value)
  {
    return isResultTuplesContaining(results, QtSqlLib::ID(tableId).get(), QtSqlLib::ID(columnId).get(), value);
  }

  template<typename TRelationshipId, typename TTableId, typename TColumnAId, typename TColumnBId>
  static void expectRelations(
    QtSqlLib::ResultSet& results, const TRelationshipId& relationshipId,
    const TTableId& fromTableId, const TColumnAId& fromColId, const TTableId& toTableId, const TColumnBId& toColId,
    const QVariant& fromValue, const QVariantList& toValues)
  {
    return expectRelations(
      results, QtSqlLib::ID(relationshipId).get(), QtSqlLib::ID(fromTableId).get(), QtSqlLib::ID(fromColId).get(),
      QtSqlLib::ID(toTableId).get(), QtSqlLib::ID(toColId).get(), fromValue, toValues);
  }


};

}
