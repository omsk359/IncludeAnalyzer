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
	string relativePath; // ���� � ����� ��������� � ��������� #include
	fs::path path; // ������ ���� � �����
	bool exist; // ���������� �� ����

	// ��������� �������� �� ������� ����, ���� �������� (���� ��� ����� �� ����������, fs::equivalent ������ ����������)
	bool operator==(const FileInfo &includeInfo) const {
		try {
			return fs::equivalent(includeInfo.path, path);
		} catch (fs::filesystem_error &) {
			return includeInfo.path == path;
		}
	}
};

// ���������� ��� �������� ��� FileInfo, ����� ������� ��� ���������� � unordered_map ��� �������� ������
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
	vector<fs::path> externalIncludes; // ���� ��� ������ #include <>
	string sourcesPath; // ���� � �������� � �����������, �������� � ��������� ������

	// �����������: ���� -> ���������� � ������, ��������� � #include �����-�����
	unordered_map<FileInfo, vector<FileInfo>> dependencies;
	// ������� ���-�� ��������� �����. ��� ���������� ����������� ����� ���������� ���� ���������, 
	// �.�. �� ������ ���������� �� �������� ������������� �������� ��� ��������
	unordered_map<FileInfo, int> counters;
	// ��� ��������� cpp �����
	vector<FileInfo> cppFiles;

	// ����������� ����� ���� ��������� ��������� �����. ���� ���� ��� �������������� �����, �� ����� ��������������.
	// � ������ �� ���������� �� ������������� �������, ������� ��� ���������� �������� ����������� ���������� ���������.
	void extractIncludesFromFile(struct FileInfo &);

public:
	// ������� �������� ������ ������������ ������ (��������� -I �������� ������)
	void setExternalIncludes(const vector<string> &);
	// ������� ����������
	void setSourcesPath(string path) { sourcesPath = path; }
	// �������� ����� ���������� ������ ��������� � �������� �� �������
	void buildTreeAndFrequence();
	// ����� � ������� ������ ���������
	void printTree() const;
	// ����� � ������� ������� ���������
	void printFrequence() const;
	// ������� ����������� ����������
	void reset();
};
