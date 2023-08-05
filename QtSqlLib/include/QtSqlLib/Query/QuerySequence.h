#pragma once

#include <QtSqlLib/API/IQueryElement.h>
#include <QtSqlLib/API/IQuerySequence.h>

#include <memory>
#include <vector>

namespace QtSqlLib::Query
{

class QuerySequence : public API::IQueryElement,
                      public API::IQuerySequence
{
public:
  void accept(API::IQueryVisitor& visitor) override;

  void addQuery(std::unique_ptr<API::IQueryElement> query) override;

  void prepare(API::ISchema& schema) override;
  void traverse(API::IQueryVisitor& visitor) override;

  int getNumQueries() const;
  API::IQueryElement& getQuery(int index);

private:
  std::vector<std::unique_ptr<IQueryElement>> m_queryElements;

};

}
