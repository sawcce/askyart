#include <fstream>
#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>

#include <argparse/argparse.hpp>

using argparse::ArgumentParser;
using cv::Mat, cv::imread, cv::Vec3b;
using std::string, std::cout, std::cerr, std::fstream;

char color[] =
    "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,\"^`'.";

void processRgb(std::fstream &stream, Vec3b data);
void processGrayscale(std::fstream &stream, uchar data);
void processImage(std::fstream &stream, Mat image);

int main(int argc, char *argv[]) {
  ArgumentParser program("Askyart", "0.0.1");

  program.add_argument("file").help("the file to be processed");
  program.add_argument("destination").help("destination file");

  program.add_argument("--grayscale")
      .help("sets the image to grayscale")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--scale")
      .help("sets a scale factor for the image")
      .default_value(-1.0f)
      .scan<'g', float>();

  program.add_argument("--width")
      .help("resizes the image by width (preserves aspect ratio)")
      .default_value(-1)
      .scan<'i', int>();

  program.add_argument("--height")
      .help("resizes the image by height (preserves aspect ratio)")
      .default_value(-1)
      .scan<'i', int>();

  program.add_argument("--info")
      .help("prints out info of the image and does **not** process it")
      .default_value(false)
      .implicit_value(true);

  program.add_argument("--show")
      .help("if true it will show the generated ascii art in the terminal")
      .default_value(false)
      .implicit_value(true);

  try {
    program.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    std::exit(1);
  }

  Mat image;
  image =
      imread(program.get<std::string>("file"), program["--grayscale"] != true);

  if (image.channels() != 1 && image.channels() != 3) {
    printf("Cannot process image with different of channels than 1 or 3, %d\n",
           image.channels());
  }

  if (program["--info"] == true) {
    printf("Info, ignoring all other flags\n");

    printf("Image size: %dx%d\n", image.cols, image.rows);
    printf("Channels: %d", image.channels());
    return 0;
  }

  float newScale = program.get<float>("--scale");
  printf("ns: %f\n", newScale);

  float aspectWidth = (float)image.cols / (float)image.rows;
  float aspectHeight = (float)image.rows / (float)image.cols;
  int nHeight = program.get<int>("--height");
  int nWidth = program.get<int>("--width");

  // TODO: Documentation
  if (nHeight > 0) {
    cv::resize(image, image, cv::Size(nHeight * aspectWidth, nHeight));
  } else if (nWidth > 0) {
    cv::resize(image, image, cv::Size(nWidth, nWidth * aspectHeight));
  } else if (newScale < 0) {
    cv::resize(image, image, cv::Size(80, 80 * aspectWidth));
  } else {
    cv::resize(image, image, cv::Size(), newScale, newScale);
  }

  if (newScale != 1) {
    printf("New size: %dx%d\n", image.cols, image.rows);
  }

  if (!image.data) {
    cerr << "No image data!\n";
    return -1;
  }

  int width = image.cols;
  int height = image.rows;
  int len = width * height;

  string destination = program.get<std::string>("destination");

  std::fstream out;
  out.open(destination, fstream::in | fstream::out | fstream::trunc);

  if (!out.is_open()) {
    std::cerr << "Err: could not open file: \"" << destination << "\"\n";
    return -1;
  }

  std::cout << program.get<std::string>("destination") << "\n";

  if (image.channels() == 3) {
    for (int y = 0; y < image.rows; y++) {
      for (int x = 0; x < image.cols; x++) {
        auto colors = image.at<cv::Vec3b>(y, x);
        processRgb(out, colors);
      }
      out << "\n";
    }
  } else {
    for (int y = 0; y < image.rows; y++) {
      for (int x = 0; x < image.cols; x++) {
        uchar pixel = image.at<uchar>(y, x);
        processGrayscale(out, pixel);
      }
      out << "\n";
    }
  }

  if (program["--show"] == true) {
    cout << "Result:\n";
    out.seekp(0);
    std::cout << out.rdbuf();
  }

  out.close();

  return 0;
}

// Maps a color value (0 to 255) to an index in the "color" array (0 to 69)
int computeIndex(float value) { return (value / 255.0) * 69; }

void processRgb(std::fstream &stream, Vec3b data) {
  // We retrieve the red, green and blue values from the data provided by opencv
  int r = data[0], g = data[1], b = data[2];
  // Computes the average of the three color channels (for brightness)
  float average = (r + g + b) / 3.0;

  char character = color[computeIndex(average)];

  stream << "\x1b[38;2;" << r << ";" << g << ";" << b << "m" << character;
}

void processGrayscale(std::fstream &stream, uchar data) {
  stream << color[computeIndex(data)];
}
