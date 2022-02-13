#pragma once
#pragma once

#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/Schema.h>

#include <vector>

namespace QtSqlLib::Query
{

class BaseInsert : public Query
{
public:
  BaseInsert(Schema::Id tableId);
  ~BaseInsert() override;

  void addColumnId(Schema::Id id);

protected:
  QSqlQuery getQSqlQuery(Schema& schema) const;
  virtual void bindQueryValues(QSqlQuery& query) const = 0;

private:
  void throwIfColumnIdAlreadyExisting(Schema::Id id) const;

  Schema::Id m_tableId;
  std::vector<Schema::Id> m_columnIds;

};

}
