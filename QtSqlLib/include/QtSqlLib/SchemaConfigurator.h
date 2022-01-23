#pragma once

#include <QtSqlLib/RelationshipConfigurator.h>
#include <QtSqlLib/TableConfigurator.h>

#include <QString>

#include <map>
#include <memory>

namespace QtSqlLib
{

class SchemaConfigurator
{
public:
  struct Schema
  {
    std::map<unsigned int, TableConfigurator::Table> tables;
    std::map<unsigned int, RelationshipConfigurator::Relationship> relationships;
  };

  SchemaConfigurator(Schema& schema);
  virtual ~SchemaConfigurator();

  TableConfigurator& configureTable(unsigned int tableId, const QString& tableName);
  RelationshipConfigurator& configureRelationship(unsigned int relationshipId, unsigned int tableFromId,
    unsigned int tableToId, RelationshipConfigurator::RelationshipType type);


private:
  Schema& m_schema;

  std::map<unsigned int, std::unique_ptr<TableConfigurator>> m_tableConfigurators;
  std::map<unsigned int, std::unique_ptr<RelationshipConfigurator>> m_relationshipConfigurators;

  bool isTableNameExisting(const QString& name) const;

};

}
