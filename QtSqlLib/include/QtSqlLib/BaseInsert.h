#pragma once
#pragma once

#include <QtSqlLib/IQuery.h>
#include <QtSqlLib/Schema.h>

#include <vector>

namespace QtSqlLib
{

class BaseInsert : public IQuery
{
public:
  BaseInsert(Schema::Id tableId);
  ~BaseInsert() override;

  void addColumnId(Schema::Id id);
  Schema::Id getTableId() const;

protected:
  QSqlQuery getQSqlQuery(Schema& schema) const;
  virtual void bindQueryValues(QSqlQuery& query) const = 0;

private:
  void throwIfColumnIdAlreadyExisting(Schema::Id id) const;

  Schema::Id m_tableId;
  std::vector<Schema::Id> m_columnIds;

};

}