//
// Created by lojaz on 13/10/2022.
//

#ifndef LINES_FIGURE_H
#define LINES_FIGURE_H
#include <SFML/Graphics.hpp>
#include <vector>
#include "MBB.h"

using namespace std;
using namespace sf;

bool sortPointsX(Vector2f p1, Vector2f p2){
    return p1.x < p2.x;
}

bool sortPointsY(Vector2f p1, Vector2f p2){
    return p1.y > p2.y;
}

struct Figure
{
    //attributes
    vector<Vector2f> points;
    MBB boundingBox;
    Color color = Color::Magenta;
    //constructors
    Figure() = default;
    Figure(Vector2f _points){
        color = Color::Magenta;
        points.push_back(_points);
        set_MBB();
    }
    Figure(vector<Vector2f> _points){
        color = Color::Magenta;
        for(auto x: _points)
            points.push_back(x);
        auto x = points[0];
        points.push_back(x);
        set_MBB();
    }
    //methods
    void set_MBB(){
        Vector2f topLeft, bottomRight;
        if(points.size()==1){
            boundingBox = MBB(points[0], points[0]);
        }else {
            auto vx = points;
            sort(vx.begin(), vx.end(), sortPointsX);
            auto vy = points;
            sort(vy.begin(), vy.end(), sortPointsY);
            topLeft = {vx[0].x, vy[vy.size()-1].y};
            bottomRight = {vx[vx.size()-1].x, vy[0].y};
            boundingBox = {topLeft, bottomRight};
        }
    }

    bool endFigure(Vector2f p1, Vector2f p2) {
        auto dist = sqrt(((p2.x - p1.x)*(p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
        return dist < 10;
    }

    bool addPoint(Vector2f v){
        if(points.size() == 0 || !endFigure(points[0], v)){
            points.push_back(v);
            set_MBB();
            return true;
        }else if(endFigure(points[0], v)){
            points.push_back(points[0]);
            return false;
        }
        return true;
    }

    void clear(){
        points.clear();
        boundingBox = MBB({0,0}, {0,0});
    }

    void drawLines(RenderWindow &window){

        for (int i = 1; i < points.size(); ++i)
        {
            CircleShape circleShape1(1), circleShape2(1);
            circleShape1.setPosition(points[i - 1]);
            circleShape2.setPosition(points[i]);
            window.draw(circleShape1);
            window.draw(circleShape2);
            sf::Vertex vertices[2] = {Vertex(Vector2f(points[i - 1]), color), Vertex(points[i], color)};
            window.draw(vertices, 2, sf::Lines);
        }
    }
    void draw(RenderWindow &window){
        if(points.size() == 1){
            CircleShape circleShape(2);
            circleShape.setPosition(boundingBox.topLeft);
            circleShape.setFillColor(Color::Green);
            window.draw(circleShape);
        }else
        {
            boundingBox.draw(window);
            for (int i = 1; i < points.size(); ++i)
            {
                sf::Vertex vertices[2] = {Vertex(Vector2f(points[i - 1]), color), Vertex(points[i], color)};
                window.draw(vertices, 2, sf::Lines);
            }
        }
    }

    MBB getBoundingBox(){
        return boundingBox;
    }
    bool isEmpty(){
        return points.empty();
    }
};


#endif //LINES_FIGURE_H
