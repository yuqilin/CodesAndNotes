
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#else
#define inline
#endif

#include "list.h"

#ifdef __cplusplus
}
#endif

struct FilterInfo
{
	char name[_MAX_PATH];
};

struct FilterItem
{
	int index;
	struct FilterInfo* pInfo;
	struct list_head list;
};


#define NUM_FILTERS	3

int main()
{
	int i;
	struct list_head filters_list, *temp;
	struct FilterItem filters[NUM_FILTERS];

	struct FilterItem* pItem = NULL;
	struct list_head* pos = NULL;
	int nFilterCount=0;

	INIT_LIST_HEAD(&filters_list);
	
	for (i=0; i<NUM_FILTERS; i++)
	{
		filters[i].index = i;
		filters[i].pInfo = (struct FilterInfo*)malloc(sizeof(struct FilterInfo));
		sprintf(filters[i].pInfo->name, "filter%d", i+1);
		list_add_tail(&filters[i].list, &filters_list);
	}

	list_for_each(pos, &filters_list)
	{
		pItem = (struct FilterItem *)list_entry(pos, struct FilterItem, list);
		printf("Filter %d, index:%d, info_name:%s\n", nFilterCount++, pItem->index, pItem->pInfo->name);
	}

	list_for_each_safe(pos, temp, &filters_list)
	{
		pItem = (struct FilterItem *)list_entry(pos, struct FilterItem, list);
		free(pItem->pInfo);
		list_del(pos);
	}

	return 0;
}