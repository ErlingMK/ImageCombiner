#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include "Image.h"

class Combiner
{
public:
	Combiner(std::string path, std::vector<std::string> camera_order, int number_of_columns, int number_of_rows, int number_of_cams, int images_per_cam);

	void initialize();

	std::filesystem::path path{};
	std::vector<std::string> camera_order;

private:
	Image checkDimensions() const;
	static void readImageAndExtractPixels(const std::filesystem::path& image_file_path, std::vector<Image>& image_row);
	void createCombinedImage(std::vector<std::vector<Image>>& image_matrix, const int image_file_name) const;
	void combineImages();
	void combineImagesFromOneTimestamp(const int image_file_name);

	int helper() const;
	bool inLastRowWithEmptySpots(const int row_number, const int images_in_last_col) const;

	int number_of_columns_;
	int number_of_rows_;
	int number_of_cams_;
	int images_per_cam_;

	std::filesystem::path combined_folder_path_{};

	Image image_prototype_;

	std::string image_format_;
	std::string magic_number_;
};

