#include <iostream>
namespace gcdst{
template<class Comparable=int, class ValueType=int>
struct rbtree_node{
    rbtree_node * parent;
    rbtree_node * left;
    rbtree_node * right;

    Comparable key;
    ValueType val;
    unsigned char color; //0->black, 1->right
};

template<class Comparable=int, class ValueType=int>
class rbtree{
private:
#define Black 0
#define Red 1
    using Node = rbtree_node<Comparable, ValueType>;
    Node * _root;
    Node * _nil;
public:
    rbtree(): _nil(new Node()){
        this->_nil->color = Black;
        _nil->parent = _nil;
        _nil->left = _nil;
        _nil->right = _nil;
        _root = _nil;
    }
    rbtree(rbtree & other):_nil(new Node()){ // deep copy, O(n)
        this->_nil->color = Black;
        _nil->parent = _nil;
        _nil->left = _nil;
        _nil->right = _nil;
        _root = dfs_for_deepcopy(other._root, _nil, other._nil);
    }

    rbtree(rbtree && other):_nil(new Node()){ // move constructor, O(1)
        _root = other._root;
        _nil = other._nil;

        other._nil = nullptr;
        other._root = nullptr;
    }

    ~rbtree(){  //O(n)
        if(_root) dfs_for_delete(_root);
        if(_nil) delete _nil;
    }

    Node * root(){return _root;}
    void setRoot(Node *rt){this->_root = rt;}
    Node * nil() {return _nil;}

    void insert(Comparable key, ValueType value){
        Node * z = new Node({_nil, _nil, _nil, key, value, Red});  //red
        insert(z);
    }

    ValueType find(Comparable key){
        Node * ret = find_node(key);
        if(ret != _nil) return ret->val;
        throw "The key is not existed!";
    }

    ValueType operator[] (Comparable key){
        return find(key);
    }

    void remove(Comparable key){
        Node * target = find_node(key);
        if(target == _nil) return;
        delete_node(target);
    }

    bool test_insert_correction(){
        assert(_root->color == Black);
        assert(_nil->left == _nil);
        assert(_nil->right = _nil);
        assert(_nil->parent = _nil);
        assert(_root->parent = _nil);
        return dfs_for_test(_root);
    }

private:
    bool dfs_for_test(Node * rt){
        if(rt->left != _nil){
            //assert(rt->left->key < rt->key);
            if(rt->left->key > rt->key){
                std::cout << "left is bigger than root" << std::endl;
                std::cout << rt->left->key << " " << rt->key << " " << rt->right->key << std::endl;
                return false;
            } 
            //assert(dfs_for_test(rt->left));
            if(!dfs_for_test(rt->left)) return false;
            if(rt->color == rt->left->color) assert(rt->color == Black);
        }
        if(rt->right != _nil){
            //assert(rt->right->key > rt->key);
            if(rt->right->key < rt->key){
                std::cout << "right is smaller than root" << std::endl;
                std::cout << rt->left->key << " " << rt->key << " " << rt->right->key << std::endl;
                return false;
            } 
            if(!dfs_for_test(rt->right)) return false;
            if(rt->color == rt->right->color) assert(rt->color == Black);
        }
        return true;
    }

    void dfs_for_delete(Node * rt){
        //in-tranverse
        if(!rt || rt == _nil) return;
        if(rt->left != _nil) dfs_for_delete(rt->left);
        if(rt->right != _nil) dfs_for_delete(rt->right);
        delete rt;
    }

    Node * dfs_for_deepcopy(Node * other_rt, Node * pre, Node * other_rt__nil){
        Node * ret = new Node();
        ret->key = other_rt->key;
        ret->val = other_rt->val;
        ret->color = other_rt->color;
        ret->parent = pre;

        if(other_rt->left != other_rt__nil) ret->left = dfs_for_deepcopy(other_rt->left, ret, other_rt__nil);
        else ret->left = this->_nil;

        if(other_rt->right != other_rt__nil) ret->right = dfs_for_deepcopy(other_rt->right, ret, other_rt__nil);
        else ret->right = this->_nil;      

        return ret;  
    }

    void left_rotate(Node *x){
        //             x                                        y
        //  alpha              y             -->           x       gamma
        //               beta    gamma              alpha      bata
        Node* y = x->right;
        if(y == _nil) return;
        x->right = y->left;
        if(y->left != _nil) y->left->parent = x;

        y->parent = x->parent;
        if(x->parent == _nil) _root = y;
        else if(x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;

        y->left = x;
        x->parent = y;
    }

    void right_rotate(Node *y){
        //             x                                        y
        //  alpha              y             <--           x       gamma
        //               beta    gamma              alpha      bata
        Node * x = y->left;
        if(x == _nil) return;
        y->left = x->right;
        if(x->right != _nil) x->right->parent = y;

        x->parent = y->parent;
        if(y->parent == _nil) _root = x;
        else if(y == y->parent->left) x->parent->left = x;
        else x->parent->right = x;

        x->right = y;
        y->parent  = x;
    }

    void insert(Node *z){
        Node* target = _root;
        if(target == _nil){ //z is the first node in the tree;
            z->parent = _nil;
            z->color = Black;
            _root = z;
            return;
        }
        Node* target_p = _nil;
        while(target != _nil){
            target_p = target;
            if(z->key <= target->key) target = target->left;  //actually, if equal, we have to throw an exception
            else {  //z > target
                target = target->right;
            }
        } 

        //insert z as a child of target_p.
        z->parent = target_p;
        if(z->key <= target_p->key) target_p->left = z;
        else target_p->right = z;

        this->rbtree_insert_fixup(z); //z should be adjust because z is red and may break some rules
    }

    void rbtree_insert_fixup(Node *z){
        //check z's parent, if its color is red, we have to adjust using the color of z's uncle 
        //or z is the _root, it is much more easy.
        Node * y = z->parent;
        while (y->color == 1){   //before adjust, if z->parent->color == 1, z->parent can not be the _root.
            Node * uncle = _nil;
            if(y == y->parent->left) uncle = y->parent->right;
            else uncle = y->parent->left;

            if(uncle->color == 1){
                y->color = Black;
                uncle->color = Black;
                y->parent->color = 1;
                z = y->parent;
                y = z->parent;
            }
            else{
                //Step 1. check if rotation of z and z'parent is needed
                //Step 2. change colors of y and y->parent
                //Step 3. rotate the parent of (z, z->parent(origin)) and grandparent
                
                if(z == y->left && y == y->parent->left){
                    //no self-rotation, change the color of y and y'parent, right_rotate(y->parent)
                    y->color = Black;
                    y->parent->color = 1;
                    right_rotate(y->parent);
                }

                else if(z == y->right && y == y->parent->right){
                    // no self-rotation, change the color of y and y'parent, left_rotation(y->parent)
                    y->color = Black;
                    y->parent->color = 1;
                    left_rotate(y->parent);
                }

                else if(z == y->right && y == y->parent->left){
                    // left_rotate(y), change the color of z and z'parent, right_rotate(z->parent)
                    z->color = Black;
                    y->parent->color = 1;
                    left_rotate(y);
                    right_rotate(z->parent);
                    y = z;
                    z = y->left;
                }

                else { //that is: z == y->left && y = y->parent->right
                    z->color = Black;
                    y->parent->color = 1;
                    right_rotate(y);
                    left_rotate(z->parent);
                    y = z;
                    z = y->left;
                }
            }
            //z is fixup as a _root
            if(y == _nil){
            z->color = Black;
            _root = z;
            }
        }
    }

    Node * find_node(Comparable key){
        Node * z = _root;
        while(z != _nil){
            if(z->key == key) return z;
            else if(z->key < key) z = z->right;
            else z = z->left;
        } 
        return z;  //here we should throw an exception
    }

    Node * maximum(Node * x){
        Node * ret = x;
        while(ret->right != _nil) ret = ret->right;
        return ret;
    }

    Node * minimum(Node * x){
        Node * ret = x;
        while (ret->left != _nil) ret = ret->left;
        return ret;
    }

    inline void swap_node(Node * a, Node * b){
        std::swap(a->key, b->key);
        std::swap(a->val, b->val);
    }

    Node * find_predecessor(Node * replaced){
        // find the predecessor(the last num that less than repalced->val)
        Node * ret = replaced->left;
        if(ret != _nil) return maximum(ret);
        Node * father = replaced->parent;
        ret = replaced;
        while (father != _nil && ret == father->left){
            ret = father;
            father = ret->parent;
        }
        ret = nullptr;
        return father;
    }

    Node * find_successor(Node * replaced){
        Node * ret = replaced->right;
        if(ret != _nil) return minimum(ret);
        Node * father = replaced->parent;
        ret = replaced;
        while (father != _nil && ret == father->right){
            ret = father;
            father = ret->parent;
        }
        ret = nullptr;
        return father;
    }

    void delete_node(Node * z){
        //there are 3 cases, divided by the num of children
        if(z->right != _nil && z->left != _nil){  // 2 children
            //swap it with its predecessor or successor
            Node* suc = find_successor(z);
            assert(suc->val > z->val);
            assert(suc->val <= z->right->val);
            this->swap_node(z, suc);  //now the value and key are in z, and the pre is what should be delete, and it has 0 or 1 child
            delete_node(suc);            
        }
        else if(z->left != _nil || z->right != _nil){ // 1 child
            Node * child = (z->left != _nil) ? z->left: z->right;
            this->swap_node(z, child);
            delete_node(child);
        }
        else{  // 0 child
            if(z->color == Black) rbtree_delete_fixup(z);
            if(z == _root) _root = _nil;
            else if(z == z->parent->left) z->parent->left = _nil;
            else z->parent->right = _nil;
            delete z;
        }
    }

    //the procated method for delele_fixup
    void rbtree_delete_fixup(Node * z){
        while (z != _root && z->color == Black)
        {
            if(z == z->parent->left){
                Node *y = z->parent->right; //y is sibling
                if(y->color == Red){  //y is red, then it must have 2 black child
                    y->left->color = Red;
                    y->color = Black;  //because father must be Black
                    left_rotate(y->parent);
                    break;
                }
                else if(y->right != _nil){
                    y->color = y->parent->color;
                    y->right->color = Black;
                    y->parent->color = Black;
                    left_rotate(y->parent);
                    break;
                }
                else if(y->left != _nil){
                    y->left->color = y->parent->color;
                    y->parent->color = Black;
                    right_rotate(y); 
                    left_rotate(y->parent->parent);
                    break;
                } else {
                    y->color = Red;
                    y->parent->color = Black;
                    z = y->parent;
                }
            } else {  //z == z->parent->right
                Node * y = z->parent->left;
                if(y->color == Red){
                    y->right->color = Red;
                    y->color = Black;
                    right_rotate(y->parent);
                    break;
                }
                else if(y->left != _nil){
                    y->color = y->parent->color;
                    y->left->color = Black;
                    y->parent->color = Black;
                    right_rotate(y->parent);
                    break;
                }
                else if(y->right != _nil){
                    y->right->color = y->parent->color;
                    y->parent->color = Black;
                    left_rotate(y); 
                    right_rotate(y->parent->parent);
                    break;
                } else {
                    y->color = Red;
                    y->parent->color = Black;
                    z = y->parent;
                }
            }
        }
    }
};
}