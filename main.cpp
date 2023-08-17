/*
 * The Example of the Withrobot oCam-1MGN-U API using with OpenCV. (Linux only.)
 *
 * This example shows how to get image from the oCam-1MGN using the Withrobot
 * camera API. And also shows how to control the oCam-1MGN using the Withrobot
 * camera API.
 *
 * This example program usage.:
 * 	- Press the key 'q' for exit this program.
 *  - Press the key ']' for increase the exposure.
 *  - Press the key '[' for decrease the exposure.
 *  - Press the key '=' for increase the brightness.
 *  - Press the key '-' for decrease the brightness.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "opencv2/opencv.hpp"
#include "withrobot_camera.hpp" /* withrobot camera API */

#define PORT 9999

/*
 *	Main
 */
int main(int argc, char* argv[]) {
  /*
   * initialize oCam-1CGN
   *
   * [ supported image formats ]
   *
   * USB 3.0
   * 	[0] "8-bit Greyscale 1280 x 720 60 fps"
   *	[1] "8-bit Greyscale 1280 x 960 45 fps"
   *	[2] "8-bit Greyscale 320 x 240 160 fps"
   * 	[3] "8-bit Greyscale 640 x 480 80 fps"
   *
   * USB 2.0
   * 	[0] "8-bit Greyscale 1280 x 720 30 fps"
   *	[1] "8-bit Greyscale 1280 x 960 22.5 fps"
   *	[2] "8-bit Greyscale 320 x 240 160 fps"
   * 	[3] "8-bit Greyscale 640 x 480 80 fps"
   *
   */
  const char* devPath = "/dev/ocam";

  Withrobot::Camera camera(devPath);

  /* USB 3.0 */
  /* bayer RBG 1280 x 720 60 fps */
  // camera.set_format(1280, 720,
  // Withrobot::fourcc_to_pixformat('G','B','G','R'), 1, 60);

  /* bayer RBG 1280 x 960 45 fps */
  // camera.set_format(1280, 960,
  // Withrobot::fourcc_to_pixformat('G','B','G','R')), 1, 45);

  /* bayer RBG 320 x 240 160 fps */
  // camera.set_format(320, 240,
  // Withrobot::fourcc_to_pixformat('G','B','G','R'), 1, 160);

  /* bayer RBG 640 x 480 80 fps */
  camera.set_format(640, 480,
                    Withrobot::fourcc_to_pixformat('G', 'B', 'G', 'R'), 1, 80);

  /* USB 2.0 */
  /* bayer RBG 1280 x 720 30 fps */
  // camera.set_format(1280, 720,
  // Withrobot::fourcc_to_pixformat(''G','B','G','R'), 1, 30);

  /* bayer RBG 1280 x 960 22.5 fps */
  // camera.set_format(1280, 960,
  // Withrobot::fourcc_to_pixformat(''G','B','G','R'), 2, 45);

  /* bayer RBG 320 x 240 160 fps */
  // camera.set_format(320, 240,
  // Withrobot::fourcc_to_pixformat(''G','B','G','R'), 1, 160);

  /* bayer RBG 640 x 480 80 fps */
  // camera.set_format(640, 480,
  // Withrobot::fourcc_to_pixformat(''G','B','G','R'), 1, 80);

  /*
   * get current camera format (image size and frame rate)
   */
  Withrobot::camera_format camFormat;
  camera.get_current_format(camFormat);

  /*
   * Print infomations
   */
  std::string camName = camera.get_dev_name();
  std::string camSerialNumber = camera.get_serial_number();

  printf("dev: %s, serial number: %s\n", camName.c_str(),
         camSerialNumber.c_str());
  printf("----------------- Current format informations -----------------\n");
  camFormat.print();
  printf("---------------------------------------------------------------\n");

  /*
   * [ supported camera controls; The double quotes are the 'get_control' and
   * the 'set_control' function string argument values. ]
   *
   *  [0] "Gain",          Value(default [min, step, max]): 64 ( 64 [0, 1, 127]
   * ) [1] "Exposure (Absolute)", Value(default [min, step, max]): 39 ( 39 [1,
   * 1, 625] )
   *
   */
  int brightness = camera.get_control("Gain");
  int exposure = camera.get_control("Exposure (Absolute)");

  camera.set_control("Gain", brightness);
  camera.set_control("Exposure (Absolute)", exposure);

  /*
   * Start streaming
   */
  if (!camera.start()) {
    perror("Failed to start.");
    exit(0);
  }

  /*
   * Initialize OpenCV
   */
  std::string windowName = camName + " " + camSerialNumber;
  cv::Mat srcImg(cv::Size(camFormat.width, camFormat.height), CV_8UC1);
  cv::Mat colorImg(cv::Size(camFormat.width, camFormat.height), CV_8UC3);
  // cv::namedWindow(windowName.c_str(),
  // CV_WINDOW_KEEPRATIO|CV_WINDOW_AUTOSIZE);

  // udp set-------------------

  int client_socket;
  struct sockaddr_in serverAddress;
  int server_addr_size;

  ssize_t receivedBytes;
  ssize_t sentBytes;

  memset(&serverAddress, 0, sizeof(serverAddress));

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr("192.168.1.151");
  serverAddress.sin_port = htons(PORT);

  // 소켓 생성
  if ((client_socket = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
    printf("socket 생성 실패\n");
    exit(0);
  }

  //-------------------------------------

  /*
   * Main loop
   */
  bool quit = false;
  while (!quit) {
    /* Copy a single frame(image) from camera(oCam-1MGN). This is a blocking
     * function. */
    int size = camera.get_frame(srcImg.data, camFormat.image_size, 1);

    /* If the error occured, restart the camera. */
    if (size == -1) {
      printf("error number: %d\n", errno);
      perror("Cannot get image from camera");
      camera.stop();
      camera.start();
      continue;
    }

    cv::cvtColor(srcImg, colorImg, cv::COLOR_BayerGB2BGR);
    /* Show image */
    //cv::imshow(windowName.c_str(), colorImg);

    //(1) jpeg compression
    std::vector<uchar> buff;  // buffer for coding
    std::vector<int> param(2);

    param[0] = cv::IMWRITE_JPEG_QUALITY;
    param[1] = 90;  // default(95) 0-100

    cv::imencode(".jpg", colorImg, buff, param);
    std::cout<<buff.size()<<std::endl;
    while ((sentBytes = sendto(client_socket, (const char *)buff.data(), buff.size(), 0,
                              (struct sockaddr*)&serverAddress,
                              sizeof(serverAddress)) )== -1)
      ;
    std::cout << sentBytes << " : send\n";
    char key = cv::waitKey(33);

    /* Keyboard options */
    switch (key) {
      /* When press the 'q' key then quit. */
      case 'q':
        quit = true;
        break;

      /* When press the '[' key then decrease the exposure time. */
      case '[':
        exposure = camera.get_control("Exposure (Absolute)");
        camera.set_control("Exposure (Absolute)", --exposure);
        break;

        /* When press the ']' key then increase the exposure time. */
      case ']':
        exposure = camera.get_control("Exposure (Absolute)");
        camera.set_control("Exposure (Absolute)", ++exposure);
        break;

        /* When press the '-' key then decrease the brightness. */
      case '-':
        exposure = camera.get_control("Gain");
        camera.set_control("Gain", --brightness);
        break;

        /* When press the '=' key then increase the brightness. */
      case '=':
        exposure = camera.get_control("Gain");
        camera.set_control("Gain", ++brightness);
        break;

      default:
        break;
    }
  }

  cv::destroyAllWindows();

  /*
   * Stop streaming
   */
  camera.stop();

  // 소켓 close
  close(client_socket);

  printf("Done.\n");

  return 0;
}
