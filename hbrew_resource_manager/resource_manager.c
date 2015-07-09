#include "resource_manager.h"
#include "data_hash_map.h"

int resource_manager_initialize(VHLCalls *calls)
{
  data_hmap_initialize(calls);
  return 0;
}
