//
// Created by lojaz on 4/10/2022.
//

#ifndef LINES_SIDEBAR_H
#define LINES_SIDEBAR_H
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include "button.h"
#define width 800
#define height 800
using namespace sf;
using namespace std;
sf::Font font;
vector<string> desc = {"pointInsert","polygonInsert", "delete", "x", "knn"};
class Sidebar
{
    float l,h;
    RectangleShape rectangleShape;

    //0 point insert; 1 polygon insert; 3 delete; 4 range; 5 knn
    vector<Text> descriptions;
    vector<Button> buttons;
public:
    Sidebar(float l, float h) {
        this->l = l;
        this->h = h;
        init();
    }
    
    void init(){
        rectangleShape.setSize({l, h});
        rectangleShape.setPosition({width-l, 0});
        rectangleShape.setFillColor(Color::White);
        float y = 100;
        float x = width-( l) / 2;
        for(int i = 0; i < 5; ++i){
            y = y + 100;
            buttons.push_back(Button({x, y}, 7, desc[i], Color::Red, Color::Cyan ));
            descriptions.emplace_back(Text(desc[i], font, 15));
            descriptions[i].setPosition({x-25, y+25});
            descriptions[i].setFillColor(Color::Black);
        }
    }
    
    void renderSideBar(RenderWindow& window){
        window.draw(rectangleShape);
        for(auto& x: descriptions) {
            window.draw(x);
        }
        for(auto& x: buttons){
            window.draw(x);
        }
    }

    //1 --> insert Point; 2 --> insert Polygon; 3 --> delete; 4 --> range; 5 --> knn
    int checkClick(float x, float y) {
        int i = 0;
         for(auto &z : buttons){
            if(!z.circleOverlap(x,y)){
                continue;
            }
            if(z.circleOverlap(x,y) && !z.getClick()){
                for(auto &d : buttons){
                        d.setClick(false);
                        d.set_state(Color::Red);
                }
                z.set_state(Color::Magenta);
                z.setClick(true);
            }
        }
         for(auto &a: buttons){
             ++i;
             if(a.getClick())
                 return i;
         }
        return i;
    }
};

#endif //LINES_SIDEBAR_H
