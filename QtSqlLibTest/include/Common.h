#pragma once

#include <TestDatabase.h>

#include <QtSqlLib/API/IID.h>
#include <QtSqlLib/API/IQueryVisitor.h>
#include <QtSqlLib/DatabaseException.h>
#include <QtSqlLib/Expr.h>
#include <QtSqlLib/ID.h>
#include <QtSqlLib/IDList.h>
#include <QtSqlLib/Query/BatchInsertInto.h>
#include <QtSqlLib/Query/FromTable.h>
#include <QtSqlLib/Query/InsertInto.h>
#include <QtSqlLib/Query/InsertIntoExt.h>
#include <QtSqlLib/Query/LinkTuples.h>
#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include <QtSqlLib/Query/UpdateTable.h>
#include <QtSqlLib/Schema.h>
#include <QtSqlLib/SchemaConfigurator.h>

namespace QtSqlLibTest
{

using BatchInsertInto = QtSqlLib::Query::BatchInsertInto;
using DatabaseException = QtSqlLib::DatabaseException;
using DataType = QtSqlLib::API::ISchema::DataType;
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

  static bool isResultTuplesContaining(
    const std::vector<IQuery::ResultTuple>& results,
    const IID& tableId, const IID& columnId, QVariant value);

  static bool isResultTuplesContaining(
    const IQuery::TupleValuesList& results,
    const IID& tableId, const IID& columnId, QVariant value);

  static IQuery::TupleValuesList& getJoinedTuples(std::vector<IQuery::ResultTuple>& results,
    const IID& tableId, const IID& columnId, const QVariant& value, const IID& relationshipId);

  static void expectRelations(std::vector<IQuery::ResultTuple>& results,
    const IID& relationshipId,
    const IID& fromTableId, const IID& fromColId, const IID& toTableId, const IID& toColId,
    const QVariant& fromValue, const QVariantList& toValues);

private:
  static IQuery::TupleValuesList s_nullTupleValuesList;

};

}
