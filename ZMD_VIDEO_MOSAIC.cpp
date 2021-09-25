#include <iostream>
#include <stdlib.h>
#include "opencv2/video.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;

void printCmdArguments(int& r, int& c, int& i_w, int& i_h, string& i_v, string& o_i);
void printVideoParameters(VideoCapture cap, long int& f_t, int& fps, double& t_t, double& v_w, double& v_h, double& a_r);
void printVideoParametersAfterResize(double& f_v_h, double& f_e_h, int& i_w, int& i_h, double& a_r);
void printOutput(int& n_of_f, int& r, int& c, int& item_w, int& item_h, double& item_v_w, double& item_v_h, double& item_e_w, double& item_e_h, int& image_w, int& image_h, double& a_r);
void getFrameNumbersToExtract(double* times_of_extraction, int* frames_to_extract, int& n_of_f, int fps, double& t_t, long int& f_t);
void printFrameNumbersToExtract(int& n_of_f, double* times_of_extraction, int* frames_to_extract);
Mat addEdges(const cv::Mat& img, int target_width, int target_height, int x, int y, int width, int height);
Mat getFinalImage(vector<Mat> frames, Mat& item, int& n_of_f, int& image_w, int& image_h, int& item_w, int& item_h);





int main(int argc, char *argv[])
{
    int rows, cols,
        image_width, image_height,
        item_width, item_height,
        number_of_frames_to_extract, 
        fps;
    long int frames_total,
             frame_index;
    double time_total, 
           video_width, video_height,
           frame_video_height, frame_edge_height,
           aspect_ratio, 
           item_video_width = 0., item_video_height = 0.,
           item_edge_width = 0., item_edge_height = 0.;
    string input_video,
           output_path = "output_images/",
           output_image;
    vector<Mat> frames;

    //Argumenty příkazového řádku
    cout << "Entered " << argc << " arguments:" << "\n";
    for (int i = 0; i < argc; ++i)
        cout << argv[i] << "\n";

    rows = strtol(argv[1], NULL, 10);
    cols = strtol(argv[2], NULL, 10);
    image_width = strtol(argv[3], NULL, 10);
    image_height = strtol(argv[4], NULL, 10);
    input_video = argv[5];
    output_image = argv[6];

    string filePath = output_path + output_image;

    //Otevření souboru
    VideoCapture cap(input_video);
    if (!cap.isOpened()) {
        cout << "Can not open video file\n";
        return 0;
    }

    //Informace
    printCmdArguments(rows, cols,
                      image_width, image_height,
                      input_video,
                      output_image);
    printVideoParameters(cap,
                         frames_total,
                         fps,
                         time_total,
                         video_width, video_height,
                         aspect_ratio);
    printVideoParametersAfterResize(frame_video_height, frame_edge_height,
                                    image_width, image_height,
                                    aspect_ratio);
    printOutput(number_of_frames_to_extract,
                rows, cols,
                item_width, item_height,
                item_video_width, item_video_height,
                item_edge_width, item_edge_height,
                image_width, image_height,
                aspect_ratio);

    //Získání potřebných snímků
    double* times_of_extracting = new double[number_of_frames_to_extract];
    int* frames_to_extract = new int[number_of_frames_to_extract];

    getFrameNumbersToExtract(times_of_extracting,
                             frames_to_extract,
                             number_of_frames_to_extract,
                             fps,
                             time_total,
                             frames_total);

    printFrameNumbersToExtract(number_of_frames_to_extract,
                               times_of_extracting,
                               frames_to_extract);

    int i = 0;

    //Spuštění videa
    for (;;)
    {
        Mat frame;
        cap >> frame;      

        imshow(input_video, frame);
        frame_index = (long int)cap.get(CAP_PROP_POS_FRAMES);
        
        if (frames_to_extract[i] == frame_index) {

            //Upravit rozměry vstupního videa na velikost výstupního obrázku
            Mat resized = Mat::zeros((int)frame_video_height, image_width, IMREAD_COLOR);
            resize(frame, frame, resized.size());

            //Přidat okraje pro zachování poměru stran
            Mat item;
            if(aspect_ratio >= (rows / (double)cols))
                item = addEdges(frame, item_width, item_height, 0, (int)item_edge_height, item_width, (int)item_video_height);
            else
                item = addEdges(frame, item_width, item_height, (int)item_edge_width, 0, (int)item_video_width, item_height);
            
            //Uložení snímku do pole
            frames.push_back(item);

            i++;

            //Skok na další snímek
            cap.set(CAP_PROP_POS_FRAMES, frames_to_extract[i] - 1);
        }   

        if (frame_index >= frames_total)
            break;
        if (waitKey(1) >= 0)
            break;
    }

    Mat output_size(image_height, image_width, frames[0].type());

    item_width = (int)item_width;
    item_height = (int)item_height;

    //Získání výsledného obrázku
    Mat result = getFinalImage(frames,
                               output_size,
                               number_of_frames_to_extract,
                               image_width, image_height,
                               item_width, item_height);

    imshow(output_image, result);
    imwrite(filePath, result);
    cout << "Done. Output file: " << output_image << " stored in " << output_path << "\n";

    waitKey();
    return 0;
}





void printCmdArguments(int& r, int& c, int& i_w, int& i_h, string& i_v, string& o_i) {
    cout << "-------------------------------------------- COMMAND LINE ARGUMENTS --------------------------------------------\n";
    cout << "Rows:\t\t\t\t" << r << "\n";
    cout << "Cols:\t\t\t\t" << c << "\n";
    cout << "Output image width:\t\t" << i_w << "\n";
    cout << "Output image height:\t\t" << i_h << "\n";
    cout << "Input video file:\t\t" << i_v << "\n";
    cout << "Output image name:\t\t" << o_i << "\n";
}

void printVideoParameters(VideoCapture cap, long int& f_t, int& fps, double& t_t, double& v_w, double& v_h, double& a_r) {
    cout << "----------------------------------------------- VIDEO PARAMETERS -----------------------------------------------\n";
    f_t = (long int)cap.get(CAP_PROP_FRAME_COUNT);
    cout << "Total number of frames in file:\t" << f_t << "\n";

    fps = (int)cap.get(CAP_PROP_FPS);
    cout << "FPS:\t\t\t\t" << fps << "\n";

    t_t = f_t / fps;
    cout << "Video length:\t\t\t" << t_t << " sec\n";

    v_w = cap.get(CAP_PROP_FRAME_WIDTH);
    cout << "Video width:\t\t\t" << v_w << " px\n";

    v_h = cap.get(CAP_PROP_FRAME_HEIGHT);
    cout << "Video height:\t\t\t" << v_h << " px\n";

    a_r = v_w / v_h;
    cout << "Aspect ratio:\t\t\t" << a_r << "\n";
}

void printVideoParametersAfterResize(double& f_v_h, double& f_e_h, int& i_w, int& i_h, double& a_r) {
    cout << "---------------------------------------------- AFTER VIDEO RESIZE ----------------------------------------------\n";
    f_v_h = i_w / a_r;
    f_e_h = (i_h - f_v_h) / (double)2;

    cout << "Video width:\t\t\t" << i_w << " px\n";
    cout << "Video height:\t\t\t" << i_h << " px ";
    cout << "(video height: " << f_v_h << " px and edge height: " << f_e_h << " px x2)\n";
}

void printOutput(int& n_of_f, int& r, int& c, int& item_w, int& item_h, double& item_v_w, double& item_v_h, double& item_e_w, double& item_e_h, int& image_w, int& image_h, double& a_r) {
    cout << "---------------------------------------------------- OUTPUT ----------------------------------------------------\n";
    n_of_f = r * c;
    cout << "Number of frames to extract:\t" << n_of_f << "\n";

    item_h = (int)(image_h / (double)r);
    item_w = (int)(image_w / (double)c);

    if (a_r >= (r / (double)c)) {
        item_v_h = item_w / a_r;
        item_e_h = (item_h - item_v_h) / (double)2;
        cout << "Item width:\t\t\t" << item_w << " px (x " << c << " = " << image_w << ")\n";
        cout << "Item height:\t\t\t" << item_h << " px (x " << r << " = " << image_h << ") ";
        cout << "(video height: " << item_v_h << " px and edge height: " << item_e_h << " px x2)\n";
    }
    else {
        item_v_w = item_h * a_r;
        item_e_w = (item_w - item_v_w) / (double)2;
        cout << "Item width:\t\t\t" << item_w << " px (x " << c << " = " << image_w << ") ";
        cout << "(video width: " << item_v_w << " px and edge width: " << item_e_w << " px x2)\n";
        cout << "Item height:\t\t\t" << item_h << " px (x " << r << " = " << image_h << ")\n";
    }
}

void getFrameNumbersToExtract(double *times_of_extraction, int *frames_to_extract, int& n_of_f, int fps, double& t_t, long int& f_t) {
    times_of_extraction[0] = 0;
    //Vybral jsem vždy prostřední snímek z dané sekundy (0:00 = 0:00.5) -> 30 FPS / 2 = 15
    //neboť první snímek celého videa (filmu) je černá obrazovka (output-2.jpg by byla černá obrazovka)
    frames_to_extract[0] = (int)(fps / (double)2);

    for (int i = 1; i <= n_of_f; i++) {
        double time_diff = t_t / ((double)n_of_f - (double)1);
        times_of_extraction[i] = times_of_extraction[i - 1] + time_diff;
        double frame_diff = f_t / ((double)n_of_f - (double)1);
        frames_to_extract[i] = frames_to_extract[i - 1] + (int)(frame_diff + 0.5);
    }
    if (n_of_f != 1)
        frames_to_extract[n_of_f - 1] = f_t - (int)(fps / (double)2);
}

void printFrameNumbersToExtract(int& n_of_f, double *times_of_extraction, int *frames_to_extract) {
    cout << "----------------------------------------------------------------------------------------------------------------\n";
    cout << "\tExtraction time\t\tFrame to extract\n";
    for (int i = 0; i < n_of_f; i++) {
        cout << i + 1 << ".\t" << times_of_extraction[i] << "\t\t\t" << frames_to_extract[i] << "\n";
    }
    cout << "----------------------------------------------------------------------------------------------------------------\n";
}

Mat addEdges(const cv::Mat& img, int target_width, int target_height, int x, int y, int width, int height)
{
    Mat result = cv::Mat::zeros(target_height, target_width, img.type());
    Rect roi(x, y, width, height);
    resize(img, result(roi), roi.size());

    return result;
}

Mat getFinalImage(vector<Mat> frames, Mat& output_size, int& n_of_f, int& image_w, int& image_h, int& item_w, int& item_h) {
    int j = 0;
    for (int y = 0; y < image_h - n_of_f; y += item_h) {
        for (int x = 0; x < image_w - n_of_f; x += item_w) {
            //cout << "x: " << to_string(x) << "\ty: " << to_string(y) << "\n";
            Mat roi = output_size(Rect(x, y, item_w, item_h));
            frames[j].copyTo(roi);
            if (j == frames.size() - 1)
                break;
            j++;
        }
    }
    return output_size;
}