

#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#include "PDBStorageCheckLRU.h"
#include <map>
#include <memory>
#include "PDBPage.h"
#include "PDBPageHandle.h"
#include "PDBSet.h"
#include "PDBPageCompare.h"
#include <queue>
#include "PDBSetCompare.h"
#include "PDBSharedMemory.h"
#include "PDBStorageManagerInterface.h"
#include <set>
#include <NodeConfig.h>

/* This is the class that implements PC's per-node storage manager.  

   There are two types of pages. Anonymous and non-anonymous. Anonymous pages don't correspond to a disk
   page; they are pages used as RAM for temporary storage.  They remain in existence until all handles to
   them are gone, at which time they disappear.  However, anonymous pages can be swapped out of RAM by
   the storage manager, so it is possible to have more anonymous pages than can fit in the physical RAM
   of the machine.

   Non-anonymous pages correspond to data that are stored on disk for later use.
    
   Pages by default are pageSize in bytes.  But, as an optimization, they can be smaller than this as 
   well.  There are two ways to get a smaller page.  One can simply create a small anonymous page by
   calling getPage (maxBytes) which returns a page that may be as small as maxBytes in size.

   The other way is to call freezeSize (numBytes), which tells the storage manager that the page is never
   going to use more than the first numBytes bytes on the page.  freezeSize () can be used on both 
   anonymous and non-anonymous pages.

   Beause the actual pages are variable sized, we don't figure out where in a file a non-anonymous page
   is going to be written until it is unpinned (at which time its size cannot change). 

   When a page is created, it is pinned.  It is kept in RAM until it is unpinned (a page can be unpinned
   manually, or because there are no more handles in scope).  When a page is unpinned, it is assumed that
   the page is read only, forever after.  Later it can be pinned again, but it is still read-only.

   At sometime before a page is unpinned, one can call freezeSize () on a handle to the page, informing
   the system that not all of the bytes on the page are going to be used.  The system can then store only
   the first, used part of the page, at great savings.

   When a large page is broken up into mini-pages, we keep track of how many mini-pages are pinned, and we
   keep references to all of those mini-pages on the page.  As long as some mini-page on the page is pinned,
   the entire page is pinned.  Once it contains no pinned mini-pages, it can potentially be re-cycled (and
   all of the un-pinned mini-pages written back to disk).

   When a non-anonymous page is unpinned for the first time, we determine its true location on disk (pages
   may not be located sequentially on disk, due to the fact that we have variable-sized pages, and we do
   not know at the outset the actual number of bytes that will be used by a page).

   An anonymous page gets its location the first time that it is written out to disk.  

   A page can be dirty or not dirty.  All pages are dirty at creation, but then once they are written out
   to disk, they are clean forever more (by definition, a page needs to be unpinned to be written out to
   disk, but once it is unpinned, it cannot be modified, so after it is written back, it can never be
   modified again).

*/   

using namespace std;

namespace pdb {

class PDBStorageManagerImpl;
typedef shared_ptr <PDBStorageManagerImpl> PDBStorageManagerImplPtr;

class PDBStorageManagerImpl : public PDBStorageManagerInterface {

public:

  // we need the default constructor for our tests
  PDBStorageManagerImpl() = default;

  // initializes the storage manager using the node configuration
  // it will check if the node already contains a metadata file if it does it will initialize the
  // storage manager using the metadata, otherwise it will use the page and memory info of the node configuration
  // to create a new storage
  explicit PDBStorageManagerImpl(pdb::NodeConfigPtr config);

  // gets the i^th page in the table whichSet... note that if the page
  // is currently being used (that is, the page is current buffered) a handle
  // to that already-buffered page should be returned
  //
  // Under the hood, the storage manager first makes sure that it has a file
  // descriptor for the file storing the page's set.  It then checks to see
  // if the page already exists.  It it does, we just return it.  If the page
  // does not already exist, we see if we have ever created the page and
  // written it back before.  If we have, we go to the disk location for the
  // page and read it in.  If we have not, we simply get an empty set of
  // bytes to store the page and return that.
  PDBPageHandle getPage(PDBSetPtr whichSet, uint64_t i) override;

  // gets a temporary page that will no longer exist (1) after the buffer manager
  // has been destroyed, or (2) there are no more references to it anywhere in the
  // program.  Typically such a temporary page will be used as buffer memory.
  // since it is just a temp page, it is not associated with any particular
  // set.  On creation, the page is pinned until it is unpinned.
  //
  // Under the hood, this simply finds a mini-page to store the page on (kicking
  // existing data out of the buffer if necessary)
  PDBPageHandle getPage () override;

  // gets a temporary page that is at least minBytes in size
  PDBPageHandle getPage (size_t minBytes) override;

  // tells the storage manager to use/create the given meta-data file.  If the file exists, then the
  // the meta-data is read from the file.  If the file does not exist, then the file is ignored.
  // return the system page size
  size_t getPageSize () override;

  // simply loop through and write back any dirty pages.
  ~PDBStorageManagerImpl () override;

  // Initialize a storage manager.  Anonymous pages will be written to tempFile.  Use the given pageSize.
  // The number of pages available to buffer data in RAM is numPages.  All meta-data is written to
  // metaDataFile.  All files to hold database files are written to the directory storageLoc
  void initialize (std :: string tempFile, size_t pageSize, size_t numPages, std :: string metaDataFile, std :: string storageLocIn);

  // initialize the storage manager using the file metaDataFile
  void initialize (std :: string metaDataFile);

  // the storage manager does not have any server functionalities, they will be defined in the frontend (makes testing easier)
  void registerHandlers(PDBServer &forMe) override {};

protected:

  // "registers" a min-page.  That is, do record-keeping so that we can link the mini-page
  // to the full page that it is located on top of.  Since this is called when a page is created
  // or read back from disk, it calls "pinParent" to make sure that the parent (full) page cannot be
  // written out
  void registerMiniPage (PDBPagePtr registerMe);

  // this is called when there are no more external references to an anonymous page, and so
  // it can be destroyed.  To do this, we first unpin it (if it is pinned) and then remove it
  // from its parent's list of constituent pages.
  void freeAnonymousPage (PDBPagePtr me) override;

  // this creates additional mini-pages of size MIN_PAGE_SIZE * 2^whichSize.  To do this, it
  // looks at the full page with the largest LRU number.  It then looks through all of the
  // mini-pages that have been allocated on that full page, and frees each of them.  To free
  // such a page, there are two cases: the page to be freed is anonymous, or it is not.  If it
  // is anonymous, then if it is dirty, we get a spot for it in the temp file and kick it out.
  // If the page is not anonymous, it is written back (it must already have a spot to be
  // written to, because it has to have been unpinned) and then if there are no references to
  // it, it is destroyed.
  void createAdditionalMiniPages(int64_t whichSize);

  // tell the buffer manager that the given page can be truncated at the indcated size
  void freezeSize (PDBPagePtr me, size_t numBytes) override;

  // unpin the page.  This freezes the size of the page (because now the page is read-only)
  // and then decrements the number of pinned pages on this pages' full parent page.  If this
  // page is not anonymous, we determine where its actual location on disk will be (for an
  // anonymous page, we wait until the page has to be written back to determine its location,
  // because unlike non-anonymous pages, anonymous pages will often never make it to disk)
  void unpin (PDBPagePtr me) override;

  // pins the page that is the parent of a mini-page.  The "parent" is the page that contains
  // the physical bits for the mini-page.  To pin the parent, we first determine the parent,
  // then we increment the number of pinned pages in the parent.  If the parent is not currently
  // pinned, we remove it from the LRU queue (its current LRU number is the negative of the
  // number of pinned pages in this case)
  void pinParent (PDBPagePtr me);

  // repins a page (it is called "repin" because by definition, each page is pinned upon
  // creation, so every page has been pinned at least once).  To repin, if the page is already
  // in RAM, we just pin the page, and then pin the page's parent.  If it is not in RAM, then
  // if it is not in RAM, then we get a mini-page to store this guy, read it in, register the
  // mini page he is written on (this allows the parent page to be aware that the mini-page
  // is located on top of him, so he can't be kicked out while the mini-page is pinned), and
  // then note that this guy is now pinned
  void repin (PDBPagePtr me) override;

  // this is called when there are zero external references to a page.  We remove all traces
  // of the page from the system, as long as the page is not being buffered in RAM (if it is,
  // then the page may be removed later if its parent page is recycled)
  void downToZeroReferences (PDBPagePtr me) override;

  // list of ALL of the page objects that are currently in existence
  map <pair <PDBSetPtr, size_t>, PDBPagePtr, PDBPageCompare> allPages;

  // tells us, for each set, where each of the various pages are physically located.  The i^th entry in the
  // vector tells us where to find the i^th entry in the set
  map <pair <PDBSetPtr, size_t>, PDBPageInfo, PDBPageCompare> pageLocations;

  // this tells us, for each set, the last used location in the file
  map <PDBSetPtr, size_t, PDBSetCompare> endOfFiles;

  // tells us the LRU number of each of the memory pages
  set <pair <void *, size_t>, PDBStorageCheckLRU> lastUsed;

  // tells us how many of the minipages constructed from each page are pinned
  // if the long is a negative value, it gives us the LRU number
  map <void *, long> numPinned;

  // lists the FDs for all of the files
  map <PDBSetPtr, int, PDBSetCompare> fds;

  // all of the full pages that are currently not being used
  vector <void *> emptyFullPages;

  // all of the mini-pages that make up a page
  map <void *, vector <PDBPagePtr>> constituentPages;

  // all of the locations from which we are currently allocating minipages.  The first
  // entry in this vector is used to allocated minipages of size MIN_PAGE_SIZE, the
  // second minipages of size MIN_PAGE_SIZE * 2, and so on.  Each entry in the vector
  // is a pair containing a pointer to the full page that is being used to allocate
  // minipages, and a pointer to the next slot that we'll allocate from on the minipage
  vector <vector <void *>> emptyMiniPages;

  // all of the positions in the temporary file that are currently not in use
  vector <vector<size_t>> availablePositions;

  // info about the shared memory of this storage manager contains the page size, number of pages and a pointer to
  // the shared memory
  PDBSharedMemory sharedMemory {};

  // the time tick associated with the MRU page
  long lastTimeTick = -1;

  // the last position in the temporary file
  size_t lastTempPos = 0;

  // where we write the data
  string tempFile;
  int32_t tempFileFD = 0;

  // this is the log of pageSize / MIN_PAGE_SIZE
  int64_t logOfPageSize = 0;

  // the location of the meta data file
  string metaDataFile;

  // the location where data are written
  string storageLoc;

  // whether the storage manager has been initialized
  bool initialized = false;

  friend class PDBPage;
};

}

#endif

