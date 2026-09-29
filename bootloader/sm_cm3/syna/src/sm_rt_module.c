#include "sm_type.h"
#include "sm_common.h"
#include "sm_rt_module.h"

extern rt_module_desc_t __rt_module_descs_start__[];
extern rt_module_desc_t __rt_module_descs_end__[];

#define RT_MODULE_DESCS_START	(__rt_module_descs_start__)
#define RT_MODULE_DESCS_END	(__rt_module_descs_end__)

static rt_module_desc_t *rt_module_descs;

#define RT_MODULE_DESCS_NUM		(RT_MODULE_DESCS_END - RT_MODULE_DESCS_START)

rt_module_desc_t * get_rt_module_desc(int index)
{
	if (RT_MODULE_DESCS_NUM == 0)
		return NULL;

	if(index >= RT_MODULE_DESCS_NUM)
		return NULL;

	rt_module_descs = RT_MODULE_DESCS_START;

	return &rt_module_descs[index];
}

extern rt_init_desc_t __rt_init_descs_start__[];
extern rt_init_desc_t __rt_init_descs_end__[];

#define RT_INIT_DESCS_START	(__rt_init_descs_start__)
#define RT_INIT_DESCS_END	(__rt_init_descs_end__)

static rt_init_desc_t *rt_init_descs;

#define RT_INIT_DESCS_NUM		(RT_INIT_DESCS_END - RT_INIT_DESCS_START)

rt_init_desc_t * get_rt_init_desc(int index)
{
	if (RT_INIT_DESCS_NUM == 0)
		return NULL;

	if(index >= RT_INIT_DESCS_NUM)
		return NULL;

	rt_init_descs = RT_INIT_DESCS_START;

	return &rt_init_descs[index];
}
