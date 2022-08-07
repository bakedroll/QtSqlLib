#pragma once
#pragma once

#include <QtSqlLib/Query/Query.h>
#include <QtSqlLib/API/ISchema.h>

#include <vector>

namespace QtSqlLib::Query
{

class BaseInsert : public Query
{
public:
  BaseInsert(API::ISchema::Id tableId);
  ~BaseInsert() override;

  void addColumnId(API::ISchema::Id id);

protected:
  QSqlQuery getQSqlQuery(const QSqlDatabase& db, API::ISchema& schema) const;
  virtual void bindQueryValues(QSqlQuery& query) const = 0;

private:
  void throwIfColumnIdAlreadyExisting(API::ISchema::Id id) const;

  API::ISchema::Id m_tableId;
  std::vector<API::ISchema::Id> m_columnIds;

};

}
