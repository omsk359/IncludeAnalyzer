#define BOOST_FILESYSTEM_NO_DEPRECATED

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
#include "boost/progress.hpp"

#include "boost/program_options.hpp"
#include <iostream>
#include "Analyzer.h"

namespace po = boost::program_options;
using namespace std;

int main(int argc, char* argv[])
{
	Analyzer analyzer;
	string sourcesPath;
	try {
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "help message")
			("sources-path", po::value<string>(), "path to sources")
			("include,I", po::value<vector<string> >()->composing(), "add include directory");

		po::positional_options_description p;
		p.add("sources-path", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
		po::notify(vm);

		if (vm.count("help") || !vm.count("sources-path")) {
			cout << desc << "\n";
			return 0;
		}

		sourcesPath = vm["sources-path"].as<string>();
		analyzer.setSourcesPath(sourcesPath);

		const vector<string> &includes = vm["include"].as<vector<string> >();
		analyzer.setExternalIncludes(includes);
	}
	catch (exception& e) {
		cerr << "error: " << e.what() << "\n";
		return 1;
	}

	analyzer.buildTreeAndFrequence();
	analyzer.printTree();
	cout << endl;
	analyzer.printFrequence();

//	system("pause");
	return 0;
}