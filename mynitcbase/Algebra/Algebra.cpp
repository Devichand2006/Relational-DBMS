#include "Algebra.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "../Buffer/counter.h"

/* used to select all the records that satisfy a condition.
the arguments of the function are
- srcRel - the source relation we want to select from
- targetRel - the relation we want to select into. (ignore for now)
- attr - the attribute that the condition is checking
- op - the operator of the condition
- strVal - the value that we want to compare against (represented as a string)
*/
bool isNumber(char *str);

int Algebra::select(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], char attr[ATTR_SIZE], int op, char strVal[ATTR_SIZE]) {
    // get the srcRel's rel-id (let it be srcRelid), using OpenRelTable::getRelId()
    // if srcRel is not open in open relation table, return E_RELNOTOPEN
    int srcRelId = OpenRelTable::getRelId(srcRel);
    if (srcRelId == E_RELNOTOPEN) {
      return E_RELNOTOPEN;
    }
    // get the attr-cat entry for attr, using AttrCacheTable::getAttrCatEntry()
    // if getAttrcatEntry() call fails return E_ATTRNOTEXIST
    AttrCatEntry attrCatEntry;
    int ret = AttrCacheTable::getAttrCatEntry(srcRelId, attr, &attrCatEntry);
    if (ret != SUCCESS) {
      return E_ATTRNOTEXIST;
    }

    /*** Convert strVal to an attribute of data type NUMBER or STRING ***/

    Attribute attrVal;
    int type = attrCatEntry.attrType;

    if (type == NUMBER) {
      // if the input argument strVal can be converted to a number
        // (check this using isNumber() function)
      if (isNumber(strVal)) {
        // convert strVal to double and store it at attrVal.nVal using atof()
        attrVal.nVal = atof(strVal);
      } else {
        return E_ATTRTYPEMISMATCH;
      }
    } else if (type == STRING) {
      // copy strVal to attrVal.sVal
      strcpy(attrVal.sVal, strVal);
    }

    /*** Creating and opening the target relation ***/
    // Prepare arguments for createRel() in the following way:
    // get RelcatEntry of srcRel using RelCacheTable::getRelCatEntry()
    RelCatEntry relcat;
    ret = RelCacheTable::getRelCatEntry(srcRelId, &relcat);
    if (ret != SUCCESS) return ret;
    int src_nAttrs = relcat.numAttrs;


    /* let attr_names[src_nAttrs][ATTR_SIZE] be a 2D array of type char
        (will store the attribute names of rel). */
    // let attr_types[src_nAttrs] be an array of type int
    char attr_names[src_nAttrs][ATTR_SIZE];
    int attr_types[src_nAttrs];
    /*iterate through 0 to src_nAttrs-1 :
        get the i'th attribute's AttrCatEntry using AttrCacheTable::getAttrCatEntry()
        fill the attr_names, attr_types arrays that we declared with the entries
        of corresponding attributes
    */
    for(int i=0;i<src_nAttrs;i++)
    {
        AttrCatEntry attrcat;
        ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrcat);
        if (ret != SUCCESS) return ret;
        strcpy(attr_names[i],attrcat.attrName);
        attr_types[i]=attrcat.attrType;
    }

    /* Create the relation for target relation by calling Schema::createRel()
       by providing appropriate arguments */
    // if the createRel returns an error code, then return that value.
    ret = Schema::createRel(targetRel, src_nAttrs, attr_names, attr_types);
    if (ret != SUCCESS) return ret;
    /* Open the newly created target relation by calling OpenRelTable::openRel()
       method and store the target relid */
    /* If opening fails, delete the target relation by calling Schema::deleteRel()
       and return the error value returned from openRel() */
    int relId = OpenRelTable::openRel(targetRel);
    if (relId < 0 || relId >=MAX_OPEN)
    {
        Schema::deleteRel(targetRel);
        return relId;
    }
    /*** Selecting and inserting records into the target relation ***/
    /* Before calling the search function, reset the search to start from the
       first using RelCacheTable::resetSearchIndex() */

    Attribute record[src_nAttrs];

    /*
        The BlockAccess::search() function can either do a linearSearch or
        a B+ tree search. Hence, reset the search index of the relation in the
        relation cache using RelCacheTable::resetSearchIndex().
        Also, reset the search index in the attribute cache for the select
        condition attribute with name given by the argument `attr`. Use
        AttrCacheTable::resetSearchIndex().
        Both these calls are necessary to ensure that search begins from the
        first record.
    */
    RelCacheTable::resetSearchIndex(srcRelId);
    AttrCacheTable::resetSearchIndex(srcRelId,attr);

    // read every record that satisfies the condition by repeatedly calling
    // BlockAccess::search() until there are no more records to be read
    cmp_count=0;
    while (BlockAccess::search(srcRelId, record, attr, attrVal, op) == SUCCESS) {

        // ret = BlockAccess::insert(targetRelId, record);
        ret = BlockAccess::insert(relId, record);
        // if (insert fails) {
        //     close the targetrel(by calling Schema::closeRel(targetrel))
        //     delete targetrel (by calling Schema::deleteRel(targetrel))
        //     return ret;
        // }
        if(ret != SUCCESS)
        {
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close the targetRel by calling closeRel() method of schema layer
    Schema::closeRel(targetRel);
    printf("Number of comparisions : %d\n",cmp_count);
    // return SUCCESS.
    return SUCCESS;
}

// will return if a string can be parsed as a floating point number
bool isNumber(char *str) {
  int len;
  float ignore;
  /*
    sscanf returns the number of elements read, so if there is no float matching
    the first %f, ret will be 0, else it'll be 1

    %n gets the number of characters read. this scanf sequence will read the
    first float ignoring all the whitespace before and after. and the number of
    characters read that far will be stored in len. if len == strlen(str), then
    the string only contains a float with/without whitespace. else, there's other
    characters.
  */
  int ret = sscanf(str, "%f %n", &ignore, &len);
  return ret == 1 && len == strlen(str);
}

int Algebra::insert(char relName[ATTR_SIZE], int nAttrs, char record[][ATTR_SIZE]){
    // if relName is equal to "RELATIONCAT" or "ATTRIBUTECAT"
    // return E_NOTPERMITTED;
    if (strcmp(relName, RELCAT_RELNAME) == 0 ||
        strcmp(relName, ATTRCAT_RELNAME) == 0) {
        return E_NOTPERMITTED;
    }
    // get the relation's rel-id using OpenRelTable::getRelId() method
    int relId = OpenRelTable::getRelId(relName);

    // if relation is not open in open relation table, return E_RELNOTOPEN
    // (check if the value returned from getRelId function call = E_RELNOTOPEN)
    // get the relation catalog entry from relation cache
    // (use RelCacheTable::getRelCatEntry() of Cache Layer)
    if (relId == E_RELNOTOPEN) {
        return E_RELNOTOPEN;
    }
    RelCatEntry relCatEntry;
    int ret = RelCacheTable::getRelCatEntry(relId, &relCatEntry);
    if (ret != SUCCESS)
        return ret;
    /* if relCatEntry.numAttrs != numberOfAttributes in relation,
       return E_NATTRMISMATCH */
    if (relCatEntry.numAttrs != nAttrs) {
        return E_NATTRMISMATCH;
    }
    // let recordValues[numberOfAttributes] be an array of type union Attribute
    union Attribute recordValues[nAttrs];
    /*
        Converting 2D char array of record values to Attribute array recordValues
     */
    // iterate through 0 to nAttrs-1: (let i be the iterator)
    for (int i = 0; i < nAttrs; i++)
    {
        // get the attr-cat entry for the i'th attribute from the attr-cache
        // (use AttrCacheTable::getAttrCatEntry())
        AttrCatEntry attrCatEntry;
        ret = AttrCacheTable::getAttrCatEntry(relId, i, &attrCatEntry);
        if (ret != SUCCESS)
            return ret;
        // let type = attrCatEntry.attrType;
        int type = attrCatEntry.attrType;
        if (type == NUMBER)
        {
            // if the char array record[i] can be converted to a number
            // (check this using isNumber() function)
            if (isNumber(record[i]))
            {
                /* convert the char array to numeral and store it
                   at recordValues[i].nVal using atof() */
                   recordValues[i].nVal = atof(record[i]);
            }
            // else
            else
            {
                return E_ATTRTYPEMISMATCH;
            }
        }
        else if (type == STRING)
        {
            // copy record[i] to recordValues[i].sVal
            strcpy(recordValues[i].sVal, record[i]);
        }
    }

    // insert the record by calling BlockAccess::insert() function
    // let retVal denote the return value of insert call
    int retVal = BlockAccess::insert(relId, recordValues);
    return retVal;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    // if srcRel is not open in open relation table, return E_RELNOTOPEN
    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;
    // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()
    RelCatEntry relCatEntry;
    int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    if (ret != SUCCESS) return ret;
    // get the no. of attributes present in relation from the fetched RelCatEntry.
    int numAttrs = relCatEntry.numAttrs;
    // attrNames and attrTypes will be used to store the attribute names
    // and types of the source relation respectively
    char attrNames[numAttrs][ATTR_SIZE];
    int attrTypes[numAttrs];

    /*iterate through every attribute of the source relation :
        - get the AttributeCat entry of the attribute with offset.
          (using AttrCacheTable::getAttrCatEntry())
        - fill the arrays `attrNames` and `attrTypes` that we declared earlier
          with the data about each attribute
    */
    for(int i = 0; i < numAttrs; i++) {
        AttrCatEntry attrEntry;

        ret = AttrCacheTable::getAttrCatEntry(srcRelId, i, &attrEntry);
        if (ret != SUCCESS) return ret;

        strcpy(attrNames[i], attrEntry.attrName);
        attrTypes[i] = attrEntry.attrType;
    }

    /*** Creating and opening the target relation ***/

    // Create a relation for target relation by calling Schema::createRel()
    ret = Schema::createRel(targetRel, numAttrs, attrNames, attrTypes);
    // if the createRel returns an error code, then return that value.
    if (ret != SUCCESS) return ret;
    // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid
    int targetRelId = OpenRelTable::openRel(targetRel);
    // If opening fails, delete the target relation by calling Schema::deleteRel() of
    // return the error value returned from openRel().
    if (targetRelId < 0) {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    /*** Inserting projected records into the target relation ***/

    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()

    Attribute record[numAttrs];
    RelCacheTable::resetSearchIndex(srcRelId);

    while (BlockAccess::project(srcRelId, record) == SUCCESS)
    {
        // record will contain the next record

        // ret = BlockAccess::insert(targetRelId, proj_record);
        ret = BlockAccess::insert(targetRelId, record);
        if (ret != SUCCESS) {
            // close the targetrel by calling Schema::closeRel()
            // delete targetrel by calling Schema::deleteRel()
            // return ret;
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close the targetRel by calling Schema::closeRel()
    Schema::closeRel(targetRel);
    // return SUCCESS.
    return SUCCESS;
}

int Algebra::project(char srcRel[ATTR_SIZE], char targetRel[ATTR_SIZE], int tar_nAttrs, char tar_Attrs[][ATTR_SIZE]) {

    int srcRelId = OpenRelTable::getRelId(srcRel);

    // if srcRel is not open in open relation table, return E_RELNOTOPEN
    if (srcRelId == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    for(int i=0;i<tar_nAttrs;i++)
    for(int j=i+1;j<tar_nAttrs;j++)
      if(strcmp(tar_Attrs[i],tar_Attrs[j])==0)
          return E_DUPLICATEATTR;
    // get RelCatEntry of srcRel using RelCacheTable::getRelCatEntry()
    RelCatEntry relCatEntry;
    // get the no. of attributes present in relation from the fetched RelCatEntry.
    int ret = RelCacheTable::getRelCatEntry(srcRelId, &relCatEntry);
    if (ret != SUCCESS) return ret;
    // declare attr_offset[tar_nAttrs] an array of type int.
    // where i-th entry will store the offset in a record of srcRel for the
    // i-th attribute in the target relation.
    int src_nAttrs = relCatEntry.numAttrs;
    // let attr_types[tar_nAttrs] be an array of type int.
    // where i-th entry will store the type of the i-th attribute in the
    // target relation.
    int attr_offset[tar_nAttrs];
    int attr_types[tar_nAttrs];

    /*** Checking if attributes of target are present in the source relation
         and storing its offsets and types ***/

    /*iterate through 0 to tar_nAttrs-1 :
        - get the attribute catalog entry of the attribute with name tar_attrs[i].
        - if the attribute is not found return E_ATTRNOTEXIST
        - fill the attr_offset, attr_types arrays of target relation from the
          corresponding attribute catalog entries of source relation
    */
    for(int i = 0; i < tar_nAttrs; i++) {

        AttrCatEntry attrEntry;

        ret = AttrCacheTable::getAttrCatEntry(srcRelId, tar_Attrs[i], &attrEntry);

        if (ret != SUCCESS)
            return E_ATTRNOTEXIST;

        attr_offset[i] = attrEntry.offset;
        attr_types[i] = attrEntry.attrType;
    }

    /*** Creating and opening the target relation ***/

    // Create a relation for target relation by calling Schema::createRel()

    // if the createRel returns an error code, then return that value.

    // Open the newly created target relation by calling OpenRelTable::openRel()
    // and get the target relid

    // If opening fails, delete the target relation by calling Schema::deleteRel()
    // and return the error value from openRel()

    ret = Schema::createRel(targetRel, tar_nAttrs, tar_Attrs, attr_types);
    if (ret != SUCCESS) return ret;

    int targetRelId = OpenRelTable::openRel(targetRel);
    if (targetRelId < 0) {
        Schema::deleteRel(targetRel);
        return targetRelId;
    }

    /*** Inserting projected records into the target relation ***/

    // Take care to reset the searchIndex before calling the project function
    // using RelCacheTable::resetSearchIndex()
    RelCacheTable::resetSearchIndex(srcRelId);
    Attribute record[src_nAttrs];

    while (BlockAccess::project(srcRelId, record) == SUCCESS) {
        // the variable `record` will contain the next record

        Attribute proj_record[tar_nAttrs];

        //iterate through 0 to tar_attrs-1:
        //    proj_record[attr_iter] = record[attr_offset[attr_iter]]
        for(int i = 0; i < tar_nAttrs; i++) {
            proj_record[i] = record[attr_offset[i]];
        }
        // ret = BlockAccess::insert(targetRelId, proj_record);
        ret = BlockAccess::insert(targetRelId, proj_record);
        if (ret != SUCCESS) {
            // close the targetrel by calling Schema::closeRel()
            // delete targetrel by calling Schema::deleteRel()
            // return ret;
            Schema::closeRel(targetRel);
            Schema::deleteRel(targetRel);
            return ret;
        }
    }

    // Close the targetRel by calling Schema::closeRel()
    Schema::closeRel(targetRel);
    // return SUCCESS.
    return SUCCESS;
}

int Algebra::join(char srcRelation1[ATTR_SIZE], char srcRelation2[ATTR_SIZE],
                  char targetRelation[ATTR_SIZE],
                  char attribute1[ATTR_SIZE], char attribute2[ATTR_SIZE]) {

    // get the srcRelation1's rel-id
    int srcRelId1 = OpenRelTable::getRelId(srcRelation1);

    // get the srcRelation2's rel-id
    int srcRelId2 = OpenRelTable::getRelId(srcRelation2);

    // if either relation not open
    if (srcRelId1 == E_RELNOTOPEN || srcRelId2 == E_RELNOTOPEN)
        return E_RELNOTOPEN;

    AttrCatEntry attrCatEntry1, attrCatEntry2;

    // get attribute entries
    int ret = AttrCacheTable::getAttrCatEntry(srcRelId1, attribute1, &attrCatEntry1);
    if (ret != SUCCESS) return E_ATTRNOTEXIST;

    ret = AttrCacheTable::getAttrCatEntry(srcRelId2, attribute2, &attrCatEntry2);
    if (ret != SUCCESS) return E_ATTRNOTEXIST;

    // type check
    if (attrCatEntry1.attrType != attrCatEntry2.attrType)
        return E_ATTRTYPEMISMATCH;

    // get relation catalog entries
    RelCatEntry relCatEntry1, relCatEntry2;

    ret = RelCacheTable::getRelCatEntry(srcRelId1, &relCatEntry1);
    if (ret != SUCCESS) return ret;

    ret = RelCacheTable::getRelCatEntry(srcRelId2, &relCatEntry2);
    if (ret != SUCCESS) return ret;

    int numOfAttributes1 = relCatEntry1.numAttrs;
    int numOfAttributes2 = relCatEntry2.numAttrs;

    // check duplicate attribute names (except join attributes)
    for (int i = 0; i < numOfAttributes1; i++) {
        AttrCatEntry a1;
        AttrCacheTable::getAttrCatEntry(srcRelId1, i, &a1);

        for (int j = 0; j < numOfAttributes2; j++) {
            AttrCatEntry a2;
            AttrCacheTable::getAttrCatEntry(srcRelId2, j, &a2);

            if (strcmp(a1.attrName, a2.attrName) == 0 &&
                strcmp(a1.attrName, attribute1) != 0 &&
                strcmp(a2.attrName, attribute2) != 0) {
                return E_DUPLICATEATTR;
            }
        }
    }

    // if no index on attr2 → create it
    if (attrCatEntry2.rootBlock == -1) {
        ret = BPlusTree::bPlusCreate(srcRelId2, attribute2);
        if (ret != SUCCESS) return ret;
    }

    int numOfAttributesInTarget = numOfAttributes1 + numOfAttributes2 - 1;

    char targetRelAttrNames[numOfAttributesInTarget][ATTR_SIZE];
    int targetRelAttrTypes[numOfAttributesInTarget];

    int k = 0;

    // copy attributes from relation1
    for (int i = 0; i < numOfAttributes1; i++) {
        AttrCatEntry a;
        AttrCacheTable::getAttrCatEntry(srcRelId1, i, &a);

        strcpy(targetRelAttrNames[k], a.attrName);
        targetRelAttrTypes[k] = a.attrType;
        k++;
    }

    // copy attributes from relation2 (except join attribute)
    for (int i = 0; i < numOfAttributes2; i++) {
        AttrCatEntry a;
        AttrCacheTable::getAttrCatEntry(srcRelId2, i, &a);

        if (strcmp(a.attrName, attribute2) == 0)
            continue;

        strcpy(targetRelAttrNames[k], a.attrName);
        targetRelAttrTypes[k] = a.attrType;
        k++;
    }

    // create target relation
    ret = Schema::createRel(targetRelation, numOfAttributesInTarget,
                            targetRelAttrNames, targetRelAttrTypes);
    if (ret != SUCCESS) return ret;

    // open target relation
    int targetRelId = OpenRelTable::openRel(targetRelation);
    if (targetRelId < 0) {
        Schema::deleteRel(targetRelation);
        return targetRelId;
    }

    Attribute record1[numOfAttributes1];
    Attribute record2[numOfAttributes2];
    Attribute targetRecord[numOfAttributesInTarget];

    // reset search index for relation1
    RelCacheTable::resetSearchIndex(srcRelId1);
    while (BlockAccess::project(srcRelId1, record1) == SUCCESS) {

        // reset search index of relation2
        RelCacheTable::resetSearchIndex(srcRelId2);

        // reset search index of attribute2
        AttrCacheTable::resetSearchIndex(srcRelId2, attribute2);
        while (BlockAccess::search(
                   srcRelId2, record2, attribute2,
                   record1[attrCatEntry1.offset], EQ) == SUCCESS) {

            int idx = 0;

            // copy record1
            for (int i = 0; i < numOfAttributes1; i++) {
                targetRecord[idx++] = record1[i];
            }

            // copy record2 except join attribute
            for (int i = 0; i < numOfAttributes2; i++) {
                if (i == attrCatEntry2.offset) continue;
                targetRecord[idx++] = record2[i];
            }

            ret = BlockAccess::insert(targetRelId, targetRecord);

            if (ret != SUCCESS) {
                OpenRelTable::closeRel(targetRelId);
                Schema::deleteRel(targetRelation);
                return E_DISKFULL;
            }
        }
    }
    
    OpenRelTable::closeRel(targetRelId);
    return SUCCESS;
}
