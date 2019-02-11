#include <PDBBufferManagerFrontEnd.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <PagedRequestHandler.h>
#include <StoGetPageRequest.h>
#include <PDBBufferManagerBackEnd.h>
#include <StoGetAnonymousPageRequest.h>
#include <PagedRequest.h>
#include <StoGetPageResult.h>
#include <SimpleRequestResult.h>
#include <StoReturnPageRequest.h>
#include <StoReturnAnonPageRequest.h>
#include <StoFreezeSizeRequest.h>
#include <StoPinPageRequest.h>
#include <StoUnpinPageRequest.h>
#include <StoPinPageResult.h>
#include <HeapRequestHandler.h>
#include <StoForwardPageRequest.h>

pdb::PDBBufferManagerFrontEnd::PDBBufferManagerFrontEnd(std::string tempFileIn, size_t pageSizeIn, size_t numPagesIn, std::string metaFile, std::string storageLocIn) {

  // initialize the buffer manager
  initialize(std::move(tempFileIn), pageSizeIn, numPagesIn, std::move(metaFile), std::move(storageLocIn));
}

void pdb::PDBBufferManagerFrontEnd::init() {

  // init the logger
  //logger = make_shared<pdb::PDBLogger>((boost::filesystem::path(getConfiguration()->rootDirectory) / "PDBStorageManagerFrontend.log").string());
  logger = make_shared<pdb::PDBLogger>("PDBStorageManagerFrontend.log");
}

bool pdb::PDBBufferManagerFrontEnd::forwardPage(pdb::PDBPageHandle &page, pdb::PDBCommunicatorPtr &communicator, std::string &error) {

  // handle the page forwarding request
  return handleForwardPage(page, communicator, error);
}

void pdb::PDBBufferManagerFrontEnd::finishForwarding(pdb::PDBPageHandle &page)  {

    // lock so we can mark the page as sent
    unique_lock<mutex> lck(this->m);

    // mark that we have finished forwarding
    this->forwarding.erase(make_pair(page->getSet(), page->page->whichPage()));
}

void pdb::PDBBufferManagerFrontEnd::initForwarding(pdb::PDBPageHandle &page) {

    // lock so we can mark the page as sent
    unique_lock<mutex> lck(this->m);

    // make the key
    pair<PDBSetPtr, long> key = std::make_pair(page->getSet(), page->whichPage());

    // wait if there is a forward of the page is happening
    cv.wait(lck, [&] { return !(forwarding.find(key) != forwarding.end()); });

    // mark the page as sent
    this->sentPages[make_pair(page->getSet(), page->page->whichPage())] = page;

    // mark that we are forwarding the page
    this->forwarding.insert(make_pair(page->getSet(), page->page->whichPage()));
}

void pdb::PDBBufferManagerFrontEnd::registerHandlers(pdb::PDBServer &forMe) {
  forMe.registerHandler(StoGetPageRequest_TYPEID,
      make_shared<pdb::HeapRequestHandler<StoGetPageRequest>>(
          [&](Handle<StoGetPageRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handleGetPageRequest(request, sendUsingMe);
      }));

  forMe.registerHandler(StoGetAnonymousPageRequest_TYPEID,
      make_shared<pdb::HeapRequestHandler<StoGetAnonymousPageRequest>>(
          [&](Handle<StoGetAnonymousPageRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handleGetAnonymousPageRequest(request, sendUsingMe);
      }));

  forMe.registerHandler(StoReturnPageRequest_TYPEID,
      make_shared<pdb::HeapRequestHandler<StoReturnPageRequest>>(
          [&](Handle<StoReturnPageRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handleReturnPageRequest(request, sendUsingMe);
      }));

  forMe.registerHandler(StoReturnAnonPageRequest_TYPEID, make_shared<pdb::HeapRequestHandler<StoReturnAnonPageRequest>>(
          [&](Handle<StoReturnAnonPageRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handleReturnAnonPageRequest(request, sendUsingMe);
      }));

  forMe.registerHandler(StoFreezeSizeRequest_TYPEID,
      make_shared<pdb::HeapRequestHandler<StoFreezeSizeRequest>>(
          [&](Handle<StoFreezeSizeRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handleFreezeSizeRequest(request, sendUsingMe);
      }));

  forMe.registerHandler(StoPinPageRequest_TYPEID,
      make_shared<pdb::HeapRequestHandler<StoPinPageRequest>>([&](Handle<StoPinPageRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handlePinPageRequest(request, sendUsingMe);
      }));

  forMe.registerHandler(StoUnpinPageRequest_TYPEID,
      make_shared<pdb::HeapRequestHandler<StoUnpinPageRequest>>([&](Handle<StoUnpinPageRequest> request, PDBCommunicatorPtr sendUsingMe) {

        // call the method to handle it
        return handleUnpinPageRequest(request, sendUsingMe);
      }));
}

pdb::PDBStorageManagerInterfacePtr pdb::PDBBufferManagerFrontEnd::getBackEnd() {

  // init the backend storage manager with the shared memory
  return std::make_shared<PDBBufferManagerBackEnd<RequestFactory>>(sharedMemory);
}


