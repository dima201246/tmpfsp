#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>

#include "include/utils.h"

/*	Функция запуска приложения с переменной длинной параметров
 - Первый параметр - количество передаваемых строк
 - Второй параметр - абсолютный путь к исполняемому файлу
 - Третий параметр - первый параметр запуска
 - ...... ........ - ...... ........ .......
*/

int startApplication(int count, ...) {
	char **params = NULL;
	unsigned int i = 0;
	pid_t chpid = 0;
	int status = 0;
	va_list args;
	int res = 0;

	chpid = fork();

	if (chpid == 0)
	{
		params = calloc(count + 1, sizeof(char *));
		params[count] = NULL;

		for (i = 0; i < count; ++i)
		{
			params[i] = calloc(64, sizeof(char));
		}

		va_start(args, count);

		for (int i = 0; i < count; ++i) {
			strcpy(params[i], va_arg(args, char *));
		}

		va_end(args);

		res = execv(params[0], params);

		if (res != 0)
		{
			for (i = 0; i < count; ++i)
			{
				free(params[i]);
			}

			free(params);
			exit(res);
		}
	}
	else
	{
		waitpid(chpid, &status, WUNTRACED);
	}

	return status;
}

/*	Функция проверки существования файла
 - Можно проверять также существование папки
 - В параметрах передаётся путь к файлу
*/

int fileExists(char const *path_file) {
	FILE * fpath;

	if ((fpath = fopen(path_file, "r")) == NULL)
	{
		return 0;
	}

	fclose(fpath);

	return 1;
}

/*	Функция копирования прав
 - Первый параметр - путь к файлу с которого будут копироваться права
 - Второй параметр - путь к файлу которому будут права скопированы
*/

int filePermissionCopy(const char *input, const char *destination) {
	struct stat file_stats = {};
	int res = 0;

	res = stat(input, &file_stats);

	if (res != 0)
	{
		return -1;
	}

	res = chmod(destination, file_stats.st_mode);

	if (res != 0)
	{
		return -2;
	}

	res = chown(destination, file_stats.st_uid, file_stats.st_gid);

	if (res != 0)
	{
		return -3;
	}

	return 0;
}

/*	Функция сравнения времени последнего изменения файлов
 - Первый параметр - путь к первому файлу
 - Второй параметр - путь ко второму файлу
 - Возвращаемые значения: ennum cpmRes
*/

CmpRes fileLastChangeCmp(const char *f_file_path, const char *s_file_path) {
	struct stat f_file_stats = {};
	struct stat s_file_stats = {};
	int res = 0;

	res = stat(f_file_path, &f_file_stats);

	if (res != 0)
	{
		return CMP_FAIL;
	}

	res = stat(s_file_path, &s_file_stats);

	if (res != 0)
	{
		return CMP_FAIL;
	}

	if (f_file_stats.st_mtime > s_file_stats.st_mtime)
	{
		return FIRST_MORE;
	}

	if (f_file_stats.st_mtime < s_file_stats.st_mtime)
	{
		return SECOND_MORE;
	}

	return EQUALLY;
}

/*	Функция получения времени последнего изменения файла
 - Первый параметр - путь к файлу
*/

time_t fileLastChange(const char *file_path) {
	struct stat file_stats = {};
	int res = 0;

	res = stat(file_path, &file_stats);

	if (res != 0)
	{
		return 0;
	}

	return file_stats.st_mtime;
}

/*	Функция копирования файла
 - Также копируются права файла
 - В параметрах передаётся путь к существующему файлу и путь к будующему
*/

int fileCopy(const char *file_path, const char *output_path) {
	char buf[1024] = {};
	int count = 0;
	int res = 0;
	FILE *file_read_p = NULL;
	FILE *file_write_p = NULL;

	file_read_p = fopen(file_path,"rb");
	file_write_p = fopen(output_path,"wb");

	if ((file_read_p == NULL) || (file_write_p == NULL))
	{
		return -1;
	}

	fseek(file_read_p, 0, SEEK_SET);

	while (!feof(file_read_p))
	{
		count = fread(&buf, 1, (1024 * sizeof(char)), file_read_p);

		fwrite(&buf, 1, (count * sizeof(char)), file_write_p);
	}

	fclose(file_read_p);
	fclose(file_write_p);

	res = filePermissionCopy(file_path, output_path);

	if (res != 0)
	{
		return -1;
	}

	return 0;
}

/*	Функция создания директории
 - Первый параметр - путь к создаваемой директории
*/

int createDir(const char *path) {
	unsigned int i = 0;
	char buf[1024] = {};
	int res = 0;

	for (i = 0; i <= strlen(path); ++i)
	{
		buf[i] = path[i];

		if ((buf[i] == '/') || (buf[i] == '\0'))
		{
			// Проверка чтобы директория не существовала
			if (fileExists(buf) == 0)
			{
				// Создание папки со стандартными правами
				res = mkdir(buf, (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH));

				if (res != 0)
				{
					return -1;
				}
			}
		}
	}

	return 0;
}

/* Функция очистки пути к файлу
 - Убирает точки и слеши в начале строки
*/

void cleanUpPath(const char *path, char *new_path) {
	unsigned int i = 0;
	unsigned int path_start_index = 0;

	// Поиск начала текста
	for (i = 0; path[i] != '\0'; ++i)
	{
		if ((path[i] != '.') && (path[i] != '/'))
		{
			path_start_index = i;

			break;
		}
	}

	// Сдвиг всех символов влево на path_start_index
	for (; path[i] != '\0'; ++i)
	{
		new_path[i - path_start_index] = path[i];
	}

	// Добавление знака конца строки
	new_path[i - path_start_index] = '\0';
}

/*	Функция добавления точки перед именем файла
 - Первый параметр - имя или путь к файлу, второй - выходное имя или путь
*/

void hidePath(const char *path, char *hidden_path) {
	int i = 0;
	unsigned int start_file_name_index = 0;

	for (i = strlen(path); i >= 0; --i)
	{
		if (path[i] == '/')
		{
			start_file_name_index = i + 1;

			break;
		}
	}

	for (i = 0; path[i] != '\0'; ++i)
	{
		if (start_file_name_index != 0)
		{
			if (i <= (start_file_name_index - 1))
			{
				hidden_path[i] = path[i];
			}

			if (i == (start_file_name_index - 1))
			{
				hidden_path[i + 1] = '.';
			}

			if (i > (start_file_name_index - 1))
			{
				hidden_path[i + 1] = path[i];
			}
		}
		else
		{
			if (i == 0)
			{
				hidden_path[i] = '.';
				hidden_path[i + 1] = path[i];
			}
			else
			{
				hidden_path[i + 1] = path[i];
			}
		}
	}

	hidden_path[i + 1] = '\0';
}

/*	Функция получения имени файла из пути
 - Первый параметр - путь к файлу, второй параметр - выходное имя файла
*/

void getNameFromPath(const char *path, char *file_name) {
	int i = 0;
	unsigned int start_file_name_index = 0;

	for (i = strlen(path); i >= 0; --i)
	{
		if ((path[i] == '/') && (i != (strlen(path) - 1)))
		{
			start_file_name_index = i + 1;

			break;
		}
	}

	for (i = start_file_name_index; path[i] != '\0'; ++i)
	{
		file_name[i - start_file_name_index] = path[i];
	}

	file_name[i - start_file_name_index] = '\0';

	if (file_name[strlen(file_name) - 1] == '/')
	{
		file_name[i - start_file_name_index - 1] = '\0';
	}
}

/*	Функция копирования директории
 - Первый параметр - путь к директории
 - Второй параметр - новый путь к директории
*/

int dirCopyFull(const char *path, const char *storage_path, const char *local_path) {
	DIR * dir_p = NULL;
	struct dirent *dir = NULL;
	int res = 0;
	char new_local_path[1024] = {};
	char next_path[1024] = {};
	char buf0[1024] = {};
	char buf1[1024] = {};

	if (fileExists(storage_path) == 0)
	{
		return -1;
	}

	dir_p = opendir(path);

	if (dir_p == NULL)
	{
		return -1;
	}

	// Создание корневой директории
	if (local_path == NULL)
	{
		getNameFromPath(path, buf0);
		sprintf(buf1, "%s/%s", storage_path, buf0);

		if (fileExists(buf1) == 0)
		{
			createDir(buf1);
			filePermissionCopy(path, buf1);
		}
	}

	while ((dir = readdir(dir_p)) != NULL)
	{
		if (dir->d_type != DT_DIR)
		{
			if (local_path == NULL)
			{
				getNameFromPath(path, buf0);
				sprintf(buf1, "%s/%s/%s", storage_path, buf0, dir->d_name);
			}
			else
			{
				sprintf(buf1, "%s/%s/%s", storage_path, local_path, dir->d_name);
			}

			// Генерация пути к существующему файлу
			sprintf(buf0, "%s/%s", path, dir->d_name);

			res = fileCopy(buf0, buf1);

			if (res < 0)
			{
				return res;
			}
		}
		else
		{
			if ((dir->d_type == DT_DIR) && (strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0))
			{
				sprintf(next_path, "%s/%s", path, dir->d_name);

				cleanUpPath(next_path, buf0);

				if (local_path == NULL)
				{
					// Получение имени копируемой папки
					getNameFromPath(path, buf1);
					sprintf(new_local_path, "%s/%s", buf1, dir->d_name);
				}
				else
				{
					sprintf(new_local_path, "%s/%s", local_path, dir->d_name);
				}

				// Генерация пути к новой папке
				sprintf(buf1, "%s/%s", storage_path, new_local_path);

				if (fileExists(buf1) == 0)
				{
					res = createDir(buf1);

					if (res != 0)
					{
						return -1;
					}

					// Копирование прав на созданную папку
					filePermissionCopy(next_path, buf1);

					if (res != 0)
					{
						return -1;
					}
				}

				res = dirCopyFull(next_path, storage_path, new_local_path);

				if (res != 0)
				{
					closedir(dir_p);
					return res;
				}
			}
		}
	}

	closedir(dir_p);
	return 0;
}

/*	Функция получения списка файлов
 - Первый параметр - путь к иследуемой директории
*/

struct list *getFilesListFull(const char *path, struct list *files_list) {
	DIR *dir_p = NULL;
	struct dirent *dir = NULL;
	struct list *local_files_list = files_list;
	char next_path[1024] = {};
	char buf0[1024] = {};

	dir_p = opendir(path);

	if (dir_p == NULL)
	{
		return NULL;
	}

	if (files_list == NULL)
	{
		local_files_list = listInit();
	}

	while ((dir = readdir(dir_p)) != NULL)
	{
		if (dir->d_type != DT_DIR)
		{
			// Генерация пути к существующему файлу
			sprintf(buf0, "%s/%s", path, dir->d_name);
			listAdd(local_files_list, buf0, (strlen(buf0) + 1));
		}
		else
		{
			if ((dir->d_type == DT_DIR) && (strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0))
			{
				sprintf(next_path, "%s/%s", path, dir->d_name);
				getFilesListFull(next_path, local_files_list);
			}
		}
	}

	closedir(dir_p);
	return local_files_list;
}

/*	Функция добавления текста в файл
 - Первый параметр - путь к файлу
 - Второй параметр - формат
 - Третий параметр - данные
 - ...... ........ - ......
*/

int fileAddText(char const *path_file, const char *fmt, ...) {
	char str_buf[512];
	FILE *some_file = NULL;

	va_list ap;
	va_start(ap, fmt);
	vsprintf(str_buf, fmt, ap);
	va_end(ap);

	if ((some_file = fopen(path_file, "a")) != NULL)
	{
		fprintf(some_file, "%s", str_buf);
		fclose(some_file);
	}
	else
	{
		return -1;
	}

	return 0;
}


int fileFindText(const char *path_file, char *match_string) {
	FILE *some_file = NULL;
	char buf[1024] = {'\0'};
	unsigned long int count = 0;

	some_file = fopen(path_file, "r");

	while (fgets(buf, 1024, some_file) != NULL)
	{
		count++;

		if (strstr(buf, match_string) != NULL)
		{
			fclose(some_file);

			return count;
		}
	}

	fclose(some_file);

	return 0;
}