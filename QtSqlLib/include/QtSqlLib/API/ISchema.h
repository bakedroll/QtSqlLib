#pragma once

#include <map>
#include <set>

#include <QVariant>

#include <QtSqlLib/API/IID.h>

namespace QtSqlLib::API
{

class ISchema
{
public:
  struct TableColumnId
  {
    IID::Type tableId = 0;
    IID::Type columnId = 0;

    bool operator<(const TableColumnId& rhs) const;
    bool operator!=(const TableColumnId& rhs) const;
  };

  struct RelationshipTableId
  {
    IID::Type relationshipId = 0;
    IID::Type tableId = 0;

    bool operator<(const RelationshipTableId& rhs) const;
  };

  using TupleValues = std::map<TableColumnId, QVariant>;
  using PrimaryForeignKeyColumnIdMap = std::map<TableColumnId, IID::Type>;

  enum class ForeignKeyAction
  {
    NoAction,
    Restrict,
    SetNull,
    SetDefault,
    Cascade
  };

  enum class DataType
  {
    Integer,
    Real,
    Varchar,
    Blob
  };

  struct Column
  {
    QString name;
    DataType type = DataType::Integer;
    int varcharLength = 0;

    bool bIsAutoIncrement = false;
    bool bIsNotNull = false;
  };

  struct ForeignKeyReference
  {
    IID::Type referenceTableId = 0;
    ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
    ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
    PrimaryForeignKeyColumnIdMap primaryForeignKeyColIdMap;
  };

  using RelationshipToForeignKeyReferencesMap = std::map<RelationshipTableId, std::vector<ForeignKeyReference>>;

  struct Table
  {
    QString name;
    std::map<IID::Type, Column> columns;
    RelationshipToForeignKeyReferencesMap relationshipToForeignKeyReferencesMap;
    std::set<IID::Type> primaryKeys;
  };

  enum class RelationshipType
  {
    OneToMany,
    ManyToOne,
    ManyToMany
  };

  struct Relationship
  {
    IID::Type tableFromId = 0;
    IID::Type tableToId = 0;
    RelationshipType type = RelationshipType::OneToMany;
    ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
    ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
  };

  ISchema() = default;
  virtual ~ISchema() = default;

  virtual std::map<IID::Type, Table>& getTables() = 0;
  virtual std::map<IID::Type, Relationship>& getRelationships() = 0;

  virtual IID::Type getManyToManyLinkTableId(IID::Type relationshipId) const = 0;

  virtual void configureRelationships() = 0;

  virtual void throwIfTableIdNotExisting(IID::Type tableId) const = 0;
  virtual void throwIfRelationshipIsNotExisting(IID::Type relationshipId) const = 0;
  virtual void throwIfColumnIdNotExisting(const Table& table, IID::Type colId) const = 0;

  virtual IID::Type validatePrimaryKeysAndGetTableId(const TupleValues& tupleKeyValues) const = 0;
  virtual IID::Type validatePrimaryKeysListAndGetTableId(const std::vector<TupleValues>& tupleKeyValuesList) const = 0;

  virtual std::pair<IID::Type, IID::Type> verifyOneToOneRelationshipPrimaryKeysAndGetTableIds(
    IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const TupleValues& toTupleKeyValues) const = 0;

  virtual std::pair<IID::Type, IID::Type> verifyOneToManyRelationshipPrimaryKeysAndGetTableIds(
    IID::Type relationshipId,
    const TupleValues& fromTupleKeyValues,
    const std::vector<TupleValues>& toTupleKeyValuesList) const = 0;

};

}
