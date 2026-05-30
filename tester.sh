rm -rf .vcs
rm -f file1.txt file2.txt file_sub.txt
rm -rf subfolder
make clean && make

echo "--- СТАРТ ПРОВЕРКИ ВСЕХ ФУНКЦИЙ ---"

# ==========================================
# 1. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 1: Вызовы до инициализации репозитория"

./vcs status
if [ $? -ne 1 ]; then echo "ОШИБКА: status до init должен вернуть 1"; exit 1; fi

./vcs add file1.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: add до init должен вернуть 1"; exit 1; fi

./vcs remove file1.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: remove до init должен вернуть 1"; exit 1; fi

./vcs commit "Test"
if [ $? -ne 1 ]; then echo "ОШИБКА: commit до init должен вернуть 1"; exit 1; fi

./vcs log
if [ $? -ne 1 ]; then echo "ОШИБКА: log до init должен вернуть 1"; exit 1; fi

./vcs diff 00000000
if [ $? -ne 1 ]; then echo "ОШИБКА: diff до init должен вернуть 1"; exit 1; fi

./vcs checkout 00000000 file1.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: checkout до init должен вернуть 1"; exit 1; fi


# ==========================================
# 2. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 2: Инициализация репозитория"

./vcs init
if [ $? -ne 0 ]; then echo "ОШИБКА: Первый запуск init должен вернуть 0"; exit 1; fi

./vcs init
if [ $? -ne 1 ]; then echo "ОШИБКА: Повторный запуск init должен вернуть 1"; exit 1; fi


# ==========================================
# 3. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 3: Статус чистого репозитория"

./vcs status
if [ $? -ne 0 ]; then echo "ОШИБКА: status при чистом репозитории должен вернуть 0"; exit 1; fi


# ==========================================
# 4. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 4: Попытка пустого коммита"

./vcs commit "Empty"
if [ $? -ne 1 ]; then echo "ОШИБКА: commit без файлов в индексе должен вернуть 1"; exit 1; fi


# ==========================================
# 5. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 5: Добавление некорректных файлов и путей"

./vcs add ../outside.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: add внешнего файла '../' должен вернуть 1"; exit 1; fi

./vcs add /etc/passwd
if [ $? -ne 1 ]; then echo "ОШИБКА: add абсолютного пути '/' должен вернуть 1"; exit 1; fi

./vcs add nonexistent.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: add несуществующего файла должен вернуть 1"; exit 1; fi


# ==========================================
# 6. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 6: Создание первого валидного коммита"

touch file1.txt
./vcs add file1.txt
if [ $? -ne 0 ]; then echo "ОШИБКА: Штатное добавление файла должно вернуть 0"; exit 1; fi

./vcs status
if [ $? -ne 0 ]; then echo "ОШИБКА: status с файлами в индексе должен вернуть 0"; exit 1; fi

./vcs commit "First real commit"
if [ $? -ne 0 ]; then echo "ОШИБКА: Создание нормального коммита должно вернуть 0"; exit 1; fi


# ==========================================
# 7. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 7: Работа с вложенными подпапками"

mkdir -p subfolder
touch subfolder/file_sub.txt
./vcs add subfolder/file_sub.txt
if [ $? -ne 0 ]; then echo "ОШИБКА: Добавление файла из вложенной папки должно вернуть 0"; exit 1; fi

./vcs commit "Subfolder commit"
if [ $? -ne 0 ]; then echo "ОШИБКА: Коммит вложенного файла должен вернуть 0"; exit 1; fi


# ==========================================
# 8. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 8: Удаление файлов через remove"

./vcs remove nonexistent_file.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: remove файла, которого нет нигде, должен вернуть 1"; exit 1; fi

touch file2.txt
./vcs remove file2.txt
if [ $? -ne 0 ]; then echo "ОШИБКА: remove неотслеживаемого файла должен вернуть 0"; exit 1; fi
rm -f file2.txt

./vcs remove file1.txt
if [ $? -ne 0 ]; then echo "ОШИБКА: remove отслеживаемого файла должен вернуть 0"; exit 1; fi


# ==========================================
# 9. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 9: Логирование истории и diff"

./vcs log
if [ $? -ne 0 ]; then echo "ОШИБКА: Вывод лога истории должен вернуть 0"; exit 1; fi

./vcs diff fakehash123
if [ $? -ne 1 ]; then echo "ОШИБКА: diff с левым хэшем должен вернуть 1"; exit 1; fi


# ==========================================
# 10. ТЕСТ
# ==========================================
echo -e "\n>>> Тест 10: Восстановление файлов через checkout"

./vcs checkout fakehash123 file1.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: checkout из фейкового коммита должен вернуть 1"; exit 1; fi

./vcs checkout 00000000 missing_in_root.txt
if [ $? -ne 1 ]; then echo "ОШИБКА: checkout файла, которого не было в том коммите, должен вернуть 1"; exit 1; fi


rm -f file1.txt file2.txt file_sub.txt
rm -rf subfolder
rm -rf .vcs

echo -e "\n--------------------------------------------------"
echo "УСПЕХ: Все функции протестированы."
echo "--------------------------------------------------"