

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <fstream>
#include <chrono>
#include <cstdlib>

using namespace std;
using namespace cv;

struct threadData{
    Mat frameCrop;
    Mat imageCrop;
    float diff;
};

void *func(void *currdata){
    struct threadData *data=(struct threadData*) currdata;
    GaussianBlur(data->frameCrop, data->frameCrop, Size(5, 5), 0, 0);
    absdiff(data->frameCrop, data->imageCrop, data->frameCrop);
    threshold(data->frameCrop, data->frameCrop, 25, 255, THRESH_BINARY);
    data->diff = countNonZero(data->frameCrop);
    pthread_exit(NULL);
}

int main(int argc, const char * argv[]) {
    
    int Method,skip,threadcount,len,wid;
    cout<<"Choose Method: "<<endl;
    cin>>Method;
    
    
    if(Method==1){
        cout<<"Enter number of frames to be skipped: "<<endl;
        cin>>skip;
    }
    
    if(Method==2){
        cout<<"Enter new dimension of the frame: "<<endl;
        cin>>len>>wid;
    }
    
    if(Method==3||Method==4){
        cout<<"Enter number of threads to be used: "<<endl;
        cin>>threadcount;
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    VideoCapture video("trafficvideo.mp4");
    Mat image =imread("clear.jpg");
    Mat imagegray,matrix,imageCrop;
    cvtColor(image, imagegray, COLOR_BGR2GRAY);
    
    if(video.isOpened()==false){
        cout << "Cannot open the video file" <<endl;
        return -1;
    }
    
    vector<Point2f> des;
    des.push_back(Point2f(472,52));
    des.push_back(Point2f(472,830));
    des.push_back(Point2f(800,830));
    des.push_back(Point2f(800,52));
    vector<Point2f> src;
    src.push_back(Point2f(973, 221));
    src.push_back(Point2f(238, 1070));
    src.push_back(Point2f(1546, 1073));
    src.push_back(Point2f(1273, 220));
    
    matrix = getPerspectiveTransform(src, des);
    warpPerspective(imagegray, imagegray, matrix,image.size());
    
    
    if(Method==0){
        ofstream MyFile("Baseline_Density.txt");
        Rect rec(472,52,328,778);
        imageCrop=imagegray(rec);
        GaussianBlur(imageCrop, imageCrop, Size(5,5), 0,0);
            
        float total=255184;
        int c=0;
            
        while(true){
            Mat frame,framegray,frameCrop;
            bool success = video.read(frame);
            if(success==false){
                cout << "Finished" <<endl;
                break;
            }
            if(c%3==0){
                c=0;
                cvtColor(frame, framegray, COLOR_BGR2GRAY);
                warpPerspective(framegray, frameCrop, matrix,image.size());
                frameCrop = frameCrop(rec);
                GaussianBlur(frameCrop ,frameCrop , Size(5,5), 0,0);
                absdiff(frameCrop, imageCrop, frameCrop);
                threshold(frameCrop, frameCrop, 25, 255, THRESH_BINARY);
                float countqueue=countNonZero(frameCrop);
                MyFile<<countqueue/total<<endl;
            }
            c=0;
        }
        
        MyFile.close();
        auto stop = std::chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = stop-start;
        cout<< elapsed.count() <<endl;
        ofstream MyFile1("Method_1.txt");
        MyFile1<<0<<"\t"<<0<<"\t"<<elapsed.count()<<endl;
        MyFile1.close();
        ofstream MyFile2("Method_2.txt");
        MyFile2.close();
        ofstream MyFile3("Method_3.txt");
        MyFile3.close();
        ofstream MyFile4("Method_4.txt");
        MyFile4.close();
    }
    
    else if(Method==1){
        ofstream MyFile("Method_1_Density.txt");
        
        Rect rec(472,52,328,778);
        imageCrop=imagegray(rec);
        GaussianBlur(imageCrop, imageCrop, Size(5,5), 0,0);
        
        float total=255184;
        int c=0;
            
        while(true){
            Mat frame,framegray,frameCrop;
            bool success = video.read(frame);
            if(success==false){
                cout << "Finished" <<endl;
                break;
            }
            if(c%skip==0){
                c=0;
                cvtColor(frame, framegray, COLOR_BGR2GRAY);
                warpPerspective(framegray, frameCrop, matrix,image.size());
                frameCrop = frameCrop(rec);
                GaussianBlur(frameCrop ,frameCrop , Size(5,5), 0,0);
                absdiff(frameCrop, imageCrop, frameCrop);
                threshold(frameCrop, frameCrop, 25, 255, THRESH_BINARY);
                float countqueue=countNonZero(frameCrop);
                for(int j=0;j<skip;j++){
                    MyFile<<countqueue/total<<endl;
                }
            }
            c++;
        }
        MyFile.close();
        auto stop = std::chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = stop-start;
        cout<< elapsed.count() <<endl;
        
        ifstream BaselineDensity("Baseline_Density.txt");
        ifstream Method1Density("Method_1_Density.txt");
        string BaseDen,Mtd1Den;
        vector<int> Baseline,temp;
        while(getline(BaselineDensity,BaseDen)){
            Baseline.push_back(stoi(BaseDen));
        }
        BaselineDensity.close();
        while(getline(Method1Density,Mtd1Den)){
            temp.push_back(stoi(Mtd1Den));
        }
        Method1Density.close();
        float err=0;
        float base=0,tempi=0;
        for(int j=0;j<Baseline.size();j++){
            err=err+abs(Baseline[j]-temp[j]);
        }
        err=err/Baseline.size();
        ofstream Mtd1("Method_1.txt",ios::app);
        Mtd1<<skip<<"\t"<<err<<"\t"<<elapsed.count()<<endl;
        Mtd1.close();
        
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method1Runtime.png' ; set xlabel 'Frames Skipped';set ylabel 'Runtime' ;plot 'Method_1.txt' using 1:3 title 'Method 1 Runtime' lt 7 lc -1 w lp\"");
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method1Error.png' ; set xlabel 'Frames Skipped';set ylabel 'Error' ;plot 'Method_1.txt' using 1:2 title 'Method 1 Error' lt 7 lc -1 w lp\"");
        
        return 0;
    }
    
    else if(Method==2){
        ofstream MyFile("Method_2_Density.txt");
        
        Rect rec(472,52,328,778);
        Size size(len,wid);
        imageCrop=imagegray(rec);
        resize(imageCrop,imageCrop,size);
        GaussianBlur(imageCrop, imageCrop, Size(5,5), 0,0);
        
        float total=len*wid;
        int c=0;
        
        
        while(true){
            Mat frame,framegray,frameCrop;
            bool success = video.read(frame);
            if(success==false){
                cout << "Finished" <<endl;
                break;
            }
            if(c%3==0){
                c=0;
                cvtColor(frame, framegray, COLOR_BGR2GRAY);
                warpPerspective(framegray, frameCrop, matrix,image.size());
                frameCrop = frameCrop(rec);
                resize(frameCrop,frameCrop,size);
                GaussianBlur(frameCrop ,frameCrop , Size(5,5), 0,0);
                absdiff(frameCrop, imageCrop, frameCrop);
                threshold(frameCrop, frameCrop, 25, 255, THRESH_BINARY);
                float countqueue=countNonZero(frameCrop);
                MyFile<<countqueue/total<<endl;
            }
            c=0;
        }
                        
        MyFile.close();
        auto stop = std::chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = stop-start;
        cout<< elapsed.count() <<endl;
        
        ifstream BaselineDensity("Baseline_Density.txt");
        ifstream Method2Density("Method_2_Density.txt");
        string BaseDen,Mtd2Den;
        vector<int> Baseline,temp;
        while(getline(BaselineDensity,BaseDen)){
            Baseline.push_back(stoi(BaseDen));
        }
        BaselineDensity.close();
        while(getline(Method2Density,Mtd2Den)){
            temp.push_back(stoi(Mtd2Den));
        }
        Method2Density.close();
        float err=0;
        float base=0,tempi=0;
        for(int j=0;j<Baseline.size();j++){
            err=err+abs(Baseline[j]-temp[j]);
        }
        err=err/Baseline.size();
        ofstream Mtd2("Method_2.txt",ios::app);
        Mtd2<<total/255184<<"\t"<<err<<"\t"<<elapsed.count()<<endl;
        Mtd2.close();
        
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method2Runtime.png' ; set xlabel 'Area Ratio';set ylabel 'Runtime' ;plot 'Method_2.txt' using 1:3 title 'Method 2 Runtime' lt 7 lc -1 w lp\"");
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method2Error.png' ; set xlabel 'Area Ratio';set ylabel 'Error' ;plot 'Method_2.txt' using 1:2 title 'Method 2 Error' lt 7 lc -1 w lp\"");
        
        return 0;
    }
    
    else if(Method==3){
        ofstream MyFile("Method_3_Density.txt");
        Mat imageCrop;
        struct threadData data[threadcount];
        int q=778/threadcount;
        Rect rec[threadcount];
        for(int i=0;i<threadcount;i++){
            if(i!=threadcount-1){
                rec[i]={472,52+(i*q),328,q};
            }
            else{
                rec[i]={472,52+(i*q),328,778-(i*q)};
            }
            imageCrop=imagegray(rec[i]);
            GaussianBlur(imageCrop ,imageCrop , Size(5,5), 0,0);
            data[i].imageCrop=imageCrop;
        }
        
        float total=255184;
        float countdensity=0;
        int c=0;
        pthread_t threads[threadcount];
        Mat frame,framegray,frameCrop;
        while(true){
            bool success = video.read(frame);
            if(success==false){
                cout << "Finished" <<endl;
                break;
            }
            if(c%3==0){
                c=0;
                cvtColor(frame, framegray, COLOR_BGR2GRAY);
                warpPerspective(framegray, frameCrop, matrix,image.size());
                for(int i=0;i<threadcount;i++){
                    data[i].frameCrop=frameCrop(rec[i]);
                    data[i].diff=0;
                    int rc;
                    rc=pthread_create(&threads[i],NULL,func,(void *)&data[i]);
                    if(rc!=0){
                        cout<<"Error:unable to create thread,"<< rc <<endl;
                        return -1;
                    }
                }
                int difference=0;
                for(int j=0;j<threadcount;j++){
                    pthread_join(threads[j],NULL);
                    difference=difference+data[j].diff;
                }
                MyFile<<difference/total<<endl;
            }
            c=0;
        }
        
        MyFile.close();
        auto stop = std::chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = stop-start;
        cout<< elapsed.count() <<endl;
        ifstream BaselineDensity("Baseline_Density.txt");
        ifstream Method3Density("Method_3_Density.txt");
        string BaseDen,Mtd3Den;
        vector<int> Baseline,temp;
        while(getline(BaselineDensity,BaseDen)){
            Baseline.push_back(stoi(BaseDen));
        }
        BaselineDensity.close();
        while(getline(Method3Density,Mtd3Den)){
            temp.push_back(stoi(Mtd3Den));
        }
        Method3Density.close();
        float err=0;
        float base=0,tempi=0;
        for(int j=0;j<Baseline.size();j++){
            err=err+abs(Baseline[j]-temp[j]);
        }
        err=err/Baseline.size();
        ofstream Mtd3("Method_3.txt",ios::app);
        Mtd3<<threadcount<<"\t"<<err<<"\t"<<elapsed.count()<<endl;
        Mtd3.close();
        
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method3Runtime.png' ; set xlabel 'Number of Threads';set ylabel 'Runtime' ;plot 'Method_3.txt' using 1:3 title 'Method 3 Runtime' lt 7 lc -1 w lp\"");
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method3Error.png' ; set xlabel 'Number of Threads';set ylabel 'Error' ;plot 'Method_3.txt' using 1:2 title 'Method 3 Error' lt 7 lc -1 w lp\"");
        
        return 0;
    }
    
    else if(Method==4){
        ofstream MyFile("Method_4_Density.txt");
        Rect rec(472,52,328,778);
        imageCrop=imagegray(rec);
        GaussianBlur(imageCrop, imageCrop, Size(5,5), 0,0);
        
        float total=255184;
        int c=0;
        
        Mat frame,framegray,frameCrop;
        int currframe=0;
        int rc;
        pthread_t threads[threadcount];
        struct threadData data[threadcount];
        for(int j=0;j<threadcount;j++){
            data[j].imageCrop=imageCrop;
        }
        while(true){
            bool success = video.read(frame);
            if(success==false){
                for(int i=0;i<threadcount;i++){
                    pthread_join(threads[currframe],NULL);
                    MyFile<<data[currframe].diff/total<<endl;
                    if(currframe==threadcount-1){
                        currframe=0;
                    }
                    else{
                        currframe++;
                    }
                }
                cout << "Finished" <<endl;
                break;
            }
            if(c%3==0){
                c=0;
                cvtColor(frame, framegray, COLOR_BGR2GRAY);
                warpPerspective(framegray, frameCrop, matrix,image.size());
                frameCrop=frameCrop(rec);
                
                data[currframe].frameCrop=frameCrop;
                data[currframe].diff=0;
                rc=pthread_create(&threads[currframe],NULL,func,(void *)&data[currframe]);
                if(rc!=0){
                    cout<<"Error:unable to create thread,"<< rc <<endl;
                    return -1;
                }

                if(currframe==threadcount-1){
                    for(int j=0;j<threadcount;j++){
                        pthread_join(threads[j],NULL);
                        MyFile<<data[j].diff/total<<endl;
                    }
                    currframe=0;
                }
                else{
                    currframe++;
                }
            }
            c=0;
        }
        MyFile.close();
        auto stop = std::chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = stop-start;
        cout<< elapsed.count() <<endl;
        ifstream BaselineDensity("Baseline_Density.txt");
        ifstream Method4Density("Method_4_Density.txt");
        string BaseDen,Mtd4Den;
        vector<int> Baseline,temp;
        while(getline(BaselineDensity,BaseDen)){
            Baseline.push_back(stoi(BaseDen));
        }
        BaselineDensity.close();
        while(getline(Method4Density,Mtd4Den)){
            temp.push_back(stoi(Mtd4Den));
        }
        Method4Density.close();
        float err=0;
        float base=0,tempi=0;
        for(int j=0;j<Baseline.size();j++){
            err=err+abs(Baseline[j]-temp[j]);
        }
        err=err/Baseline.size();
        ofstream Mtd4("Method_4.txt",ios::app);
        Mtd4<<threadcount<<"\t"<<err<<"\t"<<elapsed.count()<<endl;
        Mtd4.close();
        
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method4Runtime.png' ; set xlabel 'Number of Threads';set ylabel 'Runtime' ;plot 'Method_4.txt' using 1:3 title 'Method 4 Runtime' lt 7 lc -1 w lp\"");
        system("gnuplot -e \"set terminal png size 800,600; set output 'Method4Error.png' ; set xlabel 'Number of Threads';set ylabel 'Error' ;plot 'Method_4.txt' using 1:2 title 'Method 4 Error' lt 7 lc -1 w lp\"");
        return 0;
    }
    else{
        cout<<"Wrong input"<<endl;
        return -1;
    }
}
