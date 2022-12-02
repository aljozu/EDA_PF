//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_NODE_H
#define LINES_NODE_H
#include "MBB.h"
#include "Figure.h"
#include <iostream>
struct Node{
    //attributes
    MBB boundingBox;
    std::vector<Node*> children;
    Node* father = nullptr;

    //constructors
    Node() = default;

    virtual ~Node(){
        children.clear();
        father = nullptr;
        boundingBox = MBB();
    }
    Node(std::vector<Node*> nodes){
        copy(nodes.begin(), nodes.end(), back_inserter(children));
        mergeBoundingBoxes();
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
    }

    void mergeBoundingBoxes(){
        auto auxFig = children[0]->boundingBox;
        auto co = Color::White;
        for(auto x: children){
           auxFig = mergeTwoBoxes(auxFig, x->boundingBox);
           if(x->boundingBox.color != co) auxFig.color = x->boundingBox.color;
        }
        boundingBox = auxFig;
    }

    CircleShape mergeTwoCircles(CircleShape c1, CircleShape c2){
        return CircleShape(max(c1.getRadius(), c2.getRadius()));
    }
    MBB mergeTwoBoxes(MBB BB, MBB auxBB){
        Vector2f topleft = {min(BB.topLeft.x, auxBB.topLeft.x),
                            min(BB.topLeft.y, auxBB.topLeft.y)};
        Vector2f bottomRight = {max(BB.bottomRight.x, auxBB.bottomRight.x),
                                max(BB.bottomRight.y, auxBB.bottomRight.y)};
        return {topleft, bottomRight};
    }

    float getRadio(){
        float centerX = (boundingBox.topRight.x - boundingBox.topLeft.x) / 2;
        float centerY = (boundingBox.bottomLeft.y - boundingBox.topLeft.y) / 2;
        return sqrt((boundingBox.topLeft.x - centerX ) * (boundingBox.topLeft.x - centerX ) + (boundingBox.topLeft.y - centerY) * (boundingBox.topLeft.y - centerY));
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
    }

    Figure getFigure() {return figure;}
};

#endif //LINES_NODE_H
