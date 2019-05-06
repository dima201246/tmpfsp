#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <dirent.h>

#include "third_party/iniparser/src/iniparser.h"
#include "include/utils.h"
#include "include/list.h"
#include "include/logging.h"

#define ver	"0.9.0 (Alpha)"

#define dirCopyMod(path, storage_path, watch_list_path)	dirCopyFullMod(path, storage_path, NULL, watch_list_path)
#define getFilesListMod(path)	getFilesListFullMod(path, NULL);

#define CONFIG_PATH "configs/main.ini"
#define CONFIG_PATH_ALT "/etc/tmpfsp.ini"
#define PATH_SHUTDOWN_BACKUP "/bin/bac_shdwnbin"

volatile char want_exit = 0;
volatile char want_shutdown = 0;

void sighandler(int sig) {
	switch (sig) {
		case SIGTERM: {
			want_exit = 1;
		} break;

		case SIGUSR1: {
			want_exit = 1;
			want_shutdown = 1;
		} break;
	}
}

int tmpfsUmount(const char *path) {
	return umount(path);
}

int tmpfsMount(const char *path, const char *size) {
	const char *src  = "tmpfs";
	const char *type = "tmpfs";
	unsigned long mntflags = 0;
	char opts[64] = {};

	snprintf(opts, 64, "size=%s", size);
	// snprintf(tmpfs_size, 32, "size=%s,mode=0700,uid=65534", size);

	return mount(src, path, type, mntflags, opts);
}

int loop() {
	while (want_exit == 0)
	{
		sleep(1);
	}

	return 0;
}

int dirCopyFullMod(const char *path, const char *storage_path, const char *local_path, const char *watch_list_path) {
	DIR * dir_p = NULL;
	struct dirent *dir = NULL;
	int res = 0;
	char new_local_path[1024] = {};
	char next_path[1024] = {};
	char buf0[1024] = {};
	char buf1[1024] = {};

	if (fileExists(storage_path) == 0)
	{
		fprintf(stderr, "Not found destination directory\n");
		return -1;
	}

	dir_p = opendir(path);

	if (dir_p == NULL)
	{
		fprintf(stderr, "Can't open dir %s\n", path);
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

			printf("%s%s -> %s%s\n", CLR_BLUE, buf0, buf1, CLR_NORMAL);
			logInFile("File: %s -> %s\n", buf0,  buf1);

			res = fileCopy(buf0, buf1);

			if (res < 0)
			{
				return res;
			}

			fileAddText(watch_list_path, "%s\n", buf1);
			fileAddText(watch_list_path, "%ld\n", fileLastChange(buf1));
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
					printf("%s%s -> %s%s\n", CLR_GREEN, next_path,  buf1, CLR_NORMAL);
					logInFile("Dir: %s -> %s\n", next_path,  buf1);

					res = createDir(buf1);

					if (res != 0)
					{
						fprintf(stderr, "Can't create directory\n");
						return -1;
					}

					// Копирование прав на созданную папку
					filePermissionCopy(next_path, buf1);

					if (res != 0)
					{
						fprintf(stderr, "Can't set permission for directory\n");
						return -1;
					}
				}

				res = dirCopyFullMod(next_path, storage_path, new_local_path, watch_list_path);

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

struct list *getFilesListFullMod(const char *path, struct list *files_list) {
	DIR *dir_p = NULL;
	struct dirent *dir = NULL;
	struct list *local_files_list = files_list;
	char next_path[1024] = {};
	char buf0[1024] = {};
	char buf1[128] = {};

	dir_p = opendir(path);

	if (dir_p == NULL)
	{
		fprintf(stderr, "Can't open dir %s\n", path);
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
			// Получение времени последнего изменения файла
			sprintf(buf1, "%ld", fileLastChange(buf0));
			listAdd(local_files_list, buf1, (strlen(buf1) + 1));
		}
		else
		{
			if ((dir->d_type == DT_DIR) && (strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0))
			{
				sprintf(next_path, "%s/%s", path, dir->d_name);
				getFilesListFullMod(next_path, local_files_list);
			}
		}
	}

	closedir(dir_p);
	return local_files_list;
}

int proccessDirs(const char *partition_path, const char *dirs_list, const char *watch_list_path) {
	char path_buf[1024] = {0};
	char buf[1024] = {0};
	char buf1[1024] = {0};
	unsigned int i = 0;
	unsigned int j = 0;
	int res = 0;

	if (fileExists(watch_list_path) == 1)
	{
		res = remove(watch_list_path);

		if (res != 0)
		{
			perror("Fail in delete watch list file");
			return res;
		}
	}

	for (;; ++i)
	{
		path_buf[j] = dirs_list[i];
		j++;

		if ((dirs_list[i] == ',') || (dirs_list[i] == '\0'))
		{
			if ((i > 0) && (dirs_list[i - 1] == '\\'))
			{
				continue;
			}

			path_buf[j - 1] = '\0';
			printf("-> %s\n", path_buf);

			res = dirCopyMod(path_buf, partition_path, watch_list_path);

			if (res != 0)
			{
				fprintf(stderr, "Fail in copy dir!\n");
				return res;
			}

			hidePath(path_buf, buf);
			rename(path_buf, buf);

			getNameFromPath(path_buf, buf);
			sprintf(buf1, "%s/%s", partition_path, buf);
			symlink(buf1, path_buf);

			j = 0;
			memset(path_buf, '\0', 1024);
		}

		if (dirs_list[i] == '\0')
		{
			break;
		}
	}

	return 0;
}

void showVersion() {
	printf("tmpfsProducer version: %s\n", ver);
}

void showHelp() {
	printf("Using: [-skvh]\n");
	printf("\t-s\t\tstart\n");
	printf("\t-k\t\tkill demon\n");
	printf("\t-v\t\tshow version\n");
	printf("\t-h\t\tshow this help\n");
}

int killDemon(char shutdown) {
	char buf[32] = {0};
	const char *pid_path = NULL;
	const char *ini_path = NULL;
	FILE *file_pid = NULL;
	dictionary *ini = NULL;
	int res = 0;
	uid_t uid = 0;

	// Загрузка файла конфигураций
	if (fileExists(CONFIG_PATH))
	{
		ini_path = CONFIG_PATH;
	}
	else
	{
		ini_path = CONFIG_PATH_ALT;
	}

	ini = iniparser_load(ini_path);

	if (ini == NULL)
	{
		fprintf(stderr, "FAIL: Can't read config file!\n");
		return -2;
	}

	pid_path = iniparser_getstring(ini, "main:pid_path", "\0");

	setLogFile(iniparser_getstring(ini, "main:log_file_path", NULL));

	file_pid = fopen(pid_path, "r");

	if (file_pid == NULL)
	{
		printf("Nothing running\n");
	}
	else
	{
		uid = getuid();

		if (uid != 0)
		{
			fprintf(stderr, "You is not root\n");
			return -1;
		}

		fgets(buf, 32, file_pid);
		fclose(file_pid);

		logMsg("Stopping...\n\n");

		if (shutdown)
		{
			res = kill(atoi(buf), SIGUSR1);
		}
		else
		{
			res = kill(atoi(buf), SIGTERM);
		}

		if (res != 0)
		{
			perror("Fail in send signal");
			return -2;
		}
	}

	iniparser_freedict(ini);

	return 0;
}

struct list *parceDirsList(const char *dirs_list) {
	struct list *list_dirs = NULL;
	char path_buf[1024] = {0};

	unsigned int i = 0;
	unsigned int j = 0;

	list_dirs = listInit();

	for (;; ++i)
	{
		path_buf[j] = dirs_list[i];
		j++;

		if ((dirs_list[i] == ',') || (dirs_list[i] == '\0'))
		{
			if ((i > 0) && (dirs_list[i - 1] == '\\'))
			{
				continue;
			}

			path_buf[j - 1] = '\0';

			listAdd(list_dirs, path_buf, (strlen(path_buf) + 1));

			j = 0;
			memset(path_buf, '\0', 1024);
		}

		if (dirs_list[i] == '\0')
		{
			break;
		}
	}

	return list_dirs;
}

void getOriginalPath(const char *dirs_list_str, const char *storage_path, const char *file_path, char *orig_path) {
	const char *path_buf = NULL;
	char regex[16] = {0};
	char buf0[1024] = {0};
	char original_path[1024] = {0};
	char dir_name[64] = {0};
	unsigned int i = 0;
	struct list *list_dirs = NULL;

	sprintf(regex, "%%*%ldc%%s", (strlen(storage_path) + 1));
	sscanf(file_path, regex, original_path);

	list_dirs = parceDirsList(dirs_list_str);

	for (i = 0; i < listGetSize(list_dirs); ++i)
	{
		path_buf = listGetItem(list_dirs, i);

		getNameFromPath(path_buf, dir_name);
		// Вытаскиваем из пути к файлу имя корневой директории
		sprintf(regex, "%%%ldc", strlen(dir_name));
		sscanf(original_path, regex, buf0);

		// Сравниваем имя копируемой и корневой директории
		if (strcmp(dir_name, buf0) == 0)
		{
			// Получаем путь к файлу с вырезанной корневой директорией
			sprintf(regex, "%%*%ldc%%s", (strlen(dir_name) + 1));
			sscanf(original_path, regex, buf0);
			hidePath(path_buf, original_path);
			sprintf(orig_path, "%s/%s", original_path, buf0);
			break;
		}
	}

	listFree(list_dirs);
}

int filesComeBack(const char *dirs_str, const char *storage_path, const char *watch_list_path) {
	FILE *file_watch_list = NULL;
	char buf0[1024] = {0};
	char file_path[1024] = {0};
	char original_path[1024] = {0};
	struct list *watch_list = NULL;
	struct list *files_list = NULL;
	struct list *dirs_list = NULL;
	unsigned int i = 0;
	int res = 0;
	unsigned int item_pos = 0;
	const char *temp_item0 = NULL;
	time_t temp_item_time0 = 0;
	time_t temp_item_time1 = 0;

	watch_list = listInit();
	files_list = getFilesListMod(storage_path);

	file_watch_list = fopen(watch_list_path, "r");

	if (file_watch_list == NULL)
	{
		perror("Fail");
		logInFile("Fail in open for read watch list\n");
		return -1;
	}

	while (fgets(buf0, 1024, file_watch_list) != NULL)
	{
		// Убираем знак новой строки
		buf0[strlen(buf0) - 1] = '\0';

		if (i == 0)
		{
			// Сохранения пути к файлу
			strcpy(file_path, buf0);
			i = 1;
		}
		else
		{
			i = 0;
			// Поиск идентичного пути в списке файлов
			item_pos = listGetItemPos(files_list, file_path, (strlen(file_path) + 1));

			if (item_pos != 0)
			{
				// Получение времени последнего изменения
				temp_item_time0 = atoll(buf0);
				// Получение времени последнего изменения
				temp_item_time1 = atoll(listGetItem(files_list, item_pos));

				if (temp_item_time0 < temp_item_time1)
				{
					getOriginalPath(dirs_str, storage_path, file_path, original_path);

					res = fileCopy(file_path, original_path);

					if (res != 0)
					{
						logInFile("Fail in copy %s -> %s\n", file_path, original_path);
					}
					else
					{
						logInFile("Copy %s -> %s\n", file_path, original_path);
					}
				}

				// Удаление времения последнего изменения
				files_list = listDeleteItem(files_list, item_pos);
				// Удаление пути к файлу
				files_list = listDeleteItem(files_list, item_pos - 1);
			}
			else
			{
				listAdd(watch_list, file_path, (strlen(file_path) + 1));
				listAdd(watch_list, buf0, (strlen(buf0) + 1));
			}
		}
	}

	fclose(file_watch_list);

	for (i = 0; i < listGetSize(watch_list); i += 2)
	{
		getOriginalPath(dirs_str, storage_path, listGetItem(watch_list, i), original_path);

		res = remove(original_path);

		if (res != 0)
		{
			logInFile("Fail to delete %s\n", original_path);
		}
		else
		{
			logInFile("Deleted %s\n", original_path);
		}
	}

	for (i = 0; i < listGetSize(files_list); i += 2)
	{
		temp_item0 = listGetItem(files_list, i);

		if (strcmp(temp_item0, watch_list_path) == 0)
		{
			files_list = listDeleteItem(files_list, i + 1);
			files_list = listDeleteItem(files_list, i);
			i -= 2;
		}
		else
		{
			getOriginalPath(dirs_str, storage_path, temp_item0, original_path);

			res = fileCopy(temp_item0, original_path);

			if (res != 0)
			{
				logInFile("Fail in copy new file %s -> %s\n", temp_item0, original_path);
			}
			else
			{
				logInFile("Copy new file %s -> %s\n", temp_item0, original_path);
			}
		}
	}

	listFree(watch_list);
	listFree(files_list);

	res = tmpfsUmount(storage_path);

	if (res	!= 0)
	{
		logInFile("Fail in umount %s\n", storage_path);
	}

	dirs_list = parceDirsList(dirs_str);

	for (i = 0; i < listGetSize(dirs_list); ++i)
	{
		temp_item0 = listGetItem(dirs_list, i);

		unlink(temp_item0);
		hidePath(temp_item0, buf0);

		res = rename(buf0, temp_item0);

		if (res != 0)
		{
			logInFile("Fail rename %s -> %s\n", buf0, temp_item0);
		}
		else
		{
			logInFile("Rename %s -> %s\n", buf0, temp_item0);
		}
	}

	listFree(dirs_list);
	return 0;
}

int start() {
	pid_t pid = 0;
	uid_t uid = 0;
	const char *pid_path = NULL;
	const char *ini_path = NULL;
	const char *partition_size = NULL;
	const char *partition_path = NULL;
	const char *dirs_list = NULL;
	const char *watch_list_path = NULL;
	const char *install_path = NULL;
	const char *shutdown_bin_path = NULL;
	unsigned char overload_shutdown_bin = 0;
	unsigned int res = 0;
	dictionary *ini = NULL;

	// Проверка на root-овость пользователя
	uid = getuid();

	if (uid != 0)
	{
		fprintf(stderr, "You is not root\n");
		return -4;
	}

	// Загрузка файла конфигураций
	if (fileExists(CONFIG_PATH))
	{
		ini_path = CONFIG_PATH;
	}
	else
	{
		ini_path = CONFIG_PATH_ALT;
	}

	ini = iniparser_load(ini_path);

	if (ini == NULL)
	{
		fprintf(stderr, "FAIL: Can't read config file!\n");
		return -2;
	}

	pid_path = iniparser_getstring(ini, "main:pid_path", "\0");
	partition_size = iniparser_getstring(ini, "main:partition_size", "\0");
	partition_path = iniparser_getstring(ini, "main:partition_path", "\0");
	dirs_list = iniparser_getstring(ini, "main:dirs_list", "\0");
	watch_list_path = iniparser_getstring(ini, "main:watch_list_path", "\0");
	overload_shutdown_bin = iniparser_getboolean(ini, "system:overload_shutdown_bin", 0);
	install_path = iniparser_getstring(ini, "system:install_path", NULL);
	shutdown_bin_path = iniparser_getstring(ini, "system:shutdown_bin_path", NULL);

	setLogFile(iniparser_getstring(ini, "main:log_file_path", "\0"));

	if (fileExists(pid_path))
	{
		fprintf(stderr, "Demon already running\n");
		iniparser_freedict(ini);
		return -1;
	}

	logMsg("PID path: %s\n", pid_path);
	logMsg("Partition size: %s\n", partition_size);
	logMsg("Partition path: %s\n", partition_path);
	logMsg("Directories list: %s\n", dirs_list);

	// Создание директории под монтирование, если она отсутствует
	if (fileExists(partition_path) == 0)
	{
		createDir(partition_path);
	}

	// Монтирование области
	res = tmpfsMount(partition_path, partition_size);

	if (res != 0)
	{
		perror("Fail in mount");
		iniparser_freedict(ini);
		return -3;
	}

	// Копирование всех необходимых папок в смонтированную область
	res = proccessDirs(partition_path, dirs_list, watch_list_path);

	if (res != 0)
	{
		fprintf(stderr, "Fail in copy dir\n");
		iniparser_freedict(ini);
		return -5;
	}

	// создаем потомка
	pid = fork();

	switch (pid) {
		case -1: {
			perror("Fail in fork");
			iniparser_freedict(ini);
			return -1;
		} break;

		// если это потомок
		case 0: {
			umask(0);
			setsid();
			chdir("/");

			pid = getpid();

			if (fileAddText(pid_path, "%d", pid) != 0)
			{
				fprintf(stderr, "FAIL: Can't write \n");
				iniparser_freedict(ini);
				return -1;
			}

			res = chmod(pid_path, (S_IFREG | S_IRWXU | S_IRGRP | S_IROTH));

			if (res != 0)
			{
				fprintf(stderr, "Can't change permission for %s\n", pid_path);
				remove(pid_path);
				iniparser_freedict(ini);
				return -2;
			}

			if (overload_shutdown_bin)
			{
				res = rename(shutdown_bin_path, PATH_SHUTDOWN_BACKUP);

				if (res != 0)
				{
					logMsg("Fail in rename shutdown bin!");
				}
				else
				{
					res = symlink(install_path, shutdown_bin_path);

					if (res != 0)
					{
						logMsg("Fail to symlink shutdown bin!");
					}
				}
			}

			// закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
			close(STDIN_FILENO);
			close(STDOUT_FILENO);
			close(STDERR_FILENO);

			signal(SIGTERM, &sighandler);
			signal(SIGUSR1, &sighandler);
			// signal(SIGUSR2, &sighandler);

			loop();
			filesComeBack(dirs_list, partition_path, watch_list_path);
			remove(pid_path);

			if (overload_shutdown_bin)
			{
				res = unlink(shutdown_bin_path);

				if (res != 0)
				{
					logMsg("Fail to unlink shutdown bin!");
				}
				else
				{
					res = rename(PATH_SHUTDOWN_BACKUP, shutdown_bin_path);

					if (res != 0)
					{
						logMsg("Fail in rename shutdown bin!");
					}
				}
			}

			if (want_shutdown)
			{
				startApplication(2, shutdown_bin_path, "now");
			}
		} break;
	}

	iniparser_freedict(ini);

	return 0;
}

int main(int argc, char const *argv[]) {
	unsigned int i  = 0;

	if (strstr(argv[0], "shutdown") != NULL)
	{
		return killDemon(1);
	}

	for (i = 0; i < argc; ++i)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1]) {
				case 's': {
					return start();
				} break;

				case 'h': {
					showHelp();
					return 0;
				} break;

				case 'k': {
					return killDemon(0);
				} break;

				case 'v': {
					showVersion();
					return 0;
				} break;

				default: {
					showHelp();
					return 0;
				}
			}
		}
	}

	showHelp();
	return 0;
}