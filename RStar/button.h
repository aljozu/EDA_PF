//
// Created by lojaz on 4/10/2022.
//

#ifndef LINES_BUTTON_H
#define LINES_BUTTON_H
#include <SFML/Graphics.hpp>
#include <string>
#include <utility>

using namespace std;
using namespace sf;

class Button : public sf::Drawable{
    Vector2f position;
    float radius_size;
    string description;
    bool is_clicked;
    Color _default, pressed;
    CircleShape circleShape;


public:
    Button() = default;
    Button(Vector2f position, float radius_size, string description, Color color, Color color1) {
        this->position = position;
        this->radius_size = radius_size;
        this->description = std::move(description);
        _default = color;
        pressed = color1;
        is_clicked = false;
        init();
    }

    void init(){
        circleShape.setRadius(radius_size);
        circleShape.setPosition(position);
        circleShape.setFillColor(_default);
    }

    void set_state(Color color){
        circleShape.setFillColor(color);
    }
    void draw(RenderTarget& target, RenderStates states) const override{
        target.draw(circleShape);
    };

    bool circleOverlap(float x, float y) {
        auto coord = circleShape.getGlobalBounds();
        return coord.contains(x, y);
    }

    void setClick(bool b){
        is_clicked = b;
    }

    bool getClick(){
        return is_clicked;
    }
};
#endif //LINES_BUTTON_H
