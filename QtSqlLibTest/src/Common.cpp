#include "Common.h"

#include <gtest/gtest.h>

namespace QtSqlLibTest
{

QString Funcs::getDefaultDatabaseFilename()
{
  return "test.db";
}

void Funcs::setupReplationshipTestsSchema(DatabaseDummy& db)
{
  db.setConfigureSchemaFunc([](SchemaConfigurator& configurator)
  {
    configurator.configureTable(TableIds::Students, "students")
      .column(StudentsCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(StudentsCols::Name, "name", DataType::Varchar, 128);

    configurator.configureTable(TableIds::Projects, "projects")
      .column(ProjectsCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(ProjectsCols::Title, "title", DataType::Varchar, 128);

    configurator.configureTable(TableIds::Lectures, "lectures")
      .column(LecturesCols::Id, "id", DataType::Integer).primaryKey().autoIncrement().notNull()
      .column(LecturesCols::Topic, "topic", DataType::Varchar, 128);

    configurator.configureRelationship(Relationships::RelationshipStudentsProjects, TableIds::Students, TableIds::Projects,
      Schema::RelationshipType::OneToMany).onDelete(Schema::ForeignKeyAction::Cascade);

    configurator.configureRelationship(Relationships::RelationshipStudentsLectures, TableIds::Students, TableIds::Lectures,
      Schema::RelationshipType::ManyToMany);
  });
}

bool Funcs::isResultTuplesContaining(const std::vector<IQuery::ResultTuple>& results, Schema::Id tableId,
                                     Schema::Id columnId, QVariant value)
{
  const Schema::TableColumnId colId{ tableId, columnId };
  for (const auto& result : results)
  {
    if (result.values.count(colId) == 0)
    {
      return false;
    }
    if (result.values.at(colId) == value)
    {
      return true;
    }
  }
  return false;
}

bool Funcs::isResultTuplesContaining(const IQuery::TupleValuesList& results, Schema::Id tableId,
                                     Schema::Id columnId, QVariant value)
{
  const Schema::TableColumnId colId{ tableId, columnId };
  for (const auto& result : results)
  {
    if (result.count(colId) == 0)
    {
      return false;
    }
    if (result.at(colId) == value)
    {
      return true;
    }
  }
  return false;
}

IQuery::TupleValuesList& Funcs::getJoinedTuples(std::vector<IQuery::ResultTuple>& results,
                                                               Schema::Id tableId, Schema::Id columnId, const QVariant& value,
                                                               Schema::Id relationshipId)
{
  const Schema::TableColumnId colId{ tableId, columnId };
  for (auto& result : results)
  {
    if (result.values.at(colId) == value)
    {
      return result.joinedTuples.at(relationshipId);
    }
  }
  return s_nullTupleValuesList;
}

void Funcs::expectRelations(std::vector<IQuery::ResultTuple>& results, Schema::Id relationshipId,
                            Schema::Id fromTableId, Schema::Id fromColId, Schema::Id toTableId, Schema::Id toColId,
                            const QVariant& fromValue, const QVariantList& toValues)
{
  ASSERT_TRUE(isResultTuplesContaining(results, fromTableId, fromColId, fromValue));

  auto joinedTuples = getJoinedTuples(results, fromTableId, fromColId, fromValue, relationshipId);
  EXPECT_EQ(joinedTuples.size(), toValues.size());

  for (const auto& value : toValues)
  {
    EXPECT_TRUE(isResultTuplesContaining(joinedTuples, toTableId, toColId, value));
  }
}

IQuery::TupleValuesList Funcs::s_nullTupleValuesList;

}
