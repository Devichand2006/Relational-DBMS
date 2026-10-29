#include "OpenRelTable.h"
#include <cstring>
#include <stdlib.h>


OpenRelTableMetaInfo OpenRelTable::tableMetaInfo[MAX_OPEN];

OpenRelTable::OpenRelTable() {

  // initialise all values in relCache and attrCache to be nullptr and all entries
  // in tableMetaInfo to be free

  for (int i = 0; i < MAX_OPEN; ++i) {
    RelCacheTable::relCache[i] = nullptr;
    AttrCacheTable::attrCache[i] = nullptr;
    OpenRelTable::tableMetaInfo[i].free=true;
  }

  // load the relation and attribute catalog into the relation cache (we did this already)

  /************ Setting up Relation Cache entries ************/
  // (we need to populate relation cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Relation Cache Table****/
  RecBuffer relCatBlock(RELCAT_BLOCK);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, RELCAT_SLOTNUM_FOR_RELCAT);

  struct RelCacheEntry relCacheEntry;
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry.relCatEntry);
  relCacheEntry.recId.block = RELCAT_BLOCK;
  relCacheEntry.recId.slot = RELCAT_SLOTNUM_FOR_RELCAT;
  relCacheEntry.searchIndex = RecId{-1,-1};
  relCacheEntry.dirty = false;
  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[RELCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[RELCAT_RELID]) = relCacheEntry;

  /**** setting up Attribute Catalog relation in the Relation Cache Table ****/

  // set up the relation cache entry for the attribute catalog similarly
  // from the record at RELCAT_SLOTNUM_FOR_ATTRCAT

  // set the value at RelCacheTable::relCache[ATTRCAT_RELID]


  /************ Setting up Attribute cache entries ************/
  // (we need to populate attribute cache with entries for the relation catalog
  //  and attribute catalog.)

  /**** setting up Relation Catalog relation in the Attribute Cache Table ****/

  Attribute relCatRecord1[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord1, RELCAT_SLOTNUM_FOR_ATTRCAT);

  struct RelCacheEntry relCacheEntry1;
  RelCacheTable::recordToRelCatEntry(relCatRecord1, &relCacheEntry1.relCatEntry);
  relCacheEntry1.recId.block = RELCAT_BLOCK;
  relCacheEntry1.recId.slot = RELCAT_SLOTNUM_FOR_ATTRCAT;
  relCacheEntry1.searchIndex = RecId{-1,-1};
  relCacheEntry1.dirty = false;
  RelCacheTable::relCache[ATTRCAT_RELID] = (struct RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  *(RelCacheTable::relCache[ATTRCAT_RELID]) = relCacheEntry1;
  // iterate through all the attributes of the relation catalog and create a linked
  // list of AttrCacheEntry (slots 0 to 5)
  // for each of the entries, set
  //    attrCacheEntry.recId.block = ATTRCAT_BLOCK;
  //    attrCacheEntry.recId.slot = i   (0 to 5)
  //    and attrCacheEntry.neRELCAT_SLOTNUM_FOR_ATTRCATxt appropriately
  // NOTE: allocate each entry dynamically using malloc
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  struct AttrCacheEntry *head=nullptr;
  for(int i=0;i<6;i++)
  {
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatRecord, i);
        struct AttrCacheEntry *attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        attrCacheEntry->next=nullptr;
        attrCacheEntry->searchIndex = {-1, -1};
        attrCacheEntry->dirty = false;
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));
        struct AttrCacheEntry *x=head;
        while(x!=nullptr && x->next!=nullptr)
        {
            x=x->next;
        }
        if(x!=nullptr)
        {
            x->next = attrCacheEntry;
        }
        else
        {
            head=attrCacheEntry;
        }
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
  }
  // set the next field in the last entry to nullptr

  AttrCacheTable::attrCache[RELCAT_RELID] = head;

  // load the relation and attribute catalog into the attribute cache (we did this already)
  /**** setting up Attribute Catalog relation in the Attribute Cache Table ****/

  // set up the attributes of the attribute cache similarly.
  // read slots 6-11 from attrCatBlock and initialise recId appropriately

  // set the value at AttrCacheTable::attrCache[ATTRCAT_RELID]
  head=nullptr;
  for(int i=6;i<12;i++)
  {
        Attribute attrCatRecord[ATTRCAT_NO_ATTRS];
        attrCatBlock.getRecord(attrCatRecord, i);
        struct AttrCacheEntry *attrCacheEntry = (struct AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
        attrCacheEntry->next=nullptr;
        attrCacheEntry->searchIndex = {-1, -1};
        attrCacheEntry->dirty = false; 
        AttrCacheTable::recordToAttrCatEntry(attrCatRecord, &(attrCacheEntry->attrCatEntry));
        struct AttrCacheEntry *x=head;
        while(x!=nullptr && x->next!=nullptr)
        {
            x=x->next;
        }
        if(x!=nullptr)
        {
            x->next = attrCacheEntry;
        }
        else
        {
            head=attrCacheEntry;
        }
        attrCacheEntry->recId.block = ATTRCAT_BLOCK;
        attrCacheEntry->recId.slot = i;
  }
  AttrCacheTable::attrCache[ATTRCAT_RELID] = head;

  /************ Setting up tableMetaInfo entries ************/

  // in the tableMetaInfo array
  //   set free = false for RELCAT_RELID and ATTRCAT_RELID
  //   set relname for RELCAT_RELID and ATTRCAT_RELID
  OpenRelTable::tableMetaInfo[0].free=false;
  strcpy(OpenRelTable::tableMetaInfo[0].relName,"RELATIONCAT");
  OpenRelTable::tableMetaInfo[1].free=false;
  strcpy(OpenRelTable::tableMetaInfo[1].relName,"ATTRIBUTECAT");

}


OpenRelTable::~OpenRelTable() {

    for (int i = 2; i < MAX_OPEN; i++) {
        if (!tableMetaInfo[i].free) {
          // close the relation using openRelTable::closeRel().
            OpenRelTable::closeRel(i);
        }
    }

    /**** Closing the catalog relations in the relation cache ****/

    //releasing the relation cache entry of the attribute catalog
    RelCacheEntry *attrCatEntry = RelCacheTable::relCache[ATTRCAT_RELID];

    if (attrCatEntry != nullptr && attrCatEntry->dirty) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        union Attribute record[RELCAT_NO_ATTRS];

        RelCacheTable::relCatEntryToRecord(
            &attrCatEntry->relCatEntry,
            record
        );
        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = attrCatEntry->recId;
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(record, recId.slot);
    }
    // free the memory dynamically allocated to this RelCacheEntry
    if (attrCatEntry != nullptr) {
        free(attrCatEntry);
        RelCacheTable::relCache[ATTRCAT_RELID] = nullptr;
    }

    //releasing the relation cache entry of the relation catalog
    RelCacheEntry *relCatEntry = RelCacheTable::relCache[RELCAT_RELID];

    if(relCatEntry != nullptr && relCatEntry->dirty) {

        /* Get the Relation Catalog entry from RelCacheTable::relCache
        Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
        union Attribute record[RELCAT_NO_ATTRS];

        RelCacheTable::relCatEntryToRecord(
            &relCatEntry->relCatEntry,
            record
        );
        // declaring an object of RecBuffer class to write back to the buffer
        RecId recId = relCatEntry->recId;
        RecBuffer relCatBlock(recId.block);

        // Write back to the buffer using relCatBlock.setRecord() with recId.slot
        relCatBlock.setRecord(record, recId.slot);
    }
    // free the memory dynamically allocated for this RelCacheEntry
    if (relCatEntry != nullptr) {
        free(relCatEntry);
        RelCacheTable::relCache[RELCAT_RELID] = nullptr;
    }

    // free the memory allocated for the attribute cache entries of the
    // relation catalog and the attribute catalog
    AttrCacheEntry *curr;

    curr = AttrCacheTable::attrCache[RELCAT_RELID];
    while (curr != nullptr) {
        AttrCacheEntry *temp = curr;
        curr = curr->next;
        free(temp);
    }
    AttrCacheTable::attrCache[RELCAT_RELID] = nullptr;

    curr = AttrCacheTable::attrCache[ATTRCAT_RELID];
    while (curr != nullptr) {
        AttrCacheEntry *temp = curr;
        curr = curr->next;
        free(temp);
    }
    AttrCacheTable::attrCache[ATTRCAT_RELID] = nullptr;
}

int OpenRelTable::getRelId(char relName[ATTR_SIZE]) {

  /* traverse through the tableMetaInfo array,
    find the entry in the Open Relation Table corresponding to relName.*/
  for(int i=0;i<MAX_OPEN;i++)
  {
    if(!OpenRelTable::tableMetaInfo[i].free && strcmp(OpenRelTable::tableMetaInfo[i].relName,relName)==0)
    {
      return i;
    }
  }
  return E_RELNOTOPEN;
  // if found return the relation id, else indicate that the relation do not
  // have an entry in the Open Relation Table.
}

int OpenRelTable::getFreeOpenRelTableEntry() {

  /* traverse through the tableMetaInfo array,
    find a free entry in the Open Relation Table.*/
  for(int i=2;i<MAX_OPEN;i++)
  {
    if(OpenRelTable::tableMetaInfo[i].free)
    {
      return i;
    }
  }
  return E_CACHEFULL;
  // if found return the relation id, else return E_CACHEFULL.
}

int OpenRelTable::openRel(char relName[ATTR_SIZE]) {
  int r=OpenRelTable::getRelId(relName);
  if(r!=E_RELNOTOPEN){
    // (checked using OpenRelTable::getRelId())
    return r;
    // return that relation id;
  }

  /* find a free slot in the Open Relation Table
     using OpenRelTable::getFreeOpenRelTableEntry(). */
  int fsrd=OpenRelTable::getFreeOpenRelTableEntry();
  if (fsrd==E_CACHEFULL){
    return E_CACHEFULL;
  }

  // let relId be used to store the free slot.
  int relId;
  relId=fsrd;
  /****** Setting up Relation Cache entry for the relation ******/

  /* search for the entry with relation name, relName, in the Relation Catalog using
      BlockAccess::linearSearch().
      Care should be taken to reset the searchIndex of the relation RELCAT_RELID
      before calling linearSearch().*/
  // relcatRecId stores the rec-id of the relation `relName` in the Relation Catalog.
  RecId relcatRecId;
  RelCacheTable::resetSearchIndex(RELCAT_RELID);
  Attribute relNameAttr;
  strcpy(relNameAttr.sVal, relName);

  relcatRecId = BlockAccess::linearSearch(RELCAT_RELID, (char*)"RelName", relNameAttr, EQ);
  if (relcatRecId.block == -1) {
    // (the relation is not found in the Relation Catalog.)
    return E_RELNOTEXIST;
  }

  /* read the record entry corresponding to relcatRecId and create a relCacheEntry
      on it using RecBuffer::getRecord() and RelCacheTable::recordToRelCatEntry().
      update the recId field of this Relation Cache entry to relcatRecId.
      use the Relation Cache entry to set the relId-th entry of the RelCacheTable.
    NOTE: make sure to allocate memory for the RelCacheEntry using malloc()
  */
  RecBuffer relCatBlock(relcatRecId.block);

  Attribute relCatRecord[RELCAT_NO_ATTRS];
  relCatBlock.getRecord(relCatRecord, relcatRecId.slot);

  RelCacheEntry* relCacheEntry = (RelCacheEntry*)malloc(sizeof(RelCacheEntry));
  RelCacheTable::recordToRelCatEntry(relCatRecord, &relCacheEntry->relCatEntry);
  relCacheEntry->recId.block = relcatRecId.block;
  relCacheEntry->recId.slot = relcatRecId.slot;
  relCacheEntry->searchIndex = RecId{-1,-1};
  // allocate this on the heap because we want it to persist outside this function
  RelCacheTable::relCache[relId] = relCacheEntry;

  /****** Setting up Attribute Cache entry for the relation ******/
  RecBuffer attrCatBlock(ATTRCAT_BLOCK);
  struct AttrCacheEntry *head=nullptr,*tail=nullptr;
  RelCacheTable::resetSearchIndex(ATTRCAT_RELID);
  /*iterate over all the entries in the Attribute Catalog corresponding to each
  attribute of the relation relName by multiple calls of BlockAccess::linearSearch()
  care should be taken to reset the searchIndex of the relation, ATTRCAT_RELID,
  corresponding to Attribute Catalog before the first call to linearSearch().*/
  while (true) {
    /* let attrcatRecId store a valid record id an entry of the relation, relName,
      in the Attribute Catalog.*/
      RecId attrcatRecId;
      /* read the record entry corresponding to attrcatRecId and create an
      Attribute Cache entry on it using RecBuffer::getRecord() and
      AttrCacheTable::recordToAttrCatEntry().
      update the recId field of this Attribute Cache entry to attrcatRecId.
      add the Attribute Cache entry to the linked list of listHead .*/
      // NOTE: make sure to allocate memory for the AttrCacheEntry using malloc()
      attrcatRecId = BlockAccess::linearSearch(ATTRCAT_RELID, (char*)"RelName", relNameAttr, EQ);

      if (attrcatRecId.block == -1) break;

      RecBuffer attrBuf(attrcatRecId.block);
      Attribute attrRec[ATTRCAT_NO_ATTRS];
      attrBuf.getRecord(attrRec, attrcatRecId.slot);

      AttrCacheEntry* entry = (AttrCacheEntry*)malloc(sizeof(AttrCacheEntry));
      AttrCacheTable::recordToAttrCatEntry(attrRec, &entry->attrCatEntry);
      entry->recId = attrcatRecId;
      entry->searchIndex = {-1, -1};
      entry->next = nullptr;
      if (!head) {
          head = tail = entry;
      } else {
          tail->next = entry;
          tail = entry;
      }
  }

  AttrCacheTable::attrCache[relId] = head;

  // set the relIdth entry of the AttrCacheTable to listHead.

  /****** Setting up metadata in the Open Relation Table for the relation******/

  // update the relIdth entry of the tableMetaInfo with free as false and
  // relName as the input.
  tableMetaInfo[relId].free = false;
  strcpy(tableMetaInfo[relId].relName, relName);

  return relId;
}

int OpenRelTable::closeRel(int relId) {
	/* rel-id corresponds to relation catalog or attribute catalog*/
	if (relId == ATTRCAT_RELID || relId == RELCAT_RELID) {
		return E_NOTPERMITTED;
	}

	/* 0 <= relId < MAX_OPEN */
	if (relId >= MAX_OPEN || relId < 0) {
		return E_OUTOFBOUND;
	}

	/* rel-id corresponds to a free slot*/
	if (tableMetaInfo[relId].free) {
		return E_RELNOTOPEN;
	}

	/****** Releasing the Relation Cache entry of the relation ******/

	/* RelCatEntry of the relId-th Relation Cache entry has been modified */
	if (RelCacheTable::relCache[relId]->dirty)
	{

		/* Get the Relation Catalog entry from RelCacheTable::relCache
		Then convert it to a record using RelCacheTable::relCatEntryToRecord(). */
		Attribute record[RELCAT_NO_ATTRS];
		RelCatEntry relCatEntry;
		RelCacheTable::getRelCatEntry(relId, &relCatEntry);
		RelCacheTable::relCatEntryToRecord(&relCatEntry, record);

		RecId recId = RelCacheTable::relCache[relId]->recId;
		// declaring an object of RecBuffer class to write back to the buffer
		RecBuffer relCatBlock(recId.block);

		// Write back to the buffer using relCatBlock.setRecord() with recId.slot
		relCatBlock.setRecord(record, recId.slot);
	}
    // free the memory dynamically alloted to this Relation Cache entry
    // and assign nullptr to that entry
	free(RelCacheTable::relCache[relId]);

	/****** Releasing the Attribute Cache entry of the relation ******/

	// free the memory allocated in the attribute caches which was
	// allocated in the OpenRelTable::openRel() function
	AttrCacheEntry* entry = AttrCacheTable::attrCache[relId], * temp = nullptr;
	while (entry) {
		// if the entry has been modified:
		if (entry->dirty) {
            /* Get the Attribute Catalog entry from attrCache
             Then convert it to a record using AttrCacheTable::attrCatEntryToRecord().
             Write back that entry by instantiating RecBuffer class. Use recId
             member field and recBuffer.setRecord() */
			Attribute rec[ATTRCAT_NO_ATTRS];
			AttrCacheTable::attrCatEntryToRecord(&entry->attrCatEntry, rec);
			
			RecBuffer attrCatBlk(entry->recId.block);
			attrCatBlk.setRecord(rec, entry->recId.slot);
        }

		temp = entry;
		entry = entry->next;
		free(temp);
	}

	/****** Set the Open Relation Table entry of the relation as free ******/
	RelCacheTable::relCache[relId] = nullptr;
	AttrCacheTable::attrCache[relId] = nullptr;

	// update `metainfo` to set `relId` as a free slot
	tableMetaInfo[relId].free = true;

	return SUCCESS;
}