#include "pch.h"
#include "Combiner.h"

Combiner::Combiner(const std::string path, std::vector<std::string> camera_order, const int number_of_columns, const int number_of_rows, const int number_of_cams, const int images_per_cam):
	path(path), camera_order(std::move(camera_order)), number_of_columns_(number_of_columns),
	number_of_rows_(number_of_rows), number_of_cams_(number_of_cams), images_per_cam_(images_per_cam)
{
}

void Combiner::initialize()
{
	image_template_ = checkDimensions();

	combined_folder_path_ = path;
	combined_folder_path_ /= "combined";
	create_directory(combined_folder_path_);

	combineImages();
}

void Combiner::combineImages()
{
	// Currently the images must be named like this: 0.ppm, 1.ppm, 2.ppm etc. Change the image_file_name variable here to do it differently. Must be without the filetype suffix.
	for (auto image_file_name = 0; image_file_name < images_per_cam_; ++image_file_name)
	{
		combineImagesFromOneTimestamp(image_file_name);
	}
}

void Combiner::combineImagesFromOneTimestamp(const int image_file_name)
{
	const auto image_file_name_str = std::to_string(image_file_name);

	// Helper variable to determine the need for gray space in new combined image.
	const auto images_in_last_col = (number_of_columns_ - helper());

	auto cam_counter = 0;

	std::vector<std::vector<Image>> image_matrix;

	for (auto row_number = 0; row_number < number_of_rows_; ++row_number)
	{
		image_matrix.emplace_back();

		// Checks if the image is in the last row of the new combined image, and if gray space is needed.
		if (inLastRowWithEmptySpots(row_number, images_in_last_col))
		{
			for (auto pos_in_last_row = 0; pos_in_last_row < number_of_columns_; ++pos_in_last_row)
			{
				if (pos_in_last_row >= images_in_last_col)
				{
					Image empty_image(image_template_.width, image_template_.height, image_template_.max_val);
					empty_image.fillWithBlack();
					image_matrix[row_number].push_back(empty_image);
				}
				else
				{
					auto recording_dir = path;
					const auto image_file_path(recording_dir /=
						camera_order[cam_counter] + "\\" + image_file_name_str + "." + image_format_);

					cam_counter++;

					readImageAndExtractPixels(image_file_path, image_matrix[row_number]);
				}
			}
		}
		else
		{
			for (auto col_number = 0; col_number < number_of_columns_; ++col_number)
			{
				auto recording_dir = path;
				const auto image_file_path(recording_dir /=
					camera_order[cam_counter] + "\\" + image_file_name_str + image_format_);

				cam_counter++;

				readImageAndExtractPixels(image_file_path, image_matrix[row_number]);
			}
		}
	}
	createCombinedImage(image_matrix, image_file_name_str);
}


void Combiner::createCombinedImage(std::vector<std::vector<Image>>& image_matrix, const std::string& image_file_name) const
{
	std::fstream new_image_file;
	const auto s = image_template_.m_data_points_per_pixel * image_template_.width;
	auto new_file_path = combined_folder_path_;

	new_image_file.open(new_file_path /= image_file_name + "." + image_template_.image_format, std::ios::binary | std::ios::out);

	// Writes the headers for the .ppm or .pgm format. 
	new_image_file << image_template_.magic_number << "\n" << image_template_.width * number_of_columns_ << " " << image_template_.height * number_of_rows_ << "\n" << image_template_.max_val
		<< "\n";

	// Combines the pixels from each image into one. 
	// Goes through one row in the matrix at the time and writes one whole new row of pixels from left to right in that matrix row. 
	for (auto& k : image_matrix)
	{
		for (auto i = 0; i < image_template_.height; ++i)
		{
			// Writes one row of pixels from an image before going and doing the same to the right adjacent image.
			for (auto& j : k)
			{
				auto& pixel_rows = j.pixel_rows;
				new_image_file.write(pixel_rows[i].get(), s);
			}
		}
	}
	new_image_file.close();
	image_matrix.clear();
}

// Checks the first image in the first camera's image directory for image format and dimensions. Will use this data for the rest of the current combination process.
Image Combiner::checkDimensions() const
{
	std::fstream image_file;
	Image image;

	auto recording_dir = path;
	const auto image_file_path(recording_dir /=
		camera_order[0] + "\\0." + image_format_);

	image_file.open(image_file_path, std::ios::binary | std::ios::in);

	image.determineFormat(image_file);
	image.determineDimensionsAndMaxValue(image_file);

	image_file.close();

	return image;
}

void Combiner::readImageAndExtractPixels(const std::filesystem::path& image_file_path, std::vector<Image>& image_row)
{
	std::fstream image_file;
	Image image;

	image_file.open(image_file_path, std::ios::binary | std::ios::in);
	image.extractImageRows(image_file);
	image_file.close();

	image_row.push_back(image);
}

int Combiner::helper() const
{
	return (number_of_columns_ * number_of_rows_) - number_of_cams_;

}

bool Combiner::inLastRowWithEmptySpots(const int row_number, const int images_in_last_col) const
{
	return row_number == (number_of_rows_ - 1) && images_in_last_col != number_of_columns_;
}
