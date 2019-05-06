#ifndef _UTILS_H
#define _UTILS_H
	#include "list.h"

	#define CLR_NORMAL	"\x1B[0m"
	#define CLR_GREEN	"\x1B[32m"
	#define CLR_BLUE	"\x1B[34m"

	typedef enum cmpRes {
		EQUALLY,
		FIRST_MORE,
		SECOND_MORE,
		CMP_FAIL
	} CmpRes;

	/*	Функция запуска приложения с переменной длинной параметров
	 - Первый параметр - количество передаваемых строк
	 - Второй параметр - абсолютный путь к исполняемому файлу
	 - Третий параметр - первый параметр запуска
	 - ...... ........ - ...... ........ .......
	*/
	int startApplication(int count, ...);
	
	/*	Функция проверки существования файла
	 - Можно проверять также существование папки
	 - В параметрах передаётся путь к файлу
	*/
	int fileExists(char const *path_file);

	/*	Функция копирования прав
	 - Первый параметр - путь к файлу с которого будут копироваться права
	 - Второй параметр - путь к файлу которому будут права скопированы
	*/
	int filePermissionCopy(const char *input, const char *destination);

	/*	Функция копирования файла
	 - Также копируются права файла
	 - В параметрах передаётся путь к существующему файлу и путь к будующему
	*/
	int fileCopy(const char *file_path, const char *output_path);

	/*	Функция создания директории
	 - Первый параметр - путь к создаваемой директории
	*/
	int createDir(const char *path);

	/* Функция очистки пути к файлу
	 - Убирает точки и слеши в начале строки
	*/
	void cleanUpPath(const char *path, char *new_path);

	/*	Функция добавления точки перед именем файла
	 - Первый параметр - имя или путь к файлу, второй - выходное имя или путь
	*/
	void hidePath(const char *path, char *hidden_path);

	/*	Функция получения имени файла из пути
	 - Первый параметр - путь к файлу, второй параметр - выходное имя файла
	*/
	void getNameFromPath(const char *path, char *file_name);

	/*	Функция сравнения времени последнего изменения файлов
	 - Первый параметр - путь к первому файлу
	 - Второй параметр - путь ко второму файлу
	 - Возвращаемые значения: ennum cpmRes
	*/
	CmpRes fileLastChangeCmp(const char *f_file_path, const char *s_file_path);

	/*	Функция добавления текста в файл
	 - Первый параметр - путь к файлу
	 - Второй параметр - формат
	 - Третий параметр - данные
	 - ...... ........ - ......
	*/
	int fileAddText(char const *path_file, const char *fmt, ...);

	/*	Функция получения времени последнего изменения файла
	 - Первый параметр - путь к файлу
	*/
	time_t fileLastChange(const char *file_path);

	/*	Функция копирования директории
	 - Первый параметр - путь к директории
	 - Второй параметр - новый путь к директории
	*/
	#define dirCopy(path, storage_path)	dirCopyFull(path, storage_path, NULL)
	int dirCopyFull(const char *path, const char *storage_path, const char *local_path);

	/*	Функция получения списка файлов
	 - Первый параметр - путь к иследуемой директории
	*/
	#define getFilesList(path)	getFilesListFull(path, NULL);
	struct list *getFilesListFull(const char *path, struct list *files_list);
#endif
