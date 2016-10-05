/**
 * @file april_tags.cpp
 * @brief Example application for April tags library
 * @author: Michael Kaess
 *
 * Opens the first available camera (typically a built in camera in a
 * laptop) and continuously detects April tags in the incoming
 * images. Detections are both visualized in the live image and shown
 * in the text console. Optionally allows selecting of a specific
 * camera in case multiple ones are present and specifying image
 * resolution as long as supported by the camera. Also includes the
 * option to send tag detections via a serial port, for example when
 * running on a Raspberry Pi that is connected to an Arduino.
 */

using namespace std;

#include <iostream>
#include <cstring>
#include <vector>
#include <sys/time.h>
#include <math.h>
#include <fstream>

const string usage = "\n"
  "Usage:\n"
  "  apriltags_demo [OPTION...] [deviceID]\n"
  "\n"
  "Options:\n"
  "  -h  -?          Show help options\n"
  "  -a              Arduino (send tag ids over serial port)\n"
  "  -d              disable graphics\n"
  "  -C <bbxhh>      Tag family (default 36h11)\n"
  "  -F <fx>         Focal length in pixels\n"
  "  -W <width>      Image width (default 640, availability depends on camera)\n"
  "  -H <height>     Image height (default 480, availability depends on camera)\n"
  "  -S <size>       Tag size (square black frame) in meters\n"
  "  -E <exposure>   Manually set camera exposure (default auto; range 0-10000)\n"
  "  -G <gain>       Manually set camera gain (default auto; range 0-255)\n"
  "  -B <brightness> Manually set the camera brightness (default 128; range 0-255)\n"
  "\n";

const string intro = "\n"
    "April tags test code\n"
    "(C) 2012-2013 Massachusetts Institute of Technology\n"
    "Michael Kaess\n"
    "\n";


#ifndef __APPLE__
#define EXPOSURE_CONTROL // only works in Linux
#endif

#ifdef EXPOSURE_CONTROL
#include <libv4l2.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <errno.h>
#endif

// OpenCV library for easy access to USB camera and drawing of images
// on screen
#include "opencv2/opencv.hpp"
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/core/core.hpp>

// April tags detector and various families that can be selected by command line option
#include "AprilTags/TagDetector.h"
#include "AprilTags/Tag16h5.h"
#include "AprilTags/Tag25h7.h"
#include "AprilTags/Tag25h9.h"
#include "AprilTags/Tag36h9.h"
#include "AprilTags/Tag36h11.h"
extern "C" {
#include "AprilTags/djicam.h"
}
static int mode = 2;
#define FRAME_SIZE              (1280*720*3/2)  /*format NV12*/
static unsigned char buffer[FRAME_SIZE+8] = {0};
static unsigned int nframe = 0;
#define BLOCK_MODE                     1



// Needed for getopt / command line options processing
#include <unistd.h>
extern int optind;
extern char *optarg;

// For Arduino: locally defined serial port access class
#include "Serial.h"
typedef unsigned char  BYTE; 


const char* window_name = "apriltags_demo";


// utility function to provide current system time (used below in
// determining frame rate at which images are being processed)
double tic() {
  struct timeval t;
  gettimeofday(&t, NULL);
  return ((double)t.tv_sec + ((double)t.tv_usec)/1000000.);
}


#include <cmath>

#ifndef PI
const double PI = 3.14159265358979323846;
#endif
const double TWOPI = 2.0*PI;

/**
 * Normalize angle to be within the interval [-pi,pi].
 */
inline double standardRad(double t) {
  if (t >= 0.) {
    t = fmod(t+PI, TWOPI) - PI;
  } else {
    t = fmod(t-PI, -TWOPI) + PI;
  }
  return t;
}

void wRo_to_euler(const Eigen::Matrix3d& wRo, double& yaw, double& pitch, double& roll) {
    yaw = standardRad(atan2(wRo(1,0), wRo(0,0)));
    double c = cos(yaw);
    double s = sin(yaw);
    pitch = standardRad(atan2(-wRo(2,0), wRo(0,0)*c + wRo(1,0)*s));
    roll  = standardRad(atan2(wRo(0,2)*s - wRo(1,2)*c, -wRo(0,1)*s + wRo(1,1)*c));
  }


bool YV12ToBGR24_Native(unsigned char* pYUV,unsigned char* pBGR24,int width,int height)
{
    if (width < 1 || height < 1 || pYUV == NULL || pBGR24 == NULL)
        return false;
    const long len = width * height;
    unsigned char* yData = pYUV;
    unsigned char* uData = &yData[len];
    unsigned char* vData = &yData[len+1];

    int bgr[3];
    int yIdx,uIdx,vIdx,idx;
    for (int i = 0;i < height;i++){
        for (int j = 0;j < width;j++){
            yIdx = i * width + j;
            uIdx = ((i/2) * (width/2) + (j/2))*2;
            vIdx = uIdx;
        
            bgr[0] = (int)(yData[yIdx] + 1.732446 * (uData[vIdx] - 128));                                    // b·ÖÁ¿
            bgr[1] = (int)(yData[yIdx] - 0.698001 * (uData[uIdx] - 128) - 0.703125 * (vData[vIdx] - 128));    // g·ÖÁ¿
            bgr[2] = (int)(yData[yIdx] + 1.370705 * (vData[uIdx] - 128));                                    // r·ÖÁ¿

            for (int k = 0;k < 3;k++){
                idx = (i * width + j) * 3 + k;
                if(bgr[k] >= 0 && bgr[k] <= 255)
                    pBGR24[idx] = bgr[k];
                else
                    pBGR24[idx] = (bgr[k] < 0)?0:255;
            }
        }
    }
    return true;
}



IplImage* YUV420_To_IplImage(unsigned char* pYUV420, int width, int height)
{
    if (!pYUV420)
    {
		return NULL;
    }

    //³õÊŒ»¯±äÁ¿
    int baseSize = width*height;
    int imgSize = baseSize*3;
	BYTE* pRGB24  = new BYTE[imgSize];
	memset(pRGB24,  0, imgSize);

    /* ±äÁ¿ÉùÃ÷ */
    int temp = 0;


    if(YV12ToBGR24_Native(pYUV420, pRGB24, width, height) == false || !pRGB24)
    {
		return NULL;
	}

    IplImage *image = cvCreateImage(cvSize(width, height), 8,3);
    memcpy(image->imageData, pRGB24, imgSize);

    if (!image)
    {
        return NULL;
    }

    delete [] pRGB24;
    return image;
}



class Demo {

  AprilTags::TagDetector* m_tagDetector;
  AprilTags::TagCodes m_tagCodes;

  bool m_draw; // draw image and April tag detections?
  bool m_arduino; // send tag detections to serial port?

  int m_width; // image size in pixels
  int m_height;
  double m_tagSize; // April tag side length in meters of square black frame
  double m_fx; // camera focal length in pixels
  double m_fy;
  double m_px; // camera principal point
  double m_py;

  int m_deviceId; // camera id (in case of multiple cameras)
  cv::VideoCapture m_cap;

  int m_exposure;
  int m_gain;
  int m_brightness;

  Serial m_serial;

public:

  // default constructor
  Demo() :
    // default settings, most can be modified through command line options (see below)
    m_tagDetector(NULL),
    m_tagCodes(AprilTags::tagCodes36h11),

    m_draw(true),
    m_arduino(false),

    m_width(1280),
    m_height(720),
    m_tagSize(0.166),
    m_fx(600),
    m_fy(600),
    m_px(m_width/2),
    m_py(m_height/2),

    m_exposure(-1),
    m_gain(-1),
    m_brightness(-1),

    m_deviceId(0)
  {}

  // changing the tag family
  void setTagCodes(string s) {
    if (s=="16h5") {
      m_tagCodes = AprilTags::tagCodes16h5;
    } else if (s=="25h7") {
      m_tagCodes = AprilTags::tagCodes25h7;
    } else if (s=="25h9") {
      m_tagCodes = AprilTags::tagCodes25h9;
    } else if (s=="36h9") {
      m_tagCodes = AprilTags::tagCodes36h9;
    } else if (s=="36h11") {
      m_tagCodes = AprilTags::tagCodes36h11;
    } else {
      cout << "Invalid tag family specified" << endl;
      exit(1);
    }
  }

  // parse command line options to change default behavior
  void parseOptions(int argc, char* argv[]) {
    int c;
    while ((c = getopt(argc, argv, ":h?adC:F:H:S:W:E:G:B:")) != -1) {
      // Each option character has to be in the string in getopt();
      // the first colon changes the error character from '?' to ':';
      // a colon after an option means that there is an extra
      // parameter to this option; 'W' is a reserved character
      switch (c) {
      case 'h':
      case '?':
        cout << intro;
        cout << usage;
        exit(0);
        break;
      case 'a':
        m_arduino = true;
        break;
      case 'd':
        m_draw = false;
        break;
      case 'C':
        setTagCodes(optarg);
        break;
      case 'F':
        m_fx = atof(optarg);
        m_fy = m_fx;
        break;
      case 'H':
        m_height = atoi(optarg);
        m_py = m_height/2;
         break;
      case 'S':
        m_tagSize = atof(optarg);
        break;
      case 'W':
        m_width = atoi(optarg);
        m_px = m_width/2;
        break;
      case 'E':
#ifndef EXPOSURE_CONTROL
        cout << "Error: Exposure option (-E) not available" << endl;
        exit(1);
#endif
        m_exposure = atoi(optarg);
        break;
      case 'G':
#ifndef EXPOSURE_CONTROL
        cout << "Error: Gain option (-G) not available" << endl;
        exit(1);
#endif
        m_gain = atoi(optarg);
        break;
      case 'B':
#ifndef EXPOSURE_CONTROL
        cout << "Error: Brightness option (-B) not available" << endl;
        exit(1);
#endif
        m_brightness = atoi(optarg);
        break;
      case ':': // unknown option, from getopt
        cout << intro;
        cout << usage;
        exit(1);
        break;
      }
    }

    if (argc == optind + 1) {
      m_deviceId = atoi(argv[optind]);
    }
  }

  void setup() {
    m_tagDetector = new AprilTags::TagDetector(m_tagCodes);
    //m_tagDetector2 = new AprilTags::TagDetector(m_tagCodes);

    // find and open a USB camera (built in laptop camera, web cam etc)
    //m_cap = cv::VideoCapture(m_deviceId);
     //   if(!m_cap.isOpened()) {
    //  cerr << "ERROR: Can't find video device " << m_deviceId << "\n";
    //  exit(1);
    //}
    //m_cap.set(CV_CAP_PROP_FRAME_WIDTH, m_width);
    //m_cap.set(CV_CAP_PROP_FRAME_HEIGHT, m_height);
    //cout << "Camera successfully opened (ignore error messages above...)" << endl;
    //cout << "Actual resolution: "
        // << m_cap.get(CV_CAP_PROP_FRAME_WIDTH) << "x"
        // << m_cap.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;

    // prepare window for drawing the camera images
    if (m_draw) {
      cv::namedWindow(window_name, 1);
    }

  
  }

  void print_detection(AprilTags::TagDetection& detection) const {
    cout << "  Id: " << detection.id
         << " (Hamming: " << detection.hammingDistance << ")";

    // recovering the relative pose of a tag:

    // NOTE: for this to be accurate, it is necessary to use the
    // actual camera parameters here as well as the actual tag size
    // (m_fx, m_fy, m_px, m_py, m_tagSize)

    Eigen::Vector3d translation;
    Eigen::Matrix3d rotation;
    detection.getRelativeTranslationRotation(m_tagSize, m_fx, m_fy, m_px, m_py,
                                             translation, rotation);

    Eigen::Matrix3d F;
    F <<
      1, 0,  0,
      0,  -1,  0,
      0,  0,  1;
    Eigen::Matrix3d fixed_rot = F*rotation;
    double yaw, pitch, roll;
    wRo_to_euler(fixed_rot, yaw, pitch, roll);

    cout << "  distance=" << translation.norm()
         << "m, x=" << translation(0)
         << ", y=" << translation(1)
         << ", z=" << translation(2)
         << ", yaw=" << yaw
         << ", pitch=" << pitch
         << ", roll=" << roll
         << endl;

    // Also note that for SLAM/multi-view application it is better to
    // use reprojection error of corner points, because the noise in
    // this relative pose is very non-Gaussian; see iSAM source code
    // for suitable factors.
  }

  // The processing loop where images are retrieved, tags detected,
  // and information about detections generated
  void loop() {

    cv::Mat image;
    cv::Mat image_gray;

    int frame = 0;
    double last_t = tic();
    while (true) {

      // capture frame

       int ret;

	 //²úÉúËæ»úÊý ŽæÈ¡ŸÅž±ÍŒÆ¬
     char frame_image[30];
     int date_begin,date_end;
     int flag=1;
     srand(time(0));
     date_begin=rand()%100;
     date_end =rand()%100;

   
        if(mode)
        {
#if BLOCK_MODE
            ret = manifold_cam_read(buffer, &nframe, CAM_BLOCK); /*blocking read*/
            if(ret < 0)
            {
                printf("manifold_cam_read error \n");
                break;
            }         
           
                 //sprintf(frame_image,"%d%d%d",date_begin,date_end,flag);
                // strcat(frame_image,".jpg");
             //    printf("nframe is %d\n",nframe);
			     IplImage *rgbimg = YUV420_To_IplImage(buffer, 1280, 720);//1280, 720);  //YUV×ªRGB
                             image = cv::Mat(rgbimg);
 
                 //cvSaveImage(frame_image,rgbimg); //±£ŽæÍŒÆ¬
                 flag++; 
           


#else
            ret = manifold_cam_read(buffer, &nframe, CAM_NON_BLOCK); /*non_blocking read*/
            if(ret < 0)
            {
                printf("manifold_cam_read error \n");
                break;
            }
                         printf("nframe is %d\n",nframe);
                         printf("fu xiong \n");
#endif
        }

        usleep(500);
    

    printf("get_images_loop thread exit! \n");
     // m_cap >> image;

      // alternative way is to grab, then retrieve; allows for
      // multiple grab when processing below frame rate - v4l keeps a
      // number of frames buffered, which can lead to significant lag
      //      m_cap.grab();
      //      m_cap.retrieve(image);

      // detect April tags (requires a gray scale image)
      cv::cvtColor(image, image_gray, CV_BGR2GRAY);
      vector<AprilTags::TagDetection> detections = m_tagDetector->extractTags(image_gray);

      // print out each detection
      cout << detections.size() << " tags detected:" << endl;
      for (int i=0; i<detections.size(); i++) {
        print_detection(detections[i]);
      }

      // show the current image including any detections
      if (m_draw) {
        for (int i=0; i<detections.size(); i++) {
          // also highlight in the image
          detections[i].draw(image);
        }
        imshow(window_name, image); // OpenCV call
      }

      // optionally send tag information to serial port (e.g. to Arduino)

      // print out the frame rate at which image frames are being processed
      frame++;
      if (frame % 10 == 0) {
        double t = tic();
        cout << "  " << 10./(t-last_t) << " fps" << endl;
        last_t = t;
      }

      // exit if any key is pressed
      if (cv::waitKey(1) >= 0) break;
    }
  }


}; // Demo


// here is were everything begins
int main(int argc, char* argv[]) {
  Demo demo;
  int ret;
printf("Entered Main");

  // process command line options
  demo.parseOptions(argc, argv);
printf("Finish Parse options");
 ret = manifold_cam_init(2);
    if(-1 == ret)
    {
        printf("manifold init error \n");
        return -1;
    }

  // setup image source, window for drawing, serial port...
  demo.setup();

printf("Finish Setup");

  // the actual processing loop where tags are detected and visualized
  demo.loop();

  return 0;
}
