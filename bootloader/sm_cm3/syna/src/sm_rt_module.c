#include "sm_type.h"
#include "sm_common.h"
#include "sm_rt_module.h"

extern unsigned int __rt_module_descs_start__;
extern unsigned int __rt_module_descs_end__;

#define RT_MODULE_DESCS_START	(&__rt_module_descs_start__)
#define RT_MODULE_DESCS_END	(&__rt_module_descs_end__)

static rt_module_desc_t *rt_module_descs;

#define RT_MODULE_DESCS_NUM		(((unsigned int)RT_MODULE_DESCS_END - (unsigned int)RT_MODULE_DESCS_START)\
					/ sizeof(rt_module_desc_t))

rt_module_desc_t * get_rt_module_desc(int index)
{
	if (RT_MODULE_DESCS_NUM == 0)
		return NULL;

	if(index >= RT_MODULE_DESCS_NUM)
		return NULL;

	rt_module_descs = (rt_module_desc_t *)RT_MODULE_DESCS_START;

	return &rt_module_descs[index];
}

extern unsigned int __rt_init_descs_start__;
extern unsigned int __rt_init_descs_end__;

#define RT_INIT_DESCS_START	(&__rt_init_descs_start__)
#define RT_INIT_DESCS_END	(&__rt_init_descs_end__)

static rt_init_desc_t *rt_init_descs;

#define RT_INIT_DESCS_NUM		(((unsigned int)RT_INIT_DESCS_END - (unsigned int)RT_INIT_DESCS_START)\
					/ sizeof(rt_init_desc_t))

rt_init_desc_t * get_rt_init_desc(int index)
{
	if (RT_INIT_DESCS_NUM == 0)
		return NULL;

	if(index >= RT_INIT_DESCS_NUM)
		return NULL;

	rt_init_descs = (rt_init_desc_t *)RT_INIT_DESCS_START;

	return &rt_init_descs[index];
}
