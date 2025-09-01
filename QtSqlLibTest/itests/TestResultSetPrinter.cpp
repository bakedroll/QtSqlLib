#include <gtest/gtest.h>

#include <Common.h>

#include <QFile>

#include <limits>

namespace QtSqlLibTest
{

class TestResultSetPrinter : public testing::Test
{
public:
  TestResultSetPrinter()
  {
    QFile::remove(Funcs::getDefaultDatabaseFilename());
    setupTestDatabase(m_db);
  }

  ~TestResultSetPrinter() override
  {
    m_db.close();
  }

  QtSqlLib::Database m_db;

private:
  static void setupTestDatabase(QtSqlLib::API::IDatabase& m_db);

};

static void expectResult(QtSqlLib::ResultSetPrinter& printer, const std::vector<QString>& expectation)
{
  std::vector<QString> result;
  while (!printer.isEndOfTable())
  {
    result.emplace_back(printer.nextPrinterLine());
  }

  ASSERT_EQ(result.size(), expectation.size());
  for (size_t i=0; i<result.size(); ++i)
  {
    EXPECT_EQ(result.at(i), expectation.at(i));
  }
}

void TestResultSetPrinter::setupTestDatabase(QtSqlLib::API::IDatabase& m_db)
{
  SchemaConfigurator configurator;
  configurator.CONFIGURE_TABLE(TableIds::Albums, "albums")
    .COLUMN(AlbumsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(AlbumsCols::Name, "name", 128).NOT_NULL;

  configurator.CONFIGURE_TABLE(TableIds::Artists, "artists")
    .COLUMN(ArtistsCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(ArtistsCols::Name, "name", 128).NOT_NULL;

  configurator.CONFIGURE_TABLE(TableIds::Tracks, "tracks")
    .COLUMN(TracksCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(TracksCols::Name, "name", 128).NOT_NULL
    .COLUMN(TracksCols::Length, "length", DataType::Integer).NOT_NULL
    .COLUMN(TracksCols::Rating, "rating", DataType::Real);

  configurator.CONFIGURE_TABLE(TableIds::SteamingServices, "streaming_services")
    .COLUMN(StreamingServicesCols::Id, "id", DataType::Integer).PRIMARY_KEY.AUTO_INCREMENT.NOT_NULL
    .COLUMN_VARCHAR(StreamingServicesCols::Name, "name", 128).NOT_NULL;

  configurator.CONFIGURE_RELATIONSHIP(Relationships::AlbumArtists, TableIds::Albums, TableIds::Artists,
    QtSqlLib::API::RelationshipType::ManyToMany).ON_DELETE(QtSqlLib::API::ForeignKeyAction::Cascade);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::AlbumTracks, TableIds::Albums, TableIds::Tracks,
    QtSqlLib::API::RelationshipType::OneToMany).ON_DELETE(QtSqlLib::API::ForeignKeyAction::Cascade);

  configurator.CONFIGURE_RELATIONSHIP(Relationships::AlbumStreamingServices, TableIds::Albums, TableIds::SteamingServices,
    QtSqlLib::API::RelationshipType::ManyToMany).ON_DELETE(QtSqlLib::API::ForeignKeyAction::Cascade);

  m_db.initialize(configurator, Funcs::getDefaultDatabaseFilename());

  // Albums
  const auto album1 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 1")
    .RETURN_IDS).nextTuple().primaryKey();

  const auto album2 = m_db.execQuery(INSERT_INTO_EXT(TableIds::Albums)
    .VALUE(AlbumsCols::Name, "Album 2")
    .RETURN_IDS).nextTuple().primaryKey();

  // Artists
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Artists)
    .VALUE(ArtistsCols::Name, "Artist 1")
    .LINK_TO_MANY_TUPLES(Relationships::AlbumArtists, { album1, album2 }));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Artists)
    .VALUE(ArtistsCols::Name, "Artist 2")
    .LINK_TO_ONE_TUPLE(Relationships::AlbumArtists, album2));

  // Streaming services
  m_db.execQuery(INSERT_INTO_EXT(TableIds::SteamingServices)
    .VALUE(StreamingServicesCols::Name, "Service 1")
    .LINK_TO_ONE_TUPLE(Relationships::AlbumStreamingServices, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::SteamingServices)
    .VALUE(StreamingServicesCols::Name, "Service 2")
    .LINK_TO_MANY_TUPLES(Relationships::AlbumStreamingServices, { album1, album2 }));

  // Album 1 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 1)
      .VALUE(TracksCols::Rating, 1.0)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2 with much longer name")
      .VALUE(TracksCols::Length, 2)
      .VALUE(TracksCols::Rating, 2.5)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album1));

  // Album 2 tracks
  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 1")
      .VALUE(TracksCols::Length, 3)
      .VALUE(TracksCols::Rating, 123456.7890123456789)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 2")
      .VALUE(TracksCols::Length, 4)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));

  m_db.execQuery(INSERT_INTO_EXT(TableIds::Tracks)
      .VALUE(TracksCols::Name, "Track 3")
      .VALUE(TracksCols::Length, std::numeric_limits<qlonglong>::max())
      .VALUE(TracksCols::Rating, 3.5)
      .LINK_TO_ONE_TUPLE(Relationships::AlbumTracks, album2));
}

/**
 * @test: Test if the ResultSetPrinter formats the columns correctly.
 * @expected: The columns don't exceed the maximum size, but are at minimum as wide as the containing value text.
 */
TEST_F(TestResultSetPrinter, printedColumnsFormat)
{
  const std::vector<QString> expectedResult = {
    " 'albums'.'id' | 'albums'.'name' | 'tracks'.'id' | 'tracks'.'name'               | 'tracks'.'length'   | 'tracks'.'rating'  ",
    "----------------------------------------------------------------------------------------------------------------------------",
    " 1             | Album 1         | 1             | Track 1                       | 1                   | 1                  ",
    " 1             | Album 1         | 2             | Track 2 with much longer name | 2                   | 2.5                ",
    " 2             | Album 2         | 3             | Track 1                       | 3                   | 123456.78901234567 ",
    " 2             | Album 2         | 4             | Track 2                       | 4                   | NULL               ",
    " 2             | Album 2         | 5             | Track 3                       | 9223372036854775807 | 3.5                "
  };

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .JOIN(Relationships::AlbumTracks, TracksCols::Id, TracksCols::Name, TracksCols::Length, TracksCols::Rating));

  auto printer = m_db.createResultSetPrinter(results, 100);
  expectResult(printer, expectedResult);
}

/**
 * @test: Test output of query with multiple joins.
 * @expected: Multiple joins are correctly displayed.
 */
TEST_F(TestResultSetPrinter, printMultipleJoins)
{
  const std::vector<QString> expectedResult = {
    " 'albums'.'id' | 'albums'.'name' | 'artists'.'id' | 'artists'.'name' | 'tracks'.'name'          | 'tracks'.'id' | 'streaming_services'.... | 'streaming_services'.... ",
    "----------------------------------------------------------------------------------------------------------------------------------------------------------------------",
    " 1             | Album 1         | 1              | Artist 1         | Track 1                  | 1             | Service 2                | 2                        ",
    " 1             | Album 1         | 1              | Artist 1         | Track 2 with much lon... | 2             | Service 2                | 2                        ",
    " 2             | Album 2         | 1              | Artist 1         | Track 1                  | 3             | Service 1                | 1                        ",
    " 2             | Album 2         | 1              | Artist 1         | Track 1                  | 3             | Service 2                | 2                        ",
    " 2             | Album 2         | 1              | Artist 1         | Track 2                  | 4             | Service 1                | 1                        ",
    " 2             | Album 2         | 1              | Artist 1         | Track 2                  | 4             | Service 2                | 2                        ",
    " 2             | Album 2         | 1              | Artist 1         | Track 3                  | 5             | Service 1                | 1                        ",
    " 2             | Album 2         | 1              | Artist 1         | Track 3                  | 5             | Service 2                | 2                        ",
    " 2             | Album 2         | 2              | Artist 2         | Track 1                  | 3             | Service 1                | 1                        ",
    " 2             | Album 2         | 2              | Artist 2         | Track 1                  | 3             | Service 2                | 2                        ",
    " 2             | Album 2         | 2              | Artist 2         | Track 2                  | 4             | Service 1                | 1                        ",
    " 2             | Album 2         | 2              | Artist 2         | Track 2                  | 4             | Service 2                | 2                        ",
    " 2             | Album 2         | 2              | Artist 2         | Track 3                  | 5             | Service 1                | 1                        ",
    " 2             | Album 2         | 2              | Artist 2         | Track 3                  | 5             | Service 2                | 2                        "
  };

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .JOIN(Relationships::AlbumArtists, ArtistsCols::Id, ArtistsCols::Name)
    .JOIN(Relationships::AlbumTracks, TracksCols::Name)
    .JOIN(Relationships::AlbumStreamingServices, StreamingServicesCols::Name));

  auto printer = m_db.createResultSetPrinter(results, 24);
  expectResult(printer, expectedResult);
}

/**
 * @test: Test output of query with aggregated functions.
 * @expected: Aggregated functions are correctly displayed.
 */
TEST_F(TestResultSetPrinter, printAggregatedFunctions)
{
  const std::vector<QString> expectedResult = {
    " 'albums'.'id' | 'albums'.'name' | COUNT(ALL 'albums'.'id') | 'tracks'.'id' ",
    "----------------------------------------------------------------------------",
    " 1             | Album 1         | 2                        | 1             ",
    " 2             | Album 2         | 3                        | 3             "
  };

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name, COUNT(AlbumsCols::Id))
    .JOIN(Relationships::AlbumTracks, TracksCols::Id)
    .GROUP_BY(AlbumsCols::Id));

  auto printer = m_db.createResultSetPrinter(results, 24);
  expectResult(printer, expectedResult);
}

/**
 * @test: Test output of query that has no results.
 * @expected: Empty table is correctly displayed.
 */
TEST_F(TestResultSetPrinter, printEmptyTable)
{
  const std::vector<QString> expectedResult = {
    " 'albums'.'id' | 'albums'.'name' ",
    "---------------------------------"
  };

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .WHERE(EQUAL(AlbumsCols::Name, "INVALID_ALBUM_NAME")));

  auto printer = m_db.createResultSetPrinter(results, 24);
  expectResult(printer, expectedResult);
}

/**
 * @test: Test output of query with join that has no values.
 * @expected: Join without values is displayed correctly.
 */
TEST_F(TestResultSetPrinter, printNullJoins)
{
  const std::vector<QString> expectedResult = {
    " 'albums'.'id' | 'albums'.'name' | 'artists'.'id' | 'artists'.'name' | 'tracks'.'name'          | 'tracks'.'id' | 'streaming_services'.... | 'streaming_services'.... ",
    "----------------------------------------------------------------------------------------------------------------------------------------------------------------------",
    " 1             | Album 1         | 1              | Artist 1         | Track 1                  | 1             | Service 2                | 2                        ",
    " 1             | Album 1         | 1              | Artist 1         | Track 2 with much lon... | 2             | Service 2                | 2                        ",
    " 2             | Album 2         | 1              | Artist 1         | NULL                     | NULL          | Service 1                | 1                        ",
    " 2             | Album 2         | 1              | Artist 1         | NULL                     | NULL          | Service 2                | 2                        ",
    " 2             | Album 2         | 2              | Artist 2         | NULL                     | NULL          | Service 1                | 1                        ",
    " 2             | Album 2         | 2              | Artist 2         | NULL                     | NULL          | Service 2                | 2                        "
  };

  m_db.execQuery(DELETE_FROM(TableIds::Tracks).WHERE(GREATEREQUAL(TracksCols::Length, 3)));

  auto results = m_db.execQuery(FROM_TABLE(TableIds::Albums)
    .SELECT(AlbumsCols::Id, AlbumsCols::Name)
    .JOIN(Relationships::AlbumArtists, ArtistsCols::Id, ArtistsCols::Name)
    .JOIN(Relationships::AlbumTracks, TracksCols::Name)
    .JOIN(Relationships::AlbumStreamingServices, StreamingServicesCols::Name));

  auto printer = m_db.createResultSetPrinter(results, 24);
  expectResult(printer, expectedResult);
}

}
