#pragma once

#include <QString>

#include <map>

namespace QtSqlLib
{

class Schema
{
public:
  using Id = unsigned int;

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

    bool bIsPrimaryKey = false;
    bool bIsAutoIncrement = false;
    bool bIsNotNull = false;
  };

  struct Table
  {
    QString name;
    std::map<Id, Column> columns;
  };

  enum class RelationshipType
  {
    OneToMany,
    ManyToOne,
    ManyToMany
  };

  enum class ForeignKeyAction
  {
    NoAction,
    Restrict,
    SetNull,
    SetDefault,
    Cascade
  };

  struct Relationship
  {
    Id tableFromId = 0;
    Id tableToId = 0;
    RelationshipType type = RelationshipType::OneToMany;

    // TODO: refactor
    Id foreignKeyColId = 0;

    ForeignKeyAction onUpdateAction = ForeignKeyAction::NoAction;
    ForeignKeyAction onDeleteAction = ForeignKeyAction::NoAction;
  };

  Schema();
  virtual ~Schema();

  std::map<Id, Table>& getTables();
  std::map<Id, Relationship>& getRelationships();

  void configureRelationships();

private:
  std::map<Id, Table> m_tables;
  std::map<Id, Relationship> m_relationships;

};

}