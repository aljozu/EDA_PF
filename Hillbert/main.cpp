#include <SFML/Graphics.hpp>
#include "Sidebar.h"
#include "Figure.h"
#include "RTree.h"
#include "test.h"
#include <vector>
sf::Font font1;

using namespace sf;
using namespace std;

int main()
{

    int op;
    vector<Vector2f> polygon;
    sf::RenderWindow window(sf::VideoMode(800, 800), "WINDOW_TITLE");

    if (!font.loadFromFile(R"(C:\Users\lojaz\CLionProjects\lines\Fonts\OpenSans-Regular.ttf)"))
    {
        // error...
    }
    Figure aux_fig;
    Sidebar sidebar(200, 800);
    Figure figure({{400,400}, {500,600},{400,500}, {300,600}});
    RTree *rTree = new RTree();
    //rTree.insert(figure);
    //rTree.bfs(window);
    std::vector<Figure> figures, foundFigures;
    CircleShape circleShape(2);
    circleShape.setFillColor(Color::Yellow);
    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window.close();
                    return 0;
                }case sf::Event::MouseButtonPressed:
                {
                    op = sidebar.checkClick(event.mouseButton.x, event.mouseButton.y);
                    switch(op){
                        case 1:
                            //figures.push_back(Figure({static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y)}));
                            if(event.mouseButton.x < 600)
                            {
                                rTree->insert(Figure({static_cast<float>(event.mouseButton.x),
                                                     static_cast<float>(event.mouseButton.y)}));
                            }
                            //insert point
                            break;
                        case 2:
                            //insert polygon
                            if(event.mouseButton.x < 600)
                            {
                                if (!aux_fig.addPoint({static_cast<float>(event.mouseButton.x),
                                                       static_cast<float>(event.mouseButton.y)}))
                                {
                                    rTree->insert(aux_fig);
                                    //figures.push_back(aux_fig);
                                    aux_fig.clear();
                                }
                            }
                            break;
                        case 3:
                            //delete
                            if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                                if(event.mouseButton.x < 600){
                                    rTree->remove({static_cast<float>(event.mouseButton.x),
                                                   static_cast<float>(event.mouseButton.y)});
                                    circleShape.setPosition({static_cast<float>(event.mouseButton.x),
                                                             static_cast<float>(event.mouseButton.y)});
                                }
                            }
                            break;
                        case 4:
                            //range search
                            break;
                        case 5:
                            //knn
                            if(event.mouseButton.x < 600) {
                                foundFigures = rTree->depthFirst({static_cast<float>(event.mouseButton.x),
                                                                      static_cast<float>(event.mouseButton.y)}, 3);
                                circleShape.setPosition({static_cast<float>(event.mouseButton.x),
                                                         static_cast<float>(event.mouseButton.y)});
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
        }

        window.clear();
        if(op == 2 && !aux_fig.isEmpty()){
            aux_fig.drawLines(window);
        }
        if(op == 3 ){
            window.clear();
            window.draw(circleShape);
        }
        rTree->bfs(window);
        if(op == 5 && !foundFigures.empty()){
            window.draw(circleShape);
            rTree->drawLinesToFoundFigures(foundFigures, window, circleShape);
        }
        //for(auto x: figures)
           // x.draw(window);
        sidebar.renderSideBar(window);
        window.display();

    }

    return 0;
}
