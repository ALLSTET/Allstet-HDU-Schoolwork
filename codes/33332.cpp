#include <iostream>
#include <string>
using namespace std;

typedef struct BitNode {
    char data;
    struct BitNode *lchild, *rchild;
} BitNode, *BitTree;

void preorder(BitTree T) {
  if (T != nullptr) {
      std::cout << T->data << " ";
      preorder(T->lchild);
      preorder(T->rchild);
  }
}

void destroyTree(BitTree& T) {
    if (T != nullptr) {
        destroyTree(T->lchild);
        destroyTree(T->rchild);
        delete T;
        T = nullptr;
    }
}

BitTree findParent(BitTree root, char target, BitTree& parent) {
    if (root == nullptr) return nullptr;

    if (root->data == target) {
        return root;
    }

    if (root->lchild != nullptr) {
        BitTree found = findParent(root->lchild, target, parent);
        if (found != nullptr) {
            parent = root;
            return found;
        }
    }

    if (root->rchild != nullptr) {
        BitTree found = findParent(root->rchild, target, parent);
        if (found != nullptr) {
            parent = root;
            return found;
        }
    }

    return nullptr;
}

int main(){
  string line;
  BitTree Root = new BitNode;
  Root->data = '^';
  while (getline(cin,line)){
    if(line.empty()){
      cerr<<"Invalid input."<<endl;
      break;
    }
    else if(line[0] == '^' && line[1] == '^'){
      std::cout<<"End of input."<<endl;
      break;
    }
    else{
      char F = line[0];
      char C = line[1];
      char LR = line[2];
      std::cout<<"Successful input! F: "<<F<<", C: "<<C<<", LR: "<<LR<<endl;
      if(F == Root->data){
        BitTree p = new BitNode;
        p->data = C;
        p->lchild = nullptr;
        p->rchild = nullptr;
        if(LR == 'L'){
          Root->lchild = p;
        }
        else if(LR == 'R'){
          Root->rchild = p;
        }
      }
      else{
        BitTree parent = nullptr;
        BitTree fnode = findParent(Root, F, parent);
        if(fnode != nullptr){
          BitTree p = new BitNode;
          p->data = C;
          p->lchild = nullptr;
          p->rchild = nullptr;
          if(LR == 'L'){
            fnode->lchild = p;
          }
          else if(LR == 'R'){
            fnode->rchild = p;
          }
        }
        else{
          cerr<<"Parent node "<<F<<" not found."<<endl;
        }
      }
    }
  }
  cout<<"Preorder traversal result: ";
  preorder(Root);
  getchar();

  destroyTree(Root);
  return 0;
} 