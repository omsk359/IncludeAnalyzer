#pragma once
#include <iostream>
#include <vector>
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include <unordered_map>

using namespace std;
namespace fs = boost::filesystem;

struct FileInfo
{
	string relativePath; // путь к файлу указанный в директиве #include
	fs::path path; // полный путь к файлу
	bool exist; // существует ли файл

	// сравнение объектов по полному пути, если возможно (если оба файла не существуют, fs::equivalent кидает исключение)
	bool operator==(const FileInfo &includeInfo) const {
		try {
			return fs::equivalent(includeInfo.path, path);
		} catch (fs::filesystem_error &) {
			return includeInfo.path == path;
		}
	}
};

// вычисление хеш значения для FileInfo, чтобы хранить эту информацию в unordered_map для быстрого поиска
template <>
struct hash<FileInfo>
{
	size_t operator()(const FileInfo &fInfo) const {
		std::hash<string> hasher;
		return hasher(fInfo.path.string());
	}
};

class Analyzer
{
	vector<fs::path> externalIncludes; // пути для поиска #include <>
	string sourcesPath; // путь к каталогу с исходниками, задается в командной строке

	// отображение: Файл -> информафия о файлах, указанный в #include файла-ключа
	unordered_map<FileInfo, vector<FileInfo>> dependencies;
	// Подсчет кол-ва включений фалов. Эта информация сортируется после добавления всех элементов, 
	// т.к. на момент добавления не известно окончательное значение для элемента
	unordered_map<FileInfo, int> counters;
	// все найденные cpp файлы
	vector<FileInfo> cppFiles;

	// Рекурсивный обход всех включений заданного файла. Если файл уже просматривался ранее, он будет проигнорирован.
	// В задаче не говорилось об эффективности решения, поэтому для извлечения инклюдов применяются регулярные выражения.
	void extractIncludesFromFile(struct FileInfo &);

public:
	// внешние каталоги поиска заголовочных файлов (параметры -I командой строки)
	void setExternalIncludes(const vector<string> &);
	// каталог исходников
	void setSourcesPath(string path) { sourcesPath = path; }
	// основной метод построения дерева включений и подсчета их частоты
	void buildTreeAndFrequence();
	// вывод в консоль дерева включений
	void printTree() const;
	// вывод в консоль частоты включений
	void printFrequence() const;
	// очистка предыдущего построения
	void reset();
};
