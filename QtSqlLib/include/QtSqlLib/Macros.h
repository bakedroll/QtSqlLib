#pragma once

// Configurator
#define CONFIGURE_TABLE(X, Y) configureTable(QtSqlLib::ID(X), Y)
#define CONFIGURE_RELATIONSHIP(X, Y, Z, T) configureRelationship(QtSqlLib::ID(X), QtSqlLib::ID(Y), QtSqlLib::ID(Z), T)
#define CONFIGURE_INDEX(X) configureIndex(QtSqlLib::ID(X))

#define COLUMN(X, Y, Z) column(QtSqlLib::ID(X), Y, Z)
#define COLUMN_VARCHAR(X, Y, Z) column(QtSqlLib::ID(X), Y, QtSqlLib::API::DataType::Varchar, Z)

#define PRIMARY_KEY primaryKey()
#define AUTO_INCREMENT autoIncrement()
#define NOT_NULL notNull()

#define PRIMARY_KEYS(X) primaryKeys(X)
#define UNIQUE_COLS(X) uniqueCols(X)

#define ON_DELETE(X) onDelete(X)
#define ON_UPDATE(X) onUpdate(X)
#define ENABLE_FOREIGN_KEY_INDEXING enableForeignKeyIndexing()

#define UNIQUE unique()
#define COLUMNS(X) columns(X)

// Expr
#define EQUAL(A, B) equal(QtSqlLib::ID(A), QVariant(B))
#define EQUAL_COL(A, B) equal(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::ColumnID(QtSqlLib::ID(B))))

#define UNEQUAL(A, B) unequal(QtSqlLib::ID(A), QVariant(B))
#define UNEQUAL_COL(A, B) unequal(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::ColumnID(QtSqlLib::ID(B))))

#define LESSEQUAL(A, B) lessEqual(QtSqlLib::ID(A), QVariant(B))
#define LESSEQUAL_COL(A, B) lessEqual(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::ColumnID(QtSqlLib::ID(B))))

#define LESS(A, B) less(QtSqlLib::ID(A), QVariant(B))
#define LESS_COL(A, B) less(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::ColumnID(QtSqlLib::ID(B))))

#define GREATEREQUAL(A, B) greaterEqual(QtSqlLib::ID(A), QVariant(B))
#define GREATEREQUAL_COL(A, B) greaterEqual(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::ColumnID(QtSqlLib::ID(B))))

#define GREATER(A, B) greater(QtSqlLib::ID(A), QVariant(B))
#define GREATER_COL(A, B) greater(QtSqlLib::ID(A), QVariant::fromValue(QtSqlLib::ColumnID(QtSqlLib::ID(B))))

#define ISNULL(A, B) greater(QtSqlLib::ID(A))

#define OR opOr()
#define AND opAnd()

#define _(A) braces(QtSqlLib::Expr().A)

// Queries
#define INSERT_INTO(X) QtSqlLib::Query::InsertInto(QtSqlLib::ID(X))
#define INSERT_INTO_EXT(X) QtSqlLib::Query::InsertIntoExt(QtSqlLib::ID(X))
#define BATCH_INSERT_INTO(X) QtSqlLib::Query::BatchInsertInto(QtSqlLib::ID(X))
#define FROM_TABLE(X) QtSqlLib::Query::FromTable(QtSqlLib::ID(X))
#define UPDATE_TABLE(X) QtSqlLib::Query::UpdateTable(QtSqlLib::ID(X))
#define DELETE_FROM(X) QtSqlLib::Query::DeleteFrom(QtSqlLib::ID(X))
#define LINK_TUPLES(X) QtSqlLib::Query::LinkTuples(QtSqlLib::ID(X))
#define UNLINK_TUPLES(X) QtSqlLib::Query::UnlinkTuples(QtSqlLib::ID(X))

#define IDS(...) QtSqlLib::IDList::make(__VA_ARGS__)
#define VALUES(X, Y) values(QtSqlLib::ID(X), Y)
#define VALUE(X, Y) value(QtSqlLib::ID(X), Y)

#define SET(X, Y) set(QtSqlLib::ID(X), Y)

#define WHERE(X) where(QtSqlLib::Expr().X)

#define SELECT_ALL selectAll()
#define SELECT(X) select(X)

#define JOIN_ALL(X) joinAll(QtSqlLib::ID(X))
#define JOIN(X, Y) join(QtSqlLib::ID(X), Y)

#define LINK_TO_ONE_TUPLE(X, Y) linkToOneTuple(QtSqlLib::ID(X), Y)
#define LINK_TO_MANY_TUPLES(X, ...) linkToManyTuples(QtSqlLib::ID(X), __VA_ARGS__)

#define FROM_ONE(X) fromOne(X)
#define FROM_REMAINING_KEY fromRemainingKey()
#define TO_ONE(X) toOne(X)
#define TO_MANY(...) toMany(__VA_ARGS__)

#define RETURN_IDS returnIds()
