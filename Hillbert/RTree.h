//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_RTREE_H
#define LINES_RTREE_H


#include "Node.h"
#include <limits>
#include <queue>
#include <random>

#define M 22

std::random_device dev;
template<class T>
int Rand(T first, T last) {
    std::default_random_engine eng(dev());
    std::uniform_int_distribution<int> dis(first, last);
    return dis(eng);
}

//funcion para ordenar por menor distancia
static bool orderByLowestDistance(pair<Node*, double> p1, pair<Node*, double> p2){
    return p1.second < p2.second;
}

//funcion para ordenar por mayor distancia
static bool orderByHighestDistance(pair<Node*, double> p1, pair<Node*, double> p2){
    return p1.second > p2.second;
}


//distancia entre dos bounding boxes
static float distanceMBB(Vector2f p, MBB node){
    auto dx = max({node.topLeft.x - p.x, float(0), p.x - node.bottomRight.x});
    auto dy = max({node.topLeft.y - p.y, float(0), p.y - node.bottomRight.y});
    return sqrt(dx*dx + dy*dy);
}

//otra forma de comparar bounding boxes
struct LessThanByMBB {
    bool operator()(Figure* f1, Figure*f2, Vector2f p) const
    {
        MBB m1 = f1->getBoundingBox();
        MBB m2 = f2->getBoundingBox();
        return distanceMBB(p, m1) < distanceMBB(p, m2);
    }
};

//comparar la distancia entre nodos
struct compareNodes{
    bool operator()(pair<Node*, float> n1, pair<Node*, float> n2){
        return n1.second > n2.second;
    }
};

//escoger la menor suma entre ejes
MBB pickLowerSum(vector<MBB> boxes){
    MBB temp;
    double sum = 0;
    for(auto x: boxes){
        auto sum2 = x.topLeft.x + x.topLeft.y;
        if(sum2 < sum){
            sum = sum2;
            temp.bottomRight = x.bottomRight;
            temp.topLeft = x.topLeft;
        }
    }
    return temp;
}

//mayor suma entre ejes
MBB pickHighestSum(vector<MBB> boxes){
    MBB temp;
    double sum = 0;
    for(auto x: boxes){
        auto sum2 = x.bottomRight.x + x.bottomRight.y;
        if(sum2 < sum){
            sum = sum2;
            temp.bottomRight = x.bottomRight;
            temp.topLeft = x.topLeft;
        }
    }
    return temp;
}

//distancia entre dos puntos
double getDistance(Vector2f p1, Vector2f p2){
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y) );
}

//ditancia entre un nodo y un punto
float distanceNodePoint(Node* node, Vector2f p){
    MBB box = node->boundingBox;
    float centerX = (box.topRight.x - box.topLeft.x) / 2;
    float centerY = (box.bottomLeft.y - box.topLeft.y) / 2;
    return sqrt((centerX - p.x)*(centerX - p.x) + (centerY - p.y) * (centerY - p.y));
}

//distancia entre un punto  una figura
float distancePointFigure(Vector2f p, Node* node){
    auto dx = max({node->boundingBox.topLeft.x - p.x, float(0), p.x - node->boundingBox.bottomRight.x});
    auto dy = max({node->boundingBox.topLeft.y - p.y, float(0), p.y - node->boundingBox.bottomRight.y});
    return sqrt(dx*dx + dy*dy);
}

//union entre dos cajas
MBB mergeBounds(std::vector<Node*> bounds){
    MBB res = bounds.front()->boundingBox;
    for(auto region: bounds){
        res.merge(region->boundingBox);
    }
    return res;
}

//funcion para insertar un elemento de forma ordenada a un vector 
template <typename T, typename C>
auto insertOrdered(std::vector<T>& vec, const T& element, C& callable) -> decltype(vec.begin()) {
    auto i = vec.begin();
    for(; i != vec.end(); ++i){
        if(callable(element, *i)){
            break;
        }
    }
    return vec.insert(i, element);
}

//funcion para escoger el mejor candidato
template <typename T, typename F, typename C>
auto getFirst(std::vector<T>& vec, const F& to_find, C& callable) -> decltype(vec.begin()){
    return std::lower_bound(vec.begin(), vec.end(), to_find, callable);
}

//lambda para comprar los hilbert index entre dos nodos
const auto compare  = [](Node* n1, Node* n2){
    return n1->gethIndex() < n2->gethIndex();
};

//lambda para comparar los hilbert index de dos nodos hojos
const auto compareL  = [](leafNode* n1, leafNode* n2){
    return n1->gethIndex() < n2->gethIndex();
};

//ordenar por el valor del hindex
const auto lessHIndex = [](Node* n, Vector2f& q){
    return n->gethIndex() < xy2d(q);
};

class RTree
{
    Node* root;
    float dk = 0;
    float minRad = 100000;

    //split de los hijos entre dos semillas
    void split(Node* seedOne, Node* seedTwo){
        std::vector<Node*> auxV = seedOne->children;
        int mx = (int)auxV.size();

        std::vector<Node*> s1 = {auxV.begin(), auxV.begin() + ceil(mx/2) };
        std::vector<Node*> s2 = {auxV.begin() + ceil(mx/2) , auxV.end()};

        MBB m1 = mergeBounds(s1);
        MBB m2 = mergeBounds(s2);

        seedOne->boundingBox = m1;
        seedTwo->boundingBox = m2;

        seedOne->children = s1;
        seedTwo->children = s2;

        for(auto &n: seedOne->children) n->father = seedOne;
        for(auto &n: seedTwo->children) n->father = seedTwo;

        seedOne->mergeBoundingBoxes();
        seedTwo->mergeBoundingBoxes();
    }

    //actualizar el nodo en un insert o en un delete
    void adjustTree(std::vector<Node*>& vec, std::vector<Node*>::iterator q, std::vector<Node*>::iterator s, int d = 0){
        std::vector<Node*> auxVec;
        for(auto it = q; it != (s+1); ++it)
            for(auto child: (*it)->children)
                auxVec.push_back(child);
        std::sort(auxVec.begin(), auxVec.end(), compare);
        int n = (int)auxVec.size();
        int x = std::distance(q, s) + 1 - d;
        auto auxBeg  = auxVec.begin();
        for(auto it = q; it != s+1; it++){
            int insertInd =  ceil(n/ (x * 1.0));
            (*it)->children.clear();
            (*it)->children.insert((*it)->children.begin(), auxBeg  , auxBeg  + insertInd);
            std::for_each(auxBeg , auxBeg  + insertInd, [&](Node* ln){
                ln->father = (*it);
            } );
            (*it)->mergeBoundingBoxes();
            auxBeg += insertInd;
            n -= insertInd;
            x--;

            if(n <= 0) break;
        }
        auxVec.clear();
    }

    //manejar el overflow de un nodo
    void handleOverflow(Node* node){
        if(!node->father){
            Node* v = new Node();
            split(node, v);
            Node* overFather = new Node();
            node->father = v->father = overFather;
            insertOrdered(overFather->children, node, compare);
            insertOrdered(overFather->children, v, compare);
            overFather->mergeBoundingBoxes();
            return ;
        }

        Node* nodeFather = node->father;
        auto s = std::find(nodeFather->children.begin(), nodeFather->children.end(), node);
        auto left = std::find_if(std::make_reverse_iterator(s), nodeFather->children.rend(),
                                  [] (Node* n){ return n->children.size() < M;});
        if(left != nodeFather->children.rend()){
            auto q =left.base() - 1;
            adjustTree(nodeFather->children, q, s);
            nodeFather->mergeBoundingBoxes();
        }
        else{
            auto auxNode =  new Node();
            auxNode->father =  nodeFather;
            s = nodeFather->children.insert(s + 1, auxNode);
            adjustTree(nodeFather->children, nodeFather->children.begin(), s);
            nodeFather->mergeBoundingBoxes();
            if(nodeFather->children.size() == M +1){
                handleOverflow(nodeFather);
            }
        }
    }

    //escoger el mejor sub arbol
    Node* chooseSubTree(Node* node, Figure figure){
        float minPerimeter = std::numeric_limits<float>::max();
        auto ans = new Node();
        for(auto n: node->children){
            auto auxBB = figure.getBoundingBox();
            auxBB.merge(n->boundingBox);
            auto p = auxBB.perimeter();
            if(p < minPerimeter){
                minPerimeter = p;
                ans = n;
            }
        }
        return ans;
    }
    
    //funcion auxiliar para insertar un nodo
    Node* insertRect(Node* node, Figure figure){
        if(node->isLeaf()){
            Node* auxNode = new leafNode(figure);
            insertOrdered(node->children, auxNode, compare);
            auxNode->father = node;
            node->mergeBoundingBoxes();
            if(node->children.size() == M+1) {
                handleOverflow(node);
            }
        } else {
            Node* bestSubTree = chooseSubTree(node, figure);
            insertRect(bestSubTree, figure);
            node->mergeBoundingBoxes();
        }
        if(node->father)
            return node->father;
        return node;
    }

    //busqueda
    Node* search(Node* node, const Vector2f &p){

        if (node->isLeaf()) {
            if(node->father == nullptr) return node;
            for(auto c: node->children){
                auto leaf = dynamic_cast<leafNode*>(c);
                if(leaf->boundingBox.isInside(p))
                    return node;
            }
            return nullptr;
        }

        for (auto c : node->children) {
            if (c->boundingBox.isInside(p)){
                auto res = search(c, p);
                if(res != nullptr)
                    return res;
            }
        }
        return nullptr;
    }

    //eliminar
    void remove(Node* node, const Vector2f & p){

        Node* n = search(p);
        if(n == nullptr || !n->isLeaf() ){
            return ;
        }

        auto fun = [&p](Node* node){
            return  (dynamic_cast<leafNode*>(node)->getBoundingBox().isInside(p));
        };

        auto it = std::find_if(n->children.begin(), n->children.end(), fun);
        if(it == n->children.end()) return ;

        (n->children).erase(it);

        if(n->father == nullptr && n->children.size() == 0){
            n->boundingBox   = MBB();
            return ;
        }

        n->mergeBoundingBoxes();
        update(n);

        if(n->children.size() >= std::ceil(M/2.0) || n->father == nullptr){
            return ;
        }

        handleUnderflow(n);
        update(n);

    }

    //funcion para controlar cuando un nodo se queda con menos de m hijos
    void handleUnderflow(Node* node){

        if(node->father == nullptr){
            std::vector<Node*> vec;
            for(auto c : node->children){
                for(auto child: c->children){
                    vec.push_back(child);
                }
            }
            node->children.clear();
            std::sort(vec.begin(), vec.end(), compare);
            for(auto v: vec){
                v->father = node;
                node->children.push_back(v);
            }
            vec.clear();
            return ;
        }

        Node* nodeFather = node->father;

        auto s = std::find(nodeFather->children.begin(), nodeFather->children.end(), node);

        auto rev_q = std::find_if(std::make_reverse_iterator(s), nodeFather->children.rend(),
                                  [] (Node* n){ return n->children.size() > ceil(M/2.0);});

        if(rev_q != nodeFather->children.rend()){
            auto q =rev_q.base() -1;
            adjustTree(nodeFather->children, q, s);
        }
        else{

            adjustTree(nodeFather->children, nodeFather->children.begin(), nodeFather->children.end() - 1, 1);

            nodeFather->children.erase(nodeFather->children.end() - 1);
            nodeFather->mergeBoundingBoxes();


            if(nodeFather->children.size() < ceil(M/2.0)){
                handleUnderflow(nodeFather);
                nodeFather->mergeBoundingBoxes();
            }
        }
    }

    //funcion para actualizar el padre de hijos de algun nodo
    void update(Node* v) {
        v->mergeBoundingBoxes();
        if(v != root) {
            update(v->father);
        }
    }

    //actualizar hacia arriba
    void propagateUpwards(Node* node){
        while(node->father != nullptr){
            node->mergeBoundingBoxes();
            node = node->father;
        }
    }
public:

    RTree(){
        root = new Node();
    }

    void insert(Figure figure){
        root = insertRect(root, figure);
    }

    vector<Figure> knnSearch(Vector2f point, int n){
        priority_queue<pair<Node*, float>, vector<pair<Node*, float>>, compareNodes> pq;
        pq.push({root, distancePointFigure(point, root)});
        vector<Figure> ans;
        while(!pq.empty()){
            auto top = pq.top();
            if(top.first->isLeaf()){
                for(auto z: top.first->children)
                {
                    auto ln = dynamic_cast<leafNode *> (z);
                    ans.push_back(ln->getFigure());
                }
            }
            if(ans.size() == n) break;
            pq.pop();
            for(auto &x: top.first->children){
                if(x != nullptr)
                    pq.push({x, distancePointFigure(point, x)});
            }
        }
        return ans;
    }


    //k vecinos cercanos visto en clases recorriendo el arbol
    template<typename cmp>
    void k_depthFirst(std::priority_queue<Figure , std::vector<Figure>, cmp> &pq, int &k, Node* node, Vector2f p){
        if(node->isLeaf()){
            for(auto f: node->children){
                if(dk + distanceNodePoint(f, p) < distancePointFigure(p, f))
                    continue;
                else
                {
                    pq.push(dynamic_cast<leafNode *>(f)->getFigure());
                    if (distancePointFigure(p, f) > dk)
                        dk = distancePointFigure(p, f);
                    if (pq.size() > (unsigned) k) pq.pop();
                }
            }
            return;
        }


        for(auto r: node->children){
            auto rad = r->getRadio();
            if(rad < minRad)
                minRad = rad;
            if(dk + distanceNodePoint(r, p) < minRad)
                continue;
            else
                k_depthFirst(pq,k,r, p);
        }
    }

    //k vecinos cercanos 
    vector<Figure> depthFirst(Vector2f p, int k){
        std::vector<Figure> ans;
        auto cmp  = [&p](Figure f1, Figure f2){
            MBB m1 = f1.getBoundingBox();
            MBB m2 = f2.getBoundingBox();
            return distanceMBB(p, m1) < distanceMBB(p, m2);
        };
        std::priority_queue<Figure , std::vector<Figure>, decltype(cmp) > pq(cmp);
        k_depthFirst(pq,k,root, p);
        while(!pq.empty()){
            auto t = pq.top();
            ans.push_back(t);
            pq.pop();
        }
        return ans;
    }


    //funcion para dibujar lineas a alguna figura
    void drawLinesToFoundFigures(vector<Figure> figures, RenderWindow& renderWindow, CircleShape circle){
        for (int i = 0; i < figures.size(); ++i)
        {
            sf::Vertex vertices[2] = {Vertex(circle.getPosition()), Vertex(figures[i].getBoundingBox().centroid, Color::Blue)};
            renderWindow.draw(vertices, 2, sf::Lines);
        }
    }

    Node* search(const Vector2f& p){
        return search(root, p);
    }

    void remove(const Vector2f & p){
        remove(root, p);
    }

    //funcion que recorre el arbol por nivel y muestra las figuras en la gui
    void bfs(RenderWindow &renderWindow){
        queue<Node*> q;
        q.push(root);
        int i = 0;
        while(!q.empty()){
            ++i;
            Node* no = q.front();
            if(no != root)
               no->draw(renderWindow);
            q.pop();
            for(auto &x: no->children){
                if(x != nullptr)
                    q.push(x);
            }
        }
    }
};




#endif //LINES_RTREE_H
