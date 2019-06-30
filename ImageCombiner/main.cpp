#include "pch.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <exception>
#include <iomanip>
#include "Image.h"
#include "Combiner.h"
#pragma warning(disable : 4996)


std::string currentDateTime()
{
	auto now = time(nullptr);
	struct tm tstruct{};
	char buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%d_%m_%y_%H_%M_%S", &tstruct);

	return buf;
}

// Extracts the order the images from the cameras shall be put in from a txt file containing the names of the Image folders in that order. The txt file must be named the same as the folder containing the Image folders. One Image folder per camera.
void findFolderOrder(std::filesystem::path file_name, std::vector<std::string>& folders_in_order)
{
	std::fstream txt_file;
	std::string line;
	txt_file.open(file_name /= file_name.filename() += ".txt", std::ios::in);

	if (txt_file.is_open())
	{
		while (std::getline(txt_file, line))
		{
			folders_in_order.push_back(line);
		}
	}
	txt_file.close();
}

void writeToLog(const std::string& message)
{
	const auto current_date_time = currentDateTime();
	std::fstream log_file("error_log.txt", std::ios::out | std::ios::app);
	if (log_file.is_open())
	{
		log_file << current_date_time << ' ' << message << '\n';
	}
	log_file.close();
}

int main(const int arg, char const* argv[])
{
	// Arguments: [1]Path to folder with subfolders for each cam, [2]number of images per cam, [3]number of rows in combined Image, [4]number of columns in combined Image
	if (arg != 5)
	{
		writeToLog("Incorrect number of arguments.");
		exit(EXIT_FAILURE);
	}
	//recordingDir.make_preferred();

	std::vector<std::string> folders_in_order;
	findFolderOrder(argv[1], folders_in_order);

	const auto number_of_cams = folders_in_order.size();

	auto combiner = Combiner{ argv[1], folders_in_order, std::stoi(argv[4]), std::stoi(argv[3]), number_of_cams, std::stoi(argv[2]) };
	combiner.initialize();
}

