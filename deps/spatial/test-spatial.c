#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <spatialindex/capi/sidx_api.h>



void load(IndexH idx);
void query(IndexH idx);
void bounds(IndexH idx);

void load(IndexH idx)
{
  {
  double min[] = {0.5, 0.5};
  double max[] = {0.5, 0.5};
  Index_InsertData(idx, 1, min, max, 2, 0, 0);
  }
  {
  double min[] = {-0.4, 0.75};
  double max[] = {0.25, 0.9};
  Index_InsertData(idx, 2, min, max, 2, 0, 0);
  }
}

void query(IndexH idx)
{
    double min[] = {0.0, 0.0};
    double max[] = {1.0, 1.0};
    uint64_t nResults;
    Index_Intersects_count(idx, min, max, 2, & nResults);
    assert(nResults == 2);

    int64_t * items;
    Index_Intersects_id(idx, min, max, 2, & items, & nResults);
    assert(nResults == 2);
    for(int i=0; i < nResults; i++){
      printf("Found: %ld\n", items[i]);
    }
    free(items);
}

void bounds(IndexH idx)
{
    uint32_t dims;
    double* pMins;
    double* pMaxs;
    Index_GetBounds(idx, &pMins, &pMaxs, &dims);

    if (dims == 2){
        printf("Successful: bounds: (%f,%f),(%f,%f)\n", pMins[0], pMaxs[0], pMins[1], pMaxs[1]);
    }else{
        printf("Error\n");
    }

    free(pMins);
    free(pMaxs);
}

int main(){
  //char* pszVersion = SIDX_Version();
  //fprintf(stdout, "libspatialindex version: %s\n", pszVersion);
  //fflush(stdout);
  //free(pszVersion);

  IndexPropertyH props = IndexProperty_Create();

  // create an in-memory r*-tree index
  IndexProperty_SetIndexType(props, RT_RTree);
  IndexProperty_SetIndexVariant(props, RT_Star);
  IndexProperty_SetIndexStorage(props, RT_Memory);
  IndexH idx = Index_Create(props);
  IndexProperty_Destroy(props);

  if (! Index_IsValid(idx)){
    printf("Failed to create index\n");
    exit(1);
  }
  load(idx);
  bounds(idx);
  query(idx);
  Index_Destroy(idx);

  return 0;
}
