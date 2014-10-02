#include "Analyzer.h"
#include <iostream>
#include <boost/regex.hpp>

void Analyzer::extractIncludesFromFile(FileInfo &fileInfo)
{
	vector<FileInfo> includes;
	fileInfo.exist = fs::exists(fileInfo.path);
	if (!fileInfo.exist) {
		dependencies.emplace(fileInfo, includes);
		return;
	}
	string read_line;
	ifstream fin(fileInfo.path.string());
	string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
	
	// remove comments
	boost::regex comment("(/\\*.*?\\*/)|(//.*?([\r\n]|$))");
	content = boost::regex_replace(content, comment, "");

	boost::regex regexInclude("#include\\s+([\"<])([^\r\n]+)[\">]");
	boost::smatch result;
	string::const_iterator itStart = content.begin();
	string::const_iterator itEnd = content.end();
	while (boost::regex_search(itStart, itEnd, result, regexInclude)) {
		FileInfo includeInfo;
		includeInfo.relativePath = result[2].str();
		boost::system::error_code ec;
		if (result[1].str() == "<") {
			fs::path targetDir;
			for (const fs::path &extInclude : externalIncludes)
				if (fs::exists(extInclude / includeInfo.relativePath)) {
					targetDir = extInclude;
					break;
				}
			includeInfo.exist = !targetDir.empty();
			includeInfo.path = fs::canonical(includeInfo.relativePath, targetDir, ec);
		} else {
			includeInfo.path = fs::canonical(includeInfo.relativePath, fileInfo.path.parent_path(), ec);
			includeInfo.exist = ec == boost::system::errc::success;
		}
		if (includeInfo.path.empty())
			includeInfo.path = includeInfo.relativePath;
		includes.push_back(includeInfo);
		itStart = result[2].second;
	}

	assert(dependencies.find(fileInfo) == dependencies.end());
	dependencies.emplace(fileInfo, includes);

	for (FileInfo &include : includes) {
		if (!counters.count(include)) {
			counters.emplace(include, 1);
			extractIncludesFromFile(include);
		} else
			counters[include]++;
	}
}

void Analyzer::setExternalIncludes(vector<string> const &paths)
{
	externalIncludes.clear();
	for (const string &path : paths)
	{
		fs::path includeDir(path);
		if (!fs::is_directory(path))
			cout << "Warning: '" << path << "' is not a directory!\n";
		else
			externalIncludes.push_back(path);
	}
}

void Analyzer::buildTreeAndFrequence() 
{
	reset();

	fs::path full_path = fs::system_complete(fs::path(sourcesPath));
	for (fs::recursive_directory_iterator end, dir(full_path); dir != end; ++dir)
		if (dir->path().filename().extension().string() == ".cpp") {
			FileInfo fInfo;
			fInfo.path = dir->path();
			extractIncludesFromFile(fInfo);
			cppFiles.push_back(fInfo);
		}
}

void Analyzer::printTree() const
{
	function<void(vector<FileInfo>)> print_recursive = [this, &print_recursive](vector<FileInfo> history) {
		int lvl = history.size();
		assert(lvl > 0);
		for (int i = 0; i < lvl - 1; i++)
			cout << "..";
		const FileInfo &fInfo = history.at(lvl - 1);
		cout << fInfo.path.filename().string() << (fInfo.exist ? "" : " (!)") << endl;
		const vector<FileInfo> &includes = dependencies.at(fInfo);
		history.push_back(FileInfo());
		for (const FileInfo &include : includes) {
			auto found_it = std::find(history.begin(), history.end(), include);
			if (found_it != history.end()) {
				cout << "Warning! Found cycle: ";
				for (auto last = history.end() - 1; found_it != last; ++found_it)
					cout << found_it->path.filename() << " -> ";
				cout << include.path.filename() << endl;
				continue;
			}
			history[lvl] = include;
			print_recursive(history);
		}
	};
	for (const FileInfo &cpp : cppFiles)
		print_recursive(vector<FileInfo> { cpp });
}

void Analyzer::printFrequence() const
{
	vector<FileInfo> sorted;
	for (auto &entry : counters)
		sorted.push_back(entry.first);
	sort(sorted.begin(), sorted.end(), [this](const FileInfo &a, const FileInfo &b) -> bool {
		int cnt_a = counters.at(a), cnt_b = counters.at(b);
		if (cnt_a > cnt_b)
			return true;
		if (cnt_a == cnt_b)
			return a.path.filename().string() < b.path.filename().string();
		return false;
	});

	for (FileInfo &fInfo : sorted)
		cout << fInfo.path.filename().string() << " " << counters.at(fInfo) << endl;
}

void Analyzer::reset()
{
	cppFiles.clear();
	dependencies.clear();
	counters.clear();
}