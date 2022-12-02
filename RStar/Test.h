//
// Created by lojaz on 30/11/2022.
//

#ifndef LINES_TEST_H
#define LINES_TEST_H
#include <random>
#include <chrono>
#include <fstream>
#include "RTree.h"
#define LIENZO 2048
float minBoundingLength = LIENZO*0.0042;
float maxBoundingLength = LIENZO*0.0084;
std::random_device deva;
std::mt19937_64 enga(deva());
template<class T>
float RandF(T first, T last) {
    std::uniform_real_distribution<float> dis(first, last);
    return dis(enga);
}

float axeCoord(float x) {
    float p2 = 0.0;
    if(x - minBoundingLength > 0){
        p2 = RandF(float(max(float(0), x - maxBoundingLength)), x - minBoundingLength);
    }
    else if(x + minBoundingLength < LIENZO){
        p2 = RandF(x + minBoundingLength, min(x + maxBoundingLength, float(LIENZO)));
    }
    return p2;
}
Figure createRandomFigure(){
    vector<Vector2f> points;
    Vector2f p1;
    p1 = {RandF(float(0), float(LIENZO)), RandF(float(0), float(LIENZO))};
    points.push_back(p1);
    auto nPoints = Rand(3,8);
    for(int i = 0; i < nPoints; ++i){
        points.emplace_back(axeCoord(p1.x), axeCoord(p1.y));
    }
    return {points};
}

float getMBBoverlaps(vector<Figure>& figures){
    float totalOverlap = 0;
    auto size = figures.size();
    for(int i = 0; i < size; ++i){
        for(int j = 0; j < size; ++j){
            if (i != j){
                totalOverlap += figures[i].boundingBox.getOverlap(figures[j].boundingBox);
            }
        }
    }
    return totalOverlap;
}

vector<Figure> getNRandomFigures(int n){
    vector<Figure> aux;
    for(int i = 0; i < n; ++i)
        aux.push_back(createRandomFigure());
    return aux;
}
void testInsert(){

    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using high_resolution_clock = std::chrono::steady_clock;
    ofstream myfile;
    myfile.open (R"(C:\Users\lojaz\CLionProjects\RStar\insertTest.csv)");
    auto tree = new RTree();
    for(int j = 0; j < 100; ++j)
    {
        tree = new RTree();
        cout << "j: " << j << endl;
        for (int i = 0; i < 400; ++i)
        {
            auto aux = getNRandomFigures(10);
            auto t1 = high_resolution_clock::now();
            for (auto &x: aux)
                tree->insert(x);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            myfile << ms_double.count() << ',';

        }
        delete tree;
        myfile << endl;
    }
    myfile.close();
}

void testSearch(){
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using high_resolution_clock = std::chrono::steady_clock;
    ofstream myfile;
    myfile.open (R"(C:\Users\lojaz\CLionProjects\RStar\searchTest.csv)");

    for(int j = 0; j < 100; ++j)
    {
        RTree *tree = new RTree();
        cout << "j: " << j << endl;
        for (int i = 0; i < 5; ++i)
        {
            auto aux = getNRandomFigures(1000);
            for (auto &x: aux)
                tree->insert(x);

            auto t1 = high_resolution_clock::now();
            for(const auto& p: aux){
                tree->search(p.boundingBox.centroid);
            }
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            myfile << ms_double.count() << ',';
        }
        myfile << endl;
        delete tree;
    }
    myfile.close();
}

void testRemove(){
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using high_resolution_clock = std::chrono::steady_clock;
    ofstream myfile;
    myfile.open (R"(C:\Users\lojaz\CLionProjects\RStar\removeTest.csv)");

    for(int j = 0; j < 100; ++j)
    {
        RTree *tree = new RTree();
        cout << "j: " << j << endl;
        for (int i = 0; i < 5; ++i)
        {
            auto aux = getNRandomFigures(100);
            for (auto &x: aux)
                tree->insert(x);

            auto t1 = high_resolution_clock::now();
            for(const auto& p: aux){
                tree->remove(p.boundingBox.centroid);
            }
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            myfile << ms_double.count() << ',';
        }
        myfile << endl;
        delete tree;
    }
    myfile.close();
}

void knn1Search(){
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using high_resolution_clock = std::chrono::steady_clock;
    ofstream myfile;
    myfile.open (R"(C:\Users\lojaz\CLionProjects\RStar\knn1.csv)");

    for(int j = 0; j < 100; ++j)
    {
        RTree *tree = new RTree();
        cout << "j: " << j << endl;
        for (int i = 0; i < 5; ++i)
        {
            auto aux = getNRandomFigures(800);
            for (auto &x: aux)
                tree->insert(x);

            auto t1 = high_resolution_clock::now();
            tree->knnSearch(Vector2f(RandF(0,LIENZO), RandF(0,LIENZO)), 1);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            myfile << ms_double.count() << ',';
        }
        myfile << endl;
        delete tree;
    }
    myfile.close();
}

void knn2Search(){
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using high_resolution_clock = std::chrono::steady_clock;
    ofstream myfile;
    myfile.open (R"(C:\Users\lojaz\CLionProjects\RStar\knn2.csv)");

    for(int j = 0; j < 100; ++j)
    {
        RTree *tree = new RTree();
        cout << "j: " << j << endl;
        for (int i = 0; i < 5; ++i)
        {
            auto aux = getNRandomFigures(800);
            for (auto &x: aux)
                tree->insert(x);

            auto t1 = high_resolution_clock::now();
            tree->knnSearch(Vector2f(RandF(0,LIENZO), RandF(0,LIENZO)), 2);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            myfile << ms_double.count() << ',';
        }
        myfile << endl;
        delete tree;
    }
    myfile.close();
}

void knn3Search(){
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using high_resolution_clock = std::chrono::steady_clock;
    ofstream myfile;
    myfile.open (R"(C:\Users\lojaz\CLionProjects\RStar\knn3.csv)");

    for(int j = 0; j < 100; ++j)
    {
        RTree *tree = new RTree();
        cout << "j: " << j << endl;
        for (int i = 0; i < 5; ++i)
        {
            auto aux = getNRandomFigures(800);
            for (auto &x: aux)
                tree->insert(x);

            auto t1 = high_resolution_clock::now();
            tree->knnSearch(Vector2f(RandF(0,LIENZO), RandF(0,LIENZO)), 3);
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            myfile << ms_double.count() << ',';
        }
        myfile << endl;
        delete tree;
    }
    myfile.close();
}

void testTree(){

    vector<Figure> figures;
    for(int i = 0; i < 4800; ++i){
        figures.push_back(createRandomFigure());
    }
    auto totalOverlap = getMBBoverlaps(figures);
    cout << "Total overlap R-Tree: " << totalOverlap << endl;
    /*
    testInsert();
    testSearch();
    testRemove();
    knn1Search();
    knn2Search();
    knn3Search();
     */
}
#endif //LINES_TEST_H
