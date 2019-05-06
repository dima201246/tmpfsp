#ifndef _LIST_H
#define _LIST_H

	struct list {
		char *data;
		unsigned int data_size;

		struct list *_p_first;
		struct list *_p_prev;
		struct list *_p_next;
	};

	/*Инициализация списка*/
	struct list *listInit();
	/*Добавление элемента в список. Возвращаемое значение: количество элементов в списке*/
	unsigned int listAdd(struct list *list, const char *something, unsigned int size);
	/*Получение количества элементов в списке*/
	unsigned int listGetSize(struct list *list);
	/*Получение элемента по индексу*/
	char *listGetItem(struct list *list, unsigned int item_num);
	/*Удаление элемента по индексу. Возвращаемое значение: указатель на новый список*/
	struct list *listDeleteItem(struct list *list, unsigned int item_num);
	/*Получение индекса искомого элемента*/
	unsigned int listGetItemPos(struct list *list, const char *something, unsigned int size);
	/*Освобождение памяти*/
	void listFree(struct list *list);
#endif
