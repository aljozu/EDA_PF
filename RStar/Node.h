//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_NODE_H
#define LINES_NODE_H
#include "MBB.h"
#include "Figure.h"
#include <iostream>
#define M 3
struct Node{
    //attributes
    MBB boundingBox;
    std::vector<Node*> children;
    int level;
    Node* father = nullptr;
    CircleShape boundingCircle;

    //constructors
    Node(){
        children.reserve(M+1);
    }
    Node(std::vector<Node*> nodes){
        copy(nodes.begin(), nodes.end(), back_inserter(children));
        mergeBoundingBoxes();
    }
    Node(MBB mbb) : boundingBox{mbb} {}
    virtual ~Node(){
        children.clear();
        father = nullptr;
        boundingBox = MBB();
    }
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
        auto auxFigcolor =  children[0]->boundingBox.color;
        for(auto x: children){
           auxFig = mergeTwoBoxes(auxFig, x->boundingBox);
           if(x->boundingBox.color != Color::White) auxFigcolor = x->boundingBox.color;
        }
        boundingBox = auxFig;
        boundingBox.color = auxFigcolor;
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
    MBB getBoundingBox(){return boundingBox;}
    //virtual Figure getFigure();
};

struct leafNode: Node{
    //attributes
    Figure figure;

    //constructors
    leafNode() = default;
    leafNode(Figure figure1) {
        level = 0;
        figure=figure1;
        boundingBox = figure1.getBoundingBox();
    }
    ~leafNode() override{
        figure = Figure();
    }
    //methods
    bool isLeaf() override {return true;}

    void draw(RenderWindow &window) override {
        boundingBox.draw(window);
        figure.draw(window);
        window.draw(boundingCircle);
    }

    Figure getFigure() {return figure;}
};

#endif //LINES_NODE_H
