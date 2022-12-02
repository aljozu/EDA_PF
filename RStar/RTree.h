//
// Created by lojaz on 14/10/2022.
//

#ifndef LINES_RTREE_H
#define LINES_RTREE_H

#include "Node.h"
#include <limits>
#include <queue>
#include <numeric>
#include <random>


std::random_device dev;
template<class T>
int Rand(T first, T last) {
    std::default_random_engine eng(dev());
    std::uniform_int_distribution<int> dis(first, last);
    return dis(eng);
}

static bool orderByLowestDistance(pair<Node*, double> p1, pair<Node*, double> p2){
    return p1.second < p2.second;
}
static bool orderByHighestDistance(pair<Node*, double> p1, pair<Node*, double> p2){
    return p1.second > p2.second;
}


static float distanceMBB(Vector2f p, MBB node){
    auto dx = max({node.topLeft.x - p.x, float(0), p.x - node.bottomRight.x});
    auto dy = max({node.topLeft.y - p.y, float(0), p.y - node.bottomRight.y});
    return sqrt(dx*dx + dy*dy);
}

struct LessThanByMBB {
    bool operator()(Figure* f1, Figure*f2, Vector2f p) const
    {
        MBB m1 = f1->getBoundingBox();
        MBB m2 = f2->getBoundingBox();
        return distanceMBB(p, m1) < distanceMBB(p, m2);
    }
};

struct compareNodes{
    bool operator()(pair<Node*, float> n1, pair<Node*, float> n2){
        return n1.second > n2.second;
    }
};

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

double getDistance(Vector2f p1, Vector2f p2){
    return sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y) );
}

float distanceNodePoint(Node* node, Vector2f p){
    MBB box = node->boundingBox;
    float centerX = (box.topRight.x - box.topLeft.x) / 2;
    float centerY = (box.bottomLeft.y - box.topLeft.y) / 2;
    return sqrt((centerX - p.x)*(centerX - p.x) + (centerY - p.y) * (centerY - p.y));
}

float distancePointFigure(Vector2f p, Node* node){
    auto dx = max({node->boundingBox.topLeft.x - p.x, float(0), p.x - node->boundingBox.bottomRight.x});
    auto dy = max({node->boundingBox.topLeft.y - p.y, float(0), p.y - node->boundingBox.bottomRight.y});
    return sqrt(dx*dx + dy*dy);
}
MBB mergeBounds(std::vector<Node*> bounds){
    MBB res = bounds.front()->boundingBox;
    for(auto region: bounds){
        res.merge(region->boundingBox);
    }
    return res;
}

class RTree
{
    Node* root;
    //bool firstSplit = true;
    float dk = 0;
    float minRad = 100000;
    vector<pair<int,bool>> reinsertedLevel = {make_pair(0, false)};

    //funcion que busca si el par booleano del nivel ya fue reinsertado
    bool wasReinserted(int level){
        for(auto &x: reinsertedLevel){
            if(x.first == level) return x.second;
        }
        return false;
    }

    //despues de terminar un insert se setea todos los nodos como no visitados
    void resetBoolValues(){
        for(auto &x: reinsertedLevel){
            x.second = false;
        }
    }

    //setea un nivel como reinsertado
    void setReinserted(int level, bool b){
        for(auto &x: reinsertedLevel){
            if(x.first == level) x.second = b;
        }
    }

    tuple <float, vector<Node*>, vector<Node*>> getSemSums(vector<Node*> auxSplit){
        float lowexsum = std::numeric_limits<float>::max();
        vector<Node*> s1, s2;
        for(int i = ceil(M*0.4); i <= auxSplit.size() - ceil(M*0.4); ++i) {
            std::vector<Node*> leftPart = {auxSplit.begin(), auxSplit.begin() + i};
            std::vector<Node*> rightPart = {auxSplit.begin() + i, auxSplit.end()};

            MBB b1 = mergeBounds(leftPart);
            MBB b2 = mergeBounds(rightPart);
            if(b1.semiperimeter() + b2.semiperimeter() < lowexsum)
            {
                lowexsum = b1.semiperimeter() + b2.semiperimeter();
                s1 = leftPart;
                s2 = rightPart;
            }
        }
        return make_tuple(lowexsum, s1, s2);
    }
    vector<Node*> chooseSplitAxis(vector<Node*> nodes){
        auto xsplit = nodes;
        auto ysplit = nodes;
        //sorting by the lower over x-axis
        sort(xsplit.begin(), xsplit.end(),
             [](Node *a, Node *b) -> bool
             {
                 return a->getBoundingBox().topLeft.x < b->getBoundingBox().topLeft.x;
             });
        auto lowexsum = getSemSums(xsplit);
        //sorting by the highest over x-axis
        sort(xsplit.begin(), xsplit.end(),
             [](Node *a, Node *b) -> bool
             {
                 return a->getBoundingBox().bottomRight.x > b->getBoundingBox().bottomRight.x;
             });
        auto highxsum = getSemSums(xsplit);
        //sorting by the lower over y-axis
        sort(ysplit.begin(), ysplit.end(),
             [](Node *a, Node *b) -> bool
             {
                 return a->getBoundingBox().topLeft.y < b->getBoundingBox().topLeft.y;
             });
        auto lowysum = getSemSums(ysplit);
        //sorting by the highest over y-axis
        sort(ysplit.begin(), ysplit.end(),
             [](Node *a, Node *b) -> bool
             {
                 return a->getBoundingBox().bottomRight.y > b->getBoundingBox().bottomRight.y;
             });
        auto highysum = getSemSums(ysplit);
        vector<Node*> ans;
        //check the minimum sum
        if(std::get<0>(lowexsum) + std::get<0>(highxsum) < std::get<0>(lowysum) + std::get<0>(highysum)){
            if(std::get<0>(lowexsum) < std::get<0>(highxsum)){
                ans = std::get<1>(lowexsum);
                std::copy(std::get<2>(lowexsum).begin(), std::get<2>(lowexsum).end(), std::back_inserter(ans));
            } else {
                ans = std::get<1>(highxsum);
                std::copy(std::get<2>(highxsum).begin(), std::get<2>(highxsum).end(), std::back_inserter(ans));
            }
        }else {
            if(std::get<0>(lowysum) < std::get<0>(highysum)) {
                ans = std::get<1>(lowysum);
                std::copy(std::get<2>(lowysum).begin(), std::get<2>(lowysum).end(), std::back_inserter(ans));
            } else {
                ans = std::get<1>(highysum);
                std::copy(std::get<2>(highysum).begin(), std::get<2>(highysum).end(), std::back_inserter(ans));
            }
        }
        return ans;
    }

    pair<vector<Node*>, vector<Node*>> chooseSplitIndex(vector<Node*> nodes){
        std::vector<Node*> leftPart, rightPart;
        float minOverlap = std::numeric_limits<float>::max();
        for(int i = ceil(M*0.4); i <= nodes.size() - ceil(M*0.4); ++i) {
            std::vector<Node*> lp = {nodes.begin(), nodes.begin()+i};
            std::vector<Node*> rp = {nodes.begin()+i, nodes.end()};
            MBB b1 = mergeBounds(lp);
            MBB b2 = mergeBounds(rp);
            if(b1.getOverlap(b2) < minOverlap)
            {
                minOverlap = b1.semiperimeter() + b2.semiperimeter();
                leftPart = lp;
                rightPart = rp;
            }
        }
        return make_pair(leftPart, rightPart);
    }
    void split(Node* seedOne, Node* seedTwo){
        std::vector<Node*> auxV = seedOne->children;

        auto seed = chooseSplitAxis(auxV);
        auto splits = chooseSplitIndex(seed);

        MBB m1 = mergeBounds(splits.first);
        MBB m2 = mergeBounds(splits.second);

        seedOne->boundingBox = m1;
        seedTwo->boundingBox = m2;

        seedOne->children = splits.first;
        seedTwo->children = splits.second;

        for(auto &n: seedOne->children) n->father = seedOne;
        for(auto &n: seedTwo->children) n->father = seedTwo;

        seedOne->mergeBoundingBoxes();
        seedTwo->mergeBoundingBoxes();
    }

    void handleOverflow(Node* node, int level){
        if(!node->father){
            Node* v = new Node();
            split(node, v);
            root = new Node();
            root->level = node->level+1;
            v->level=node->level;
            node->father = v->father = root;
            root->children.push_back(node);
            root->children.push_back(v);
            root->mergeBoundingBoxes();
            reinsertedLevel.emplace_back(root->level, false);
            node->boundingBox.color = v->boundingBox.color = Color(Rand(0,255), Rand(0,255), Rand(0,255));
            return;
        }
        if(!wasReinserted(level) && root->level != level){
            reInsert(node, level);
        }
        else{
            Node* u = new Node();
            split(node, u);
            auto w = node->father;
            u->level = node->level;
            u->father = w;
            w->children.push_back(u);
            w->mergeBoundingBoxes();
            if(w->children.size() == M + 1){
                handleOverflow(w, w->level);
                w->mergeBoundingBoxes();
            }
        }
    }

    void reInsert(Node* node, int level){
        setReinserted(level, true);
        vector<pair<Node*, double>> farNodes;
        auto center = node->boundingBox.centroid;
        for(auto n: node->children){
            farNodes.emplace_back(n, getDistance(n->boundingBox.centroid, center));
        }
        sort(farNodes.begin(), farNodes.end(), orderByLowestDistance);

        auto size = (int)farNodes.size();

        for(int i = size - 1; i >= size - ceil(size*0.3); --i){
            auto it = std::find_if(node->children.begin(), node->children.end(), [&](Node* n) { return n == farNodes[i].first; });
            node->children.erase(it);
        }

        node->mergeBoundingBoxes();
        for(int i = size - 1; i >= size - ceil(size*0.3); --i) {
            insertRect(node, farNodes[i].first, node->level);
        }
    }

    Node* chooseSubTree(Node* node, Node* figure, int level){
        auto ans = new Node();

        if(node->level == 0 || (node->level == level )) {

            return node;
        }
        else{
            if(node->children[0]->level==0){
                float minOverlap = std::numeric_limits<float>::max();
                for(auto n: node->children){
                    auto auxBB = figure->boundingBox;
                    auto a = auxBB.getOverlap(n->boundingBox);
                    if(a < minOverlap){
                        minOverlap = a;
                        ans = n;
                    }
                }
            } else {
                float minPerimeter = std::numeric_limits<float>::max();
                for (auto n: node->children)
                {
                    auto auxBB = figure->boundingBox;
                    auxBB.merge(n->boundingBox);
                    auto p = auxBB.perimeter();
                    if (p < minPerimeter)
                    {
                        minPerimeter = p;
                        ans = n;
                    }
                }
            }
        }
        chooseSubTree(ans, figure, level);
    }

    void insertRect(Node* node, Node* auxNode, int level){
        auto bestSubTree = chooseSubTree(node, auxNode, level);
        bestSubTree->children.push_back(auxNode);
        auxNode->father = bestSubTree;
        bestSubTree->mergeBoundingBoxes();
        if(bestSubTree->children.size() > M) {
            handleOverflow(bestSubTree, bestSubTree->level);
        }
    }

    Node* search(Node* node, const Vector2f &p){
        if (node->isLeaf()) {
            if(node->father == nullptr) return node;
            for(auto c: node->children){
                auto lf = dynamic_cast<leafNode*>(c);
                if(lf->boundingBox.isInside(p))
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

public:

    RTree(){
        root = new Node();
        root->level=0;
    }

    ~RTree(){
        delete root;
    }

    void insert(Figure figure) {
        resetBoolValues();
        Node* auxNode = new leafNode(figure);
        //root = insertRect(root, auxNode, 0);
        insertRect(root, auxNode, 0);

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


        if(n->children.size() >= std::ceil(M/2.0) || n->father == nullptr){
            n->mergeBoundingBoxes();
            propagateUpwards(n);
            return ;
        }

        reinsert();
    }

    void propagateUpwards(Node* node){
        while(node){
            node->mergeBoundingBoxes();
            node = node->father;
        }
    }

    void reinsert(){

        std::vector<Figure> figs;
        dfs(figs,root);
        root = new Node();
        clearChildren(root);
        for(auto f: figs){
            insert(f);
        }
        root->mergeBoundingBoxes();
    }

    void clearChildren(Node* node){
        for(auto child: node->children){
            clearChildren(child);
            delete child;
        }
    }

    void dfs(std::vector<Figure> &figs, Node* node){
        if(node->isLeaf()){
            for(auto f: node->children){
                figs.push_back(dynamic_cast<leafNode*>(f)->getFigure());
            }
            return ;
        }

        for(auto r: node->children){
            dfs(figs,r);
        }
    }

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
