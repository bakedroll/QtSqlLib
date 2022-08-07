#pragma once

#include <DatabaseDummy.h>

#include <QtSqlLib/API/IQueryVisitor.h>
#include <QtSqlLib/DatabaseException.h>
#include <QtSqlLib/Expr.h>
#include <QtSqlLib/Query/BatchInsertInto.h>
#include <QtSqlLib/Query/FromTable.h>
#include <QtSqlLib/Query/InsertInto.h>
#include <QtSqlLib/Query/InsertIntoExt.h>
#include <QtSqlLib/Query/LinkTuples.h>
#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Query/QuerySequence.h>
#include <QtSqlLib/Query/UpdateTable.h>
#include <QtSqlLib/SchemaConfigurator.h>

namespace QtSqlLibTest
{

using BatchInsertInto = QtSqlLib::Query::BatchInsertInto;
using DatabaseException = QtSqlLib::DatabaseException;
using DataType = QtSqlLib::Schema::DataType;
using Expr = QtSqlLib::Expr;
using FromTable = QtSqlLib::Query::FromTable;
using InsertInto = QtSqlLib::Query::InsertInto;
using InsertIntoExt = QtSqlLib::Query::InsertIntoExt;
using IQuery = QtSqlLib::API::IQuery;
using IQuerySequence = QtSqlLib::API::IQuerySequence;
using IQueryVisitor = QtSqlLib::API::IQueryVisitor;
using LinkTuples = QtSqlLib::Query::LinkTuples;
using Query = QtSqlLib::Query::Query;
using QuerySequence = QtSqlLib::Query::QuerySequence;
using Schema = QtSqlLib::Schema;
using SchemaConfigurator = QtSqlLib::SchemaConfigurator;
using UpdateTable = QtSqlLib::Query::UpdateTable;

namespace TableIds
{
enum
{
  Table1,
  Table2,
  Students,
  Projects,
  Lectures
};
}

namespace Table1Cols
{
enum
{
  Id,
  Text,
  Number,
  Mandatory
};
}

namespace Table2Cols
{
enum
{
  Id = 0U,
  Text = 1U,
  Mandatory = 2U
};
}

namespace StudentsCols
{
enum
{
  Id = 3U,
  Name = 4U
};
}

namespace ProjectsCols
{
enum
{
  Id = 5U,
  Title = 6U
};
}

namespace LecturesCols
{
enum
{
  Id = 7U,
  Topic = 8U
};
}

namespace Relationships
{
enum
{
  RelationshipStudentsProjects,
  RelationshipStudentsLectures,
  SpecialRel1,
  SpecialRel2,
  SpecialRel3,
  SpecialRel4,
  SpecialRel5,
  SpecialRel6
};
}

class Funcs
{
public:
  Funcs() = delete;

  static QString getDefaultDatabaseFilename();

  static void setupReplationshipTestsSchema(DatabaseDummy& db);

  static bool isResultTuplesContaining(
    const std::vector<IQuery::ResultTuple>& results,
    Schema::Id tableId, Schema::Id columnId, QVariant value);

  static bool isResultTuplesContaining(
    const IQuery::TupleValuesList& results,
    Schema::Id tableId, Schema::Id columnId, QVariant value);

  static IQuery::TupleValuesList& getJoinedTuples(std::vector<IQuery::ResultTuple>& results,
    Schema::Id tableId, Schema::Id columnId, const QVariant& value, Schema::Id relationshipId);

  static void expectRelations(std::vector<IQuery::ResultTuple>& results,
    Schema::Id relationshipId,
    Schema::Id fromTableId, Schema::Id fromColId, Schema::Id toTableId, Schema::Id toColId,
    const QVariant& fromValue, const QVariantList& toValues);

private:
  static IQuery::TupleValuesList s_nullTupleValuesList;

};

}
