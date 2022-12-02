//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_NODE_H
#define LINES_NODE_H
#include "MBB.h"
#include "Figure.h"
#include <iostream>
#define DIM 1024
void rot(int n, int *x, int *y, int rx, int ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }
        //Swap x and y
        int t  = *x;
        *x = *y;
        *y = t;
    }
}


int xy2d ( const Vector2f& p) {
    int n = DIM;
    int rx, ry, s, d=0;
    int x = p.x;
    int y = DIM - p.y - 1;
    for (s=n/2; s>0; s/=2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        rot(n, &x, &y, rx, ry);
    }
    return d;
}

struct Node{
    //attributes
    MBB boundingBox;
    std::vector<Node*> children;
    Node* father = nullptr;
    int hIndex;
    CircleShape boundingCircle;

    //constructors
    Node() = default;
    Node(std::vector<Node*> nodes){
        copy(nodes.begin(), nodes.end(), back_inserter(children));
        mergeBoundingBoxes();
        boundingCircle.setOutlineColor(Color::Blue);
    }
    Node(MBB mbb) : boundingBox{mbb} {}

    //methods
    virtual bool isLeaf(){
        if(children.empty() || (!children.empty() && children[0]->children.empty()))
            return true;
        else{
            return false;
        }
    }

    virtual void draw(RenderWindow &window){
        boundingBox.draw(window);
        window.draw(boundingCircle);
    }
    float getRadio(){
        float centerX = (boundingBox.topRight.x + boundingBox.topLeft.x) / 2;
        float centerY = (boundingBox.bottomLeft.y + boundingBox.topLeft.y) / 2;
        return sqrt((boundingBox.topLeft.x - centerX ) * (boundingBox.topLeft.x - centerX ) + (boundingBox.topLeft.y - centerY) * (boundingBox.topLeft.y - centerY));
    }
    void mergeBoundingBoxes(){
        auto auxFig = children[0]->boundingBox;

        for(auto x: children){
           auxFig = mergeTwoBoxes(auxFig, x->boundingBox);
        }
        boundingBox = auxFig;
        hIndex = this->children.back()->hIndex;
    }

    MBB mergeTwoBoxes(MBB BB, MBB auxBB){
        Vector2f topleft = {min(BB.topLeft.x, auxBB.topLeft.x),
                            min(BB.topLeft.y, auxBB.topLeft.y)};
        Vector2f bottomRight = {max(BB.bottomRight.x, auxBB.bottomRight.x),
                                max(BB.bottomRight.y, auxBB.bottomRight.y)};
        return {topleft, bottomRight};
    }

    void mergeBB(MBB auxBB){
        if(children.size() == 1){
            boundingBox = auxBB;
        } else
        {
            Vector2f topleft = {min(boundingBox.topLeft.x, auxBB.topLeft.x),
                                min(boundingBox.topLeft.y, auxBB.topLeft.y)};
            Vector2f bottomRight = {max(boundingBox.bottomRight.x, auxBB.bottomRight.x),
                                    max(boundingBox.bottomRight.y, auxBB.bottomRight.y)};
            boundingBox = MBB(topleft, bottomRight);
        }
    }
    virtual int gethIndex() {
        return hIndex;
    }
    MBB getBoundingBox(){return boundingBox;}
    //virtual Figure getFigure();
};

struct leafNode: Node{
    //attributes
    Figure figure;

    //constructors
    leafNode() = default;
    leafNode(Figure figure1) {
        figure=figure1;
        boundingBox = figure1.getBoundingBox();
    }

    //methods
    bool isLeaf() override {return true;}
    int gethIndex() override {return xy2d(figure.boundingBox.centroid);}
    void draw(RenderWindow &window) override {
        boundingBox.draw(window);
        figure.draw(window);
        window.draw(boundingCircle);
    }

    Figure getFigure() {return figure;}
};

#endif //LINES_NODE_H
