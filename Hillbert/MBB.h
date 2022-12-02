//
// Created by lojaz on 13/10/2022.
//

#ifndef LINES_MBB_H
#define LINES_MBB_H
#include <SFML/Graphics.hpp>
#include <cmath>
using namespace sf;
struct MBB
{
    //Attributes
    Vector2f topLeft, topRight, bottomRight, bottomLeft, centroid;
    Color color = Color::White;
    //Constructors
    MBB() = default;
    MBB(Vector2f topL, Vector2f bottomR)
    {
        topLeft = topL;
        bottomRight = bottomR;
        topRight = {bottomRight.x, topLeft.y};
        bottomLeft = {topLeft.x, bottomRight.y};
        centroid = {(topRight.x + topLeft.x)/2, (bottomRight.y + topRight.y)/2};
    }
    //methods
    float perimeter() const{
        auto l = abs(bottomRight.x - topLeft.x);
        auto w = abs(bottomRight.y - topLeft.y);
        return 2*(l+w);
    }

    float area() const{
        auto l = bottomRight.x - topLeft.x;
        auto w = bottomRight.y - topLeft.y;
        return std::abs(l*w);
    }

    void merge(MBB other){
        topLeft = {std::min(topLeft.x, other.topLeft.x),
                   std::min(topLeft.y, other.topLeft.y)};
        bottomRight = {std::min(bottomRight.x, other.bottomRight.x),
                   std::min(bottomRight.y, other.bottomRight.y)};
        centroid = {(bottomRight.x + topLeft.x) / 2, (bottomRight.y + topLeft.y) / 2};
    }

    bool isPoint() {
        return topLeft == bottomRight;
    }

    bool nearPoint(Vector2f p1, Vector2f p2) {
        auto dist = sqrt(((p2.x - p1.x)*(p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
        return dist < 10;
    }

    float getOverlap(MBB other) {
        auto x_left = max(topLeft.x, other.topLeft.x);
        auto y_top = max(topLeft.y, other.topLeft.y);
        auto x_right = min(bottomRight.x, other.bottomRight.x);
        auto y_bottom = min(bottomRight.y, other.bottomRight.y);
        if (x_right < x_left or y_bottom < y_top)
            return 0.0;
        auto intersection_area = (x_right - x_left) * (y_bottom - y_top);
        auto iou = intersection_area / float(area() + other.area() - intersection_area);
        return iou;
    }

    bool isInside(Vector2f p){
        if(isPoint()){
            if(nearPoint(topLeft, p)){
                return true;
            }
            return false;
        }
        if( (p.x > topLeft.x && p.x < bottomRight.x) &&
               (p.y > topLeft.y && p.y < bottomRight.y))
            return true;
        return false;
    }

    void draw(RenderWindow& window){
        std::vector<Vector2f> points = {topLeft, topRight, bottomRight, bottomLeft, topLeft};
        for(int i = 1; i < points.size(); ++i){
            sf::Vertex vertices[2] = {Vertex(Vector2f(points[i - 1]), color), Vertex(points[i], color)};
            window.draw(vertices, 2, sf::Lines);
        }
    }
};


#endif //LINES_MBB_H
