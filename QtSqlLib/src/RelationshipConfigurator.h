#pragma once

#include <QtSqlLib/API/IRelationshipConfigurator.h>

#include <optional>

namespace QtSqlLib
{

class RelationshipConfigurator : public API::IRelationshipConfigurator
{
public:
  RelationshipConfigurator(API::Relationship& relationship);
  ~RelationshipConfigurator() override;

  IRelationshipConfigurator& onDelete(API::ForeignKeyAction action) override;
  IRelationshipConfigurator& onUpdate(API::ForeignKeyAction action) override;
  IRelationshipConfigurator& enableForeignKeyIndexing() override;
  IRelationshipConfigurator& attribute(
    const API::IID& attributeId, const QString& attributeName, API::DataType type, int varcharLength) override;

  IRelationshipConfigurator& notNull() override;

private:
  API::Relationship& m_relationship;
  std::optional<API::IID::Type> m_lastAttributeId;

  bool m_bOnDeleteCalled;
  bool m_bOnUpdateCalled;
  bool m_bEnableForeignKeyIndexingCalled;

  bool isAttributeNameExisting(const QString& name) const;
  void checkAttribute() const;

};

}