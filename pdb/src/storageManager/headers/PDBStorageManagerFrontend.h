//
// Created by dimitrije on 2/9/19.
//

#ifndef PDB_STORAGEMANAGERFRONTEND_H
#define PDB_STORAGEMANAGERFRONTEND_H

#include <mutex>
#include <unordered_set>

#include <PDBSet.h>
#include <PDBPageCompare.h>
#include <PDBCatalogNode.h>
#include <ServerFunctionality.h>
#include <PDBPageHandle.h>
#include <StoGetPageRequest.h>
#include <StoGetNextPageRequest.h>
#include <StoDispatchData.h>
#include <StoSetStatsRequest.h>
#include <StoStartWritingToSetRequest.h>

namespace pdb {

struct PDBStorageSetStats {

  /**
   * The size of the set
   */
  size_t size;

  /**
   * The number of pages
   */
  size_t lastPage;

};

class PDBStorageManagerFrontend : public ServerFunctionality {

public:

  virtual ~PDBStorageManagerFrontend();

  /**
   * Initialize the storage manager frontend
   */
  void init() override;

  void registerHandlers(PDBServer &forMe) override;

 private:

  /**
   * This is the response to @see requestPage. Basically it compresses the page and sends it's bytes over the wire to
   * the node that made the request.
   *
   * @tparam Communicator - the communicator class PDBCommunicator is used to handle the request. This is basically here
   * so we could write unit tests
   *
   * @param request - the request for the page we got
   * @param sendUsingMe - the communicator to the node that made the request
   * @return - the result of the handler (success, error)
   */
  template <class Communicator>
  std::pair<bool, std::string> handleGetPageRequest(const pdb::Handle<pdb::StoGetPageRequest> &request, std::shared_ptr<Communicator> &sendUsingMe);

  /**
   * This handler basically accepts the data issued by the dispatcher onto a anonymous page,
   * does some bookkeeping and forwards the page to the backend to be stored
   *
   * @tparam Communicator - the communicator class PDBCommunicator is used to handle the request. This is basically here
   * so we could write unit tests
   *
   * @tparam Requests - the factory class to make request. RequestsFactory class is being used this is just here as a template so we
   * can mock it in the unit tests
   *
   * @param request - the request to handle the dispatched data issued by the @see PDBDistributedStorage of the manager
   * @param sendUsingMe - the communicator to the node that made the request (should be the manager)
   * @return - the result of the handler (success, error)
   */
  template <class Communicator, class Requests>
  std::pair<bool, std::string> handleDispatchedData(pdb::Handle<pdb::StoDispatchData> request, std::shared_ptr<Communicator> sendUsingMe);

  /**
   * Handles the the request to get stats about a particular set.
   *
   * @tparam Communicator- the communicator class PDBCommunicator is used to handle the request. This is basically here
   * so we could write unit tests
   *
   * @tparam Requests - the factory class to make request. RequestsFactory class is being used this is just here as a template so we
   * can mock it in the unit tests
   *
   * @param request - request to get the stats of particular set within this nodes storage. Contains the database and set names
   * @param sendUsingMe - the communicator to the node that made the request
   * @return - the result of the handler (success, error)
   */
  template <class Communicator, class Requests>
  std::pair<bool, std::string> handleGetSetStats(pdb::Handle<pdb::StoSetStatsRequest> request, std::shared_ptr<Communicator> sendUsingMe);

  /**
   *
   * @tparam Communicator
   * @tparam Requests
   * @param request
   * @param sendUsingMe
   * @return
   */
  template <class Communicator, class Requests>
  std::pair<bool, std::string> handleStartWritingToSet(pdb::Handle<pdb::StoStartWritingToSetRequest> request, std::shared_ptr<Communicator> sendUsingMe);

  /**
   *
   * @param set
   * @param pageNum
   * @param size
   * @param communicator
   * @return
   */
  bool handleDispatchFailure(const PDBSetPtr &set, uint64_t pageNum, uint64_t size, PDBCommunicatorPtr communicator);

  /**
   * Checks whether we are writing to a particular page.
   * This method is not thread-safe and should only be used when locking the page mutex
   *
   * @param set - the name of the set the page we are checking belongs to
   * @param pageNum - the page number
   * @return - true if we are writing to that page false otherwise
   */
  bool isPageBeingWrittenTo(const PDBSetPtr &set, uint64_t pageNum);

  /**
   * Check whether the page is free for some reason.
   * This method is not thread-safe and should only be used when locking the page mutex
   * @param set - the name of the set the page we are checking belongs to
   * @param pageNum - the page number
   * @return true if is free, false otherwise
   */
  bool isPageFree(const PDBSetPtr &set, uint64_t pageNum);

  /**
   * Checks whether the page exists, no mater what state it is in. For example one could be writing currently to it
   * or it could be a free page.
   * This method is not thread-safe and should only be used when locking the page mutex
   *
   * @param set - the name of the set the page we are checking belongs to
   * @param pageNum - the page number
   * @return true if does false otherwise
   */
  bool pageExists(const PDBSetPtr &set, uint64_t pageNum);

  /**
   * This method returns the next free page it can find.
   * If there are free pages in the @see freeSkippedPages then we will use those otherwise we will get the next page
   * after the last page
   * This method is not thread-safe and should only be used when locking the page mutex
   *
   * @param set - the set we want to get the free page for
   * @return the id of the next page.
   */
  uint64_t getNextFreePage(const PDBSetPtr &set);

  /**
   * this method marks a page as free, meaning, that it can be assigned by get @see getNextFreePage
   * This method is not thread-safe and should only be used when locking the page mutex
   *
   * @param set - the set the page belongs to
   * @param pageNum - the number of that page
   */
  void freeSetPage(const PDBSetPtr &set, uint64_t pageNum);

  /**
   * Mark the page as being written to so that it can not be sent
   * This method is not thread-safe and should only be used when locking the page mutex
   *
   * @param set - the set the page belongs to
   * @param pageNum - the number of that page
   */
  void startWritingToPage(const PDBSetPtr &set, uint64_t pageNum);

  /**
   * Unmark the page as being written to so that the storage can send it for reading and stuff
   * This method is not thread-safe and should only be used when locking the page mutex
   *
   * @param set - the set the page belongs to
   * @param pageNum - the number of that page
   */
  void endWritingToPage(const PDBSetPtr &set, uint64_t pageNum);

  /**
   * This method increments the set size. It assumes the set exists, should not be called unless it exists!
   * This method is not thread-safe and should only be used when locking the page mutex
   * @param set
   */
  void incrementSetSize(const PDBSetPtr &set, uint64_t uncompressedSize);

  /**
   * This method decrements the set size. It assumes the set exists, should not be called unless it exists!
   * This method is not thread-safe and should only be used when locking the page mutex
   * @param set
   */
  void decrementSetSize(const PDBSetPtr &set, uint64_t uncompressedSize);

  /**
   * Retu
   * @param set
   * @return
   */
  std::shared_ptr<PDBStorageSetStats> getSetStats(const PDBSetPtr &set);

  /**
   * The logger
   */
  PDBLoggerPtr logger;

  /**
   * This keeps track of the stats for a particular set. @see PDBStorageSetStats for the kind of information that is being stored
   */
  map <PDBSetPtr, PDBStorageSetStats, PDBSetCompare> pageStats;

  /**
   * Pages that are currently being written to for a particular set
   */
  map<PDBSetPtr, std::unordered_set<uint64_t>, PDBSetCompare> pagesBeingWrittenTo;

  /**
   * The pages that we skipped for some reason when writing to. This can happen when some requests fail or something of that sort.
   */
  map<PDBSetPtr, std::unordered_set<uint64_t>, PDBSetCompare> freeSkippedPages;

  /**
   * Lock last pages
   */
  std::mutex pageMutex;
};

}

#include <PDBStorageManagerFrontendTemplate.cc>

#endif //PDB_STORAGEMANAGERFRONTEND_H
