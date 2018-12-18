#include <iostream>
#include "PDBCatalog.h"
#include <qunit.h>
#include <boost/filesystem.hpp>

class Tests {

  QUnit::UnitTest qunit;

  /**
   * This test tests a simple aggregation
   */
  void test() {

    // remove the catalog if it exists from a previous run
    boost::filesystem::remove("out.sqlite");

    // create a catalog
    pdb::PDBCatalog catalog("out.sqlite");

    std::string error;

    // create the databases
    QUNIT_IS_TRUE(catalog.registerDatabase(std::make_shared<pdb::PDBCatalogDatabase>("db1"), error));
    QUNIT_IS_TRUE(catalog.registerDatabase(std::make_shared<pdb::PDBCatalogDatabase>("db2"), error));

    // store the number of registred types
    auto numBefore = catalog.numRegisteredTypes();

    // create a type
    QUNIT_IS_TRUE(catalog.registerType(std::make_shared<pdb::PDBCatalogType>(8341, "built-in", "Type1", std::vector<char>()), error))
    QUNIT_IS_TRUE(catalog.registerType(std::make_shared<pdb::PDBCatalogType>(8342, "built-in", "Type2", std::vector<char>()), error));

    // check if we added two new types
    QUNIT_IS_EQUAL(catalog.numRegisteredTypes() - numBefore, 2);

    // create the set
    QUNIT_IS_TRUE(catalog.registerSet(std::make_shared<pdb::PDBCatalogSet>("set1", "db1", "Type1"), error));
    QUNIT_IS_TRUE(catalog.registerSet(std::make_shared<pdb::PDBCatalogSet>("set2", "db1", "Type1"), error));
    QUNIT_IS_TRUE(catalog.registerSet(std::make_shared<pdb::PDBCatalogSet>("set3", "db2", "Type2"), error));

    // create the nodes
    QUNIT_IS_TRUE(catalog.registerNode(std::make_shared<pdb::PDBCatalogNode>("localhost:8080", "localhost", 8080, "master", 8, 1024, true), error));
    QUNIT_IS_TRUE(catalog.registerNode(std::make_shared<pdb::PDBCatalogNode>("localhost:8081", "localhost", 8081, "worker", 8, 1024, true), error));
    QUNIT_IS_TRUE(catalog.registerNode(std::make_shared<pdb::PDBCatalogNode>("localhost:8082", "localhost", 8082, "worker", 8, 1024, true), error));

    // check the exists functions for databases
    QUNIT_IS_TRUE(catalog.databaseExists("db1"));
    QUNIT_IS_TRUE(catalog.databaseExists("db2"));

    // check the exists functions for sets
    QUNIT_IS_TRUE(catalog.setExists("db1", "set1"));
    QUNIT_IS_TRUE(catalog.setExists("db1", "set2"));
    QUNIT_IS_TRUE(catalog.setExists("db2", "set3"));

    QUNIT_IS_FALSE(catalog.setExists("db1", "set3"));
    QUNIT_IS_FALSE(catalog.setExists("db2", "set1"));
    QUNIT_IS_FALSE(catalog.setExists("db2", "set2"));

    // check if the types exist
    QUNIT_IS_TRUE(catalog.typeExists("Type1"));
    QUNIT_IS_TRUE(catalog.typeExists("Type2"));

    // check if the node exist
    QUNIT_IS_TRUE(catalog.nodeExists("localhost:8080"));
    QUNIT_IS_TRUE(catalog.nodeExists("localhost:8081"));
    QUNIT_IS_TRUE(catalog.nodeExists("localhost:8082"));

    // check the get method for the database
    auto db = catalog.getDatabase("db1");

    QUNIT_IS_EQUAL(db->name, "db1");
    QUNIT_IS_TRUE(db->createdOn > 0)

    db = catalog.getDatabase("db2");

    QUNIT_IS_EQUAL(db->name, "db2");
    QUNIT_IS_TRUE(db->createdOn > 0)

    // check the get method for the sets
    auto set = catalog.getSet("db1", "set1");

    QUNIT_IS_EQUAL(set->name, "set1")
    QUNIT_IS_EQUAL(set->setIdentifier, "db1:set1")
    QUNIT_IS_EQUAL(set->database, "db1")
    QUNIT_IS_EQUAL(*set->type, "Type1")


    set = catalog.getSet("db1", "set2");

    QUNIT_IS_EQUAL(set->name, "set2")
    QUNIT_IS_EQUAL(set->setIdentifier, "db1:set2")
    QUNIT_IS_EQUAL(set->database, "db1")
    QUNIT_IS_EQUAL(*set->type, "Type1")

    set = catalog.getSet("db2", "set3");

    QUNIT_IS_EQUAL(set->name, "set3")
    QUNIT_IS_EQUAL(set->setIdentifier, "db2:set3")
    QUNIT_IS_EQUAL(set->database, "db2")
    QUNIT_IS_EQUAL(*set->type, "Type2")

    set = catalog.getSet("db1", "set3");
    QUNIT_IS_TRUE(set == nullptr);

    // check the nodes
    auto node = catalog.getNode("localhost:8080");

    QUNIT_IS_EQUAL(node->nodeID, "localhost:8080");
    QUNIT_IS_EQUAL(node->nodeType, "master");
    QUNIT_IS_EQUAL(node->address, "localhost");
    QUNIT_IS_EQUAL(node->port, 8080);

    node = catalog.getNode("localhost:8081");

    QUNIT_IS_EQUAL(node->nodeID, "localhost:8081");
    QUNIT_IS_EQUAL(node->nodeType, "worker");
    QUNIT_IS_EQUAL(node->address, "localhost");
    QUNIT_IS_EQUAL(node->port, 8081);

    node = catalog.getNode("localhost:8082");

    QUNIT_IS_EQUAL(node->nodeID, "localhost:8082");
    QUNIT_IS_EQUAL(node->nodeType, "worker");
    QUNIT_IS_EQUAL(node->address, "localhost");
    QUNIT_IS_EQUAL(node->port, 8082);

    auto t1 = catalog.getTypeWithoutLibrary(8341);
    auto t2 = catalog.getTypeWithoutLibrary("Type1");

    QUNIT_IS_EQUAL(t1->name, t2->name)
    QUNIT_IS_EQUAL(t1->id, t2->id)
    QUNIT_IS_EQUAL(t1->typeCategory, t2->typeCategory)

    // print out the catalog
    std::cout << catalog.listNodesInCluster() << std::endl;
    std::cout << catalog.listRegisteredDatabases() << std::endl;
    std::cout << catalog.listUserDefinedTypes() << std::endl;

    // remove the database
    catalog.removeDatabase("db1", error);

    // check the get method for the sets
    QUNIT_IS_TRUE(!catalog.setExists("db1", "set1"));
    QUNIT_IS_TRUE(!catalog.setExists("db1", "set2"));
  }

 public:

  explicit Tests(std::ostream & out, int verboseLevel = QUnit::verbose): qunit(out, verboseLevel) {}

  /**
   * Runs the tests
   * @return if the tests succeeded
   */
  int run() {

    // run tests
    test();

    // return the errors
    return qunit.errors();
  }

};

int main() {
  return Tests(std::cerr).run();
}