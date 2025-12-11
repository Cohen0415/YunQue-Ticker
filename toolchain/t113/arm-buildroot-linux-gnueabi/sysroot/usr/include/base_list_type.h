/** @file
  declare struct needed by base_list, this struct will be referenced by other header files.

  @author   <eric_wang@allwinnertech.com>
  @date     2024/04/13
*/
#ifndef _BASE_LIST_TYPE_H_
#define _BASE_LIST_TYPE_H_

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

struct list_head {
	struct list_head *next, *prev;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* _BASE_LIST_TYPE_H_ */

