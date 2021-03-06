#include "ardrone/ardrone.h"
#include "zbar.h"

using namespace std;
using namespace cv;
// --------------------------------------------------------------------------
// main(Number of arguments, Argument values)
// Description  : This is the entry point of the program.
// Return value : SUCCESS:0  ERROR:-1
// --------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    // AR.Drone class	
    ARDrone ardrone;

    // Initialize
    if (!ardrone.open()) {
        std::cout << "Failed to initialize." << std::endl;
        return -1;
    }

    // Battery
    std::cout << "Battery = " << ardrone.getBatteryPercentage() << "[%]" << std::endl;

	zbar::ImageScanner scanner;
	scanner.set_config(zbar::ZBAR_NONE, zbar::ZBAR_CFG_ENABLE, 1);
    
	// Main loop
	while (1) {
		// Key input
		int key = cv::waitKey(33);
		if (key == 0x1b) break;

		// Get an image
		cv::Mat image = ardrone.getImage();
		//cout << image.channels() << " channels " << endl;
		cvtColor(image, image, CV_RGB2GRAY);
		//cout << image.channels() << " channels efter RGB2GRAY" << endl;
		cv::Mat imgout;
		//cout << image.cols << "image ardrone cols" << endl;
		cvtColor(image, imgout, CV_GRAY2RGB);
		//cvtColor(image, imgout);


		// Take off / Landing 
		if (key == ' ') {
			if (ardrone.onGround()) ardrone.takeoff();
			else                    ardrone.landing();
		}

		// Move
		double x = 0.0, y = 0.0, z = 0.0, r = 0.0;
		if (key == 'i' || key == CV_VK_UP)    x = 1.0;
		if (key == 'k' || key == CV_VK_DOWN)  x = -1.0;
		if (key == 'u' || key == CV_VK_LEFT)  r = 1.0;
		if (key == 'o' || key == CV_VK_RIGHT) r = -1.0;
		if (key == 'j') y = 1.0;
		if (key == 'l') y = -1.0;
		if (key == 'q') z = 1.0;
		if (key == 'a') z = -1.0;
		ardrone.move3D(x, y, z, r);

        // Change camera
        static int mode = 0;
        if (key == 'c') ardrone.setCamera(++mode%4);

		//cv::cvtColor(image, image, CV_GRAY2RGB);
		//cv::cvtColor(image, image, CV_RGB2GRAY);
		int width = image.cols;
		int height = image.rows;
		uchar *raw = (uchar *) image.data;
		// wrap image data  
		zbar::Image imageQR(width, height, "Y800", raw, width * height);
		// scan the image for barcodes  
		int n = scanner.scan(imageQR);
		// extract results  
		for (zbar::Image::SymbolIterator symbol = imageQR.symbol_begin();
			symbol != imageQR.symbol_end();
			++symbol) {
			cout << "Inde i for loop" << endl;
			vector<Point> vp;
			// do something useful with results  
			cout << "decoded " << symbol->get_type_name()
				<< " symbol \"" << symbol->get_data() << '"' << " " << endl;
			int n = symbol->get_location_size();
			for (int i = 0; i<n; i++) {
				vp.push_back(Point(symbol->get_location_x(i), symbol->get_location_y(i)));
			}
			RotatedRect r = minAreaRect(vp);
			Point2f pts[4];
			r.points(pts);
			for (int i = 0; i<4; i++) {
				line(imgout, pts[i], pts[(i + 1) % 4], Scalar(255, 0, 0), 3);
			}
			cout << "Angle: " << r.angle << endl;
		}
		//if(imgout)
		imshow("imgout.jpg", imgout);
		//cout << imgout.cols << " cols" << imgout.rows << " rows" << endl;
		// clean up  
		imageQR.set_data(NULL, 0);
		//waitKey(); Med dette fjernet kan vi bruge Esc

        // Display the image from camera
        cv::imshow("camera", image);
    }

    // See you
    ardrone.close();

    return 0;
}