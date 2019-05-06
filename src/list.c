#include <stdlib.h>
#include <string.h>

#include "include/list.h"

/*#include <stdio.h>
void printPoints(struct list *list) {
	struct list *temp_list = list;

	if (list->_p_first != NULL)
	{
		list = list->_p_first;
	}

	printf("\n");
	printf("Point now: %p\n", list);

	for (; list->_p_next != NULL; list = list->_p_next)
	{
		printf("Point: %p First: %p Prev: %p Next: %p Data: %p\n", list, list->_p_first, list->_p_prev, list->_p_next, list->data);
	}

	printf("Point: %p First: %p Prev: %p Next: %p Data: %p\n", list, list->_p_first, list->_p_prev, list->_p_next, list->data);
	printf("\n");

	list = temp_list;
}*/

struct list *listInit() {
	struct list *temp_list_element = NULL;

	temp_list_element = calloc(1, sizeof(struct list));

	if (temp_list_element != NULL)
	{
		temp_list_element->data = NULL;
		temp_list_element->data_size = 0;

		temp_list_element->_p_first = NULL;
		temp_list_element->_p_prev = NULL;
		temp_list_element->_p_next = NULL;

		return temp_list_element;
	}

	return NULL;
}

unsigned int listAdd(struct list *list, const char *something, unsigned int size) {
	unsigned int item_counter = 0;

	if (list != NULL)
	{
		if (list->_p_first == NULL)
		{
			list->_p_first = list;

			list->data = calloc(1, size);
			list->data_size = size;

			if (list->data != NULL)
			{
				memcpy(list->data, something, size);
				return 1;
			}
		}
		else
		{
			list = list->_p_first;

			// Перемещение в конец списка
			for (; list->_p_next != NULL; list = list->_p_next, ++item_counter);

			list->_p_next = calloc(1, sizeof(struct list));

			if (list->_p_next != NULL)
			{
				list->_p_next->_p_first = list->_p_first;
				list->_p_next->_p_prev = list;
				list->_p_next->_p_next = NULL;

				list->_p_next->data = calloc(1, size);
				list->_p_next->data_size = size;

				if (list->_p_next->data != NULL)
				{
					memcpy(list->_p_next->data, something, size);

					list = list->_p_first;

					return (item_counter + 2);
				}
			}
		}
	}

	return 0;
}

char *listGetItem(struct list *list, unsigned int item_num) {
	unsigned int item_counter = 0;
	char *returned_data = NULL;

	if (list != NULL)
	{
		if (list->_p_first == NULL)
		{
			return NULL;
		}

		list = list->_p_first;

		if (item_num == 0)
		{
			return list->data;
		}

		for (item_counter = 0; list->_p_next != NULL; list = list->_p_next)
		{
			if (item_counter == item_num)
			{
				break;
			}

			item_counter++;
		}

		if (item_counter == item_num)
		{
			returned_data = list->data;
			list = list->_p_first;

			return returned_data;
		}

		list = list->_p_first;
	}

	return NULL;
}

unsigned int listGetSize(struct list *list) {
	unsigned int item_counter = 0;

	if (list != NULL)
	{
		if (list->_p_first != NULL)
		{
			list = list->_p_first;
		}

		for (item_counter = 0; list->_p_next != NULL; list = list->_p_next)
		{
			item_counter++;
		}

		item_counter++;
	}

	if ((item_counter == 1) && (list->_p_first == NULL))
	{
		return 0;
	}

	list = list->_p_first;

	return item_counter;
}

struct list *listDeleteItem(struct list *list, unsigned int item_num) {
	struct list *temp_list_element = NULL;
	struct list *temp_first_list_element = NULL;

	unsigned int item_counter = 0;

	if (list != NULL)
	{
		list = list->_p_first;

		if (item_num == 0)
		{
			list->_p_first = NULL;

			if (list->_p_next != NULL)
			{
				temp_list_element = list;
				temp_first_list_element = list->_p_next;
				temp_first_list_element->_p_prev = NULL;

				for (; list->_p_next != NULL; list = list->_p_next)
				{
					list->_p_first = temp_first_list_element;
				}

				list->_p_first = temp_first_list_element;
				list = temp_list_element;
			}
		}
		else
		{
			for (item_counter = 0; list->_p_next != NULL; list = list->_p_next)
			{
				if (item_counter == item_num)
				{
					break;
				}

				item_counter++;
			}
		}

		if (item_counter == item_num)
		{
			temp_list_element = list;

			if (list->data != NULL)
			{
				free(list->data);

				list->data = NULL;
				list->data_size = 0;
			}


			if ((list->_p_prev != NULL))
			{
				if (list->_p_next != NULL)
				{
					list->_p_next->_p_prev = list->_p_prev;
					list->_p_prev->_p_next = list->_p_next;
				}
				else
				{
					list->_p_prev->_p_next = NULL;
				}
			}

			if (list->_p_next != NULL)
			{
				list = list->_p_next->_p_first;
			}
			else
			{
				if (list->_p_prev != NULL)
				{
					list = list->_p_prev->_p_first;
				}
			}

/*			if ((list != NULL) && (list->_p_prev == NULL) && (list->_p_next == NULL))
			{
				// list->_p_first = NULL;
			}
			else*/
			if ((list->_p_prev != NULL) || (list->_p_next != NULL))
			{
				free(temp_list_element);
			}
		}
	}

	return list;
}

void listFree(struct list *list) {
	struct list *temp_list_element = NULL;

	if (list != NULL)
	{
		if (list->_p_first != NULL)
		{
			list = list->_p_first;
		}

		for (; list->_p_next != NULL;)
		{
			temp_list_element = list->_p_next;

			if (list->data != NULL)
			{
				free(list->data);
			}

			free(list);

			list = temp_list_element;
		}

		if (list->data != NULL)
		{
			free(list->data);
		}

		free(list);
	}

	return;
}

unsigned int listGetItemPos(struct list *list, const char *something, unsigned int size) {
	unsigned int item_counter = 0;

	if (list != NULL)
	{
		if (list->_p_first == NULL)
		{
			return 0;
		}

		list = list->_p_first;

		for (item_counter = 1; list->_p_next != NULL; list = list->_p_next)
		{
			if ((list->data != NULL)  && (size == list->data_size) &&\
				(memcmp(something, list->data, size) == 0))
			{
				break;
			}

			item_counter++;
		}

		if ((list->data != NULL)  && (size == list->data_size) &&\
			(memcmp(something, list->data, size) == 0))
		{
			return item_counter;
		}
	}

	return 0;
}
